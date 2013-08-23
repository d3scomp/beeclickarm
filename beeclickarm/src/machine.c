#include "machine.h"
#include <stm32f4_discovery.h>
#include <string.h>
#include <stdio.h>
#include "uart_io.h"


#define LED_RXTX LED4
#define LED_OUT_OF_SYNC LED5
#define LED_RF_RECV LED3
#define LED_RF_SEND LED6


#define MAX_TOD_QUEUE_LENGTH 8
#define MAX_TOH_QUEUE_LENGTH 8
#define MAX_RF_PACKET_LENGTH 128

#define TOHD_SYNC_PATTERN "Hello there!"
#define TOHD_SYNC_PATTERN_LENGTH 12

#define VOLUNATARY_SYNC_PERIOD 1000000 // Every nth period, we sent SYNC just by ourselves
#define MAX_INFO_TEXT_LENGTH 255

typedef enum {
	TOD_SYNC,
	TOD_SEND_PACKET,
	TOD_SET_CHANNEL,
	TOD_TYPES_NUMBER
} TODType;

typedef struct {
	TODType type;
	uint8_t pattern[TOHD_SYNC_PATTERN_LENGTH];
} TODSync;

typedef struct {
	TODType type;
	uint8_t length;
	uint32_t seq;
	uint8_t data[MAX_RF_PACKET_LENGTH];
} TODSendPacket;

typedef struct {
	TODType type;
	uint8_t channel;
} TODSetChannel;

typedef union {
	TODType type;
	TODSync sync;
	TODSendPacket sendPacket;
	TODSetChannel setChannel;
} TOD;


size_t todFixedLengths[TOD_TYPES_NUMBER] = {
	sizeof(TODSync),
	0,
	sizeof(TODSetChannel)
};

static size_t todLengthHandlerFixed(TOD msg) {
	size_t length = todFixedLengths[msg.type];

	assert_param(length != 0);

	return length;
}

static size_t todLengthHandlerSendPacket(TOD msg) {
	return sizeof(TODType) + sizeof(uint8_t) + sizeof(uint32_t) + msg.sendPacket.length;
}

typedef size_t(*TODLengthHandler)(TOD msg);

TODLengthHandler todLengthHandlers[TOD_TYPES_NUMBER] = {
		todLengthHandlerFixed,		// TOD_SYNC
		todLengthHandlerSendPacket,	// TOD_SEND_PACKET
		todLengthHandlerFixed		// TOD_SET_CHANNEL
};



typedef enum {
	TOH_SYNC,
	TOH_RECV_PACKET,
	TOH_PACKET_SENT,
	TOH_CHANNEL_SET,
	TOH_INFO,
	TOH_TYPES_NUMBER
} TOHType;

typedef struct {
	TOHType type;
	uint8_t pattern[TOHD_SYNC_PATTERN_LENGTH];
} TOHSync;

typedef struct {
	TOHType type;
	uint8_t length;
	uint8_t rssi;
	uint8_t lqi;
	uint8_t fcs;
	uint8_t data[MAX_RF_PACKET_LENGTH];
} TOHRecvPacket;

typedef struct {
	TOHType type;
	uint32_t seq;
} TOHPacketSent;

typedef struct {
	TOHType type;
	uint8_t channel;
} TOHChannelSet;

typedef struct {
	TOHType type;
	uint8_t length;
	char text[MAX_INFO_TEXT_LENGTH];
} TOHInfo;

typedef union {
	TOHType type;
	TOHSync sync;
	TOHRecvPacket recvPacket;
	TOHPacketSent packetSent;
	TOHChannelSet channelSet;
	TOHInfo info;
} TOH;


size_t tohFixedLengths[TOH_TYPES_NUMBER] = {
	sizeof(TOHSync),
	0,
	sizeof(TOHPacketSent),
	sizeof(TOHChannelSet),
	0
};

static size_t tohLengthHandlerFixed(TOH msg) {
	size_t length = tohFixedLengths[msg.type];

	assert_param(length != 0);

	return length;
}

static size_t tohLengthHandlerRecvPacket(TOH msg) {
	return sizeof(TOHType) + sizeof(uint8_t) * 4 + msg.recvPacket.length;
}

static size_t tohLengthHandlerInfo(TOH msg) {
	return sizeof(TOHType) + sizeof(uint8_t) + msg.info.length;
}

typedef size_t(*TOHLengthHandler)(TOH msg);

TOHLengthHandler tohLengthHandlers[TOH_TYPES_NUMBER] = {
		tohLengthHandlerFixed,		// TOH_SYNC
		tohLengthHandlerRecvPacket,	// TOH_RECV_PACKET
		tohLengthHandlerFixed,		// TOH_PACKET_SENT
		tohLengthHandlerFixed,		// TOH_CHANNEL_SET
		tohLengthHandlerInfo		// TOH_INFO
};



TOD todQueue[MAX_TOD_QUEUE_LENGTH];
uint32_t todQueueReadPtr = 0; // From which position the msg is to be read (if todQueueReadPtr == todQueueWritePtr then there is nothing to be read)
uint32_t todQueueWritePtr = 0; // To which position the msg is to be written

TOH tohQueue[MAX_TOH_QUEUE_LENGTH];
uint32_t tohQueueReadPtr = 0; // From which position the msg is to be read (if tohQueueReadPtr == tohQueueWritePtr then there is nothing to be read)
uint32_t tohQueueWritePtr = 0; // To which position the msg is to be written

static void incTODQueueReadPtr() {
	assert_param(todQueueReadPtr != todQueueWritePtr);

	todQueueReadPtr++;
	if (todQueueReadPtr == MAX_TOH_QUEUE_LENGTH) {
		todQueueReadPtr = 0;
	}
}

static void incTODQueueWritePtr() {
	todQueueWritePtr++;
	if (todQueueWritePtr == MAX_TOH_QUEUE_LENGTH) {
		todQueueWritePtr = 0;
	}

	assert_param(todQueueReadPtr != todQueueWritePtr);
}

static void incTOHQueueReadPtr() {
	assert_param(tohQueueReadPtr != tohQueueWritePtr);

	tohQueueReadPtr++;
	if (tohQueueReadPtr == MAX_TOH_QUEUE_LENGTH) {
		tohQueueReadPtr = 0;
	}
}

static void incTOHQueueWritePtr() {
	tohQueueWritePtr++;
	if (tohQueueWritePtr == MAX_TOH_QUEUE_LENGTH) {
		tohQueueWritePtr = 0;
	}

	assert_param(tohQueueReadPtr != tohQueueWritePtr);
}


uint32_t rxIdleCount, txIdleCount;

static void updateRXTXLed() {
	if (rxIdleCount > 100 && txIdleCount > 100) {
		STM_EVAL_LEDOff(LED_RXTX);
	} else {
		STM_EVAL_LEDOn(LED_RXTX);
	}
}


enum {
	RX_SYNC,
	RX_OPERATIONAL
} rxState;

uint32_t rxBufferPtr = 0;

TODSync correctTODSyncMsg = {
	.type = TOD_SYNC,
	.pattern = TOHD_SYNC_PATTERN
};

static void handleRX() {
	if (uart2IsBreakOrError()) {
		uart2ClearBreakOrError();
		uart2ClearBreakOrError();

		rxBufferPtr = 0;
		rxState = RX_SYNC;
	}

	if (rxState == RX_SYNC) {
		STM_EVAL_LEDOn(LED_OUT_OF_SYNC);
	} else {
		STM_EVAL_LEDOff(LED_OUT_OF_SYNC);
	}

	if (uart2CanRecv()) {
		uint8_t *rxBuffer = (uint8_t*)&todQueue[todQueueWritePtr];
		rxBuffer[rxBufferPtr++] = uart2RecvChar();

		if (rxState == RX_SYNC) {
			if (rxBuffer[rxBufferPtr - 1] == ((uint8_t*)&correctTODSyncMsg)[rxBufferPtr - 1]) {
				if (rxBufferPtr == sizeof(TODSync)) {
					rxBufferPtr = 0;
					rxState = RX_OPERATIONAL;
				}

			} else {
				rxBufferPtr = 0;
			}

		} else {
			TODType msgType = todQueue[todQueueWritePtr].type;

			if (msgType == TOD_SYNC) { // This fires at receiving the first byte of the sync, the rest of the sync is handled above (in RX_SYNC)
				rxState = RX_SYNC;

			} else if (msgType < 0 || msgType >= TOD_TYPES_NUMBER) {
				rxBufferPtr = 0;
				rxState = RX_SYNC;

			} else if (rxBufferPtr == todLengthHandlers[msgType](todQueue[todQueueWritePtr])) {
				// Note that we call above a handler while the message is still only partially received. That is essentially wrong, but we assume the handlers
				// won't return size which is smaller than the preamble of the message containing the length of the the message data. Once we read the length, the
				// handler returns a correct value. Essentially it means that messages have to store the data length at the beginning and the length should be unsigned.

				incTODQueueWritePtr();
				rxBufferPtr = 0;
			}
		}

		rxIdleCount = 0;
	} else {
		rxIdleCount++;
	}

	updateRXTXLed();
}


uint32_t txBufferPtr = 0;

static void handleTX() {
	if (tohQueueReadPtr != tohQueueWritePtr && uart2CanSend()) {
		uart2SendChar(((uint8_t*)&tohQueue[tohQueueReadPtr])[txBufferPtr++]);

		TOHType msgType = tohQueue[tohQueueReadPtr].type;
		assert_param(msgType >=0 && msgType < TOH_TYPES_NUMBER);

		if (txBufferPtr == tohLengthHandlers[msgType](tohQueue[tohQueueReadPtr])) {
			incTOHQueueReadPtr();
			txBufferPtr = 0;
		}

		txIdleCount = 0;
	} else {
		txIdleCount++;
	}

	updateRXTXLed();
}



uint32_t todHandlerProgress; // 0 means no handler is currently in progress

uint32_t lastInfoButtonState; // 0 - not pressed, 1 - pressed (action pending), 2 - pressed (action already performed), 3..max - waiting
static void handleInfoButton() {
	uint32_t infoButtonState = STM_EVAL_PBGetState(BUTTON_USER);

	if (infoButtonState == 1 && lastInfoButtonState == 0) {
		lastInfoButtonState = 1;
	}

	if (lastInfoButtonState == 1 && todHandlerProgress == 0) {
		TOH *msg = &tohQueue[tohQueueWritePtr];

		msg->type = TOH_INFO;

		snprintf(msg->info.text, MAX_INFO_TEXT_LENGTH, "\n=== Info ===\nrxState: %d\n", rxState);
		msg->info.length = strlen(msg->info.text);

		incTOHQueueWritePtr();

		lastInfoButtonState = 2;
	}

	if (infoButtonState == 0 && lastInfoButtonState == 2) {
		lastInfoButtonState = 3;
	}

	if (lastInfoButtonState >= 3) {
		if (lastInfoButtonState == 1000) {
			lastInfoButtonState = 0;
		} else {
			lastInfoButtonState++;
		}
	}
}

uint32_t voluntarySyncCountdown = VOLUNATARY_SYNC_PERIOD - 1;
TOHSync tohSyncMsg = {
	.type = TOH_SYNC,
	.pattern = TOHD_SYNC_PATTERN
};
static void handleVoluntarySync() {
	if (voluntarySyncCountdown == 0) {
		if (todHandlerProgress == 0) { // If we should already sync but there is a handler in progress, we simply wait by not decrementing the handleTODSyncCountdown
			memcpy(&tohQueue[tohQueueWritePtr], &tohSyncMsg, sizeof(TOHSync));
			incTOHQueueWritePtr();

			voluntarySyncCountdown = VOLUNATARY_SYNC_PERIOD - 1;
		}
	} else {
		voluntarySyncCountdown--;
	}
}


static void handleTODSync() {
	assert_param(0); // This is unimplemented method because sync is handler already in rx routine
}

static void handleTODSendPacket() {
	incTODQueueReadPtr();
}

static void handleTODSetChannel() {
	incTODQueueReadPtr();
}

typedef void(*TODHandler)();

TODHandler todHandlers[TOD_TYPES_NUMBER] = {
	handleTODSync,
	handleTODSendPacket,
	handleTODSetChannel
};

static void handleTOD() {
	if (todQueueReadPtr != todQueueWritePtr) {
		TODType msgType = todQueue[todQueueReadPtr].type;

		assert_param(msgType >= 0 && msgType < TOD_TYPES_NUMBER);

		todHandlers[msgType]();
	}

	handleVoluntarySync();
	handleInfoButton();
}


void initSM() {
	STM_EVAL_LEDInit(LED_RXTX);
	STM_EVAL_LEDInit(LED_OUT_OF_SYNC);
	STM_EVAL_LEDInit(LED_RF_RECV);
	STM_EVAL_LEDInit(LED_RF_SEND);

	STM_EVAL_PBInit(BUTTON_USER, BUTTON_MODE_GPIO);
	uart2Init();
}

void tickSM() {
	handleRX();
	handleTOD();
	handleTX();
}

