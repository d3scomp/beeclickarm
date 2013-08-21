#include "machine.h"
#include <stm32f4_discovery.h>
#include <string.h>
#include "uart_io.h"


#define LED_RXTX LED3
#define LED_OUT_OF_SYNC LED4
#define LED_RF_RECV LED5
#define LED_RF_SEND LED6


#define MAX_TOD_QUEUE_LENGTH 8
#define MAX_TOH_QUEUE_LENGTH 8
#define MAX_RF_PACKET_LENGTH 128

#define TOHD_SYNC_PATTERN "Hello there!"
#define TOHD_SYNC_PATTERN_LENGTH 12

#define VOLUNATARY_SYNC_PERIOD 5000 // Every nth period, we sent SYNC just by ourselves

typedef enum {
	TOD_SYNC_REQUESTED,
	TOD_SYNC,
	TOD_SYNC_REQUEST_NEEDED,
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
	uint32_t seq;
	uint8_t data[MAX_RF_PACKET_LENGTH];
	uint8_t length;
} TODSendPacket;

typedef struct {
	TODType type;
	uint8_t channel;
} TODSetChannel;

typedef union {
	TODType type;
	TODSync init;
	TODSendPacket sendPacket;
	TODSetChannel setChannel;
} TOD;

uint8_t TODMsgLengths[TOD_TYPES_NUMBER] = {
	sizeof(TODType), // TOD_SYNC_REQUESTED
	sizeof(TODSync),
	sizeof(TODType), // TOD_SYNC_REQUEST_NEEDED
	sizeof(TODSendPacket),
	sizeof(TODSetChannel)
};


typedef enum {
	TOH_SYNC_REQUESTED,
	TOH_SYNC,
	TOH_RECV_PACKET,
	TOH_PACKET_SENT,
	TOH_CHANNEL_SET,
	TOH_TYPES_NUMBER
} TOHType;

typedef struct {
	TOHType type;
	uint8_t pattern[TOHD_SYNC_PATTERN_LENGTH];
} TOHInit;

typedef struct {
	TOHType type;
	uint8_t data[MAX_RF_PACKET_LENGTH];
	uint8_t length;
	uint8_t rssi;
	uint8_t lqi;
	uint8_t fcs;
} TOHRecvPacket;

typedef struct {
	TOHType type;
	uint32_t seq;
} TOHPacketSent;

typedef struct {
	TOHType type;
	uint8_t channel;
} TOHChannelSet;

typedef union {
	TOHType type;
	TOHInit init;
	TOHRecvPacket recvPacket;
	TOHPacketSent packetSent;
	TOHChannelSet channelSet;
} TOH;

uint8_t TOHMsgLengths[TOH_TYPES_NUMBER] = {
	sizeof(TOHType), // TOD_INIT_REQUESTED
	sizeof(TOHInit),
	sizeof(TOHRecvPacket),
	sizeof(TOHPacketSent),
	sizeof(TOHChannelSet)
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
	if (rxIdleCount > 10 && txIdleCount > 10) {
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

TODSync correctTODInitMsg = {
	.type = TOD_SYNC,
	.pattern = TOHD_SYNC_PATTERN
};

static void rxPanic() {
	rxBufferPtr = 0;
	rxState = RX_SYNC;

	todQueue[todQueueWritePtr].type = TOD_SYNC_REQUEST_NEEDED;
	incTODQueueWritePtr();

	STM_EVAL_LEDOn(LED_OUT_OF_SYNC);
}

static void handleRX() {
	if (uart2IsBreakOrError()) {
		uart2ClearBreakOrError();
		uart2ClearBreakOrError();

		rxPanic();
	}

	if (uart2CanRecv()) {
		uint8_t *rxBuffer = (uint8_t*)&todQueue[todQueueWritePtr];
		rxBuffer[rxBufferPtr++] = uart2RecvChar();

		if (rxState == RX_SYNC) {

			if (rxBuffer[rxBufferPtr - 1] == ((uint8_t*)&correctTODInitMsg)[rxBufferPtr - 1]) {
				if (rxBufferPtr == sizeof(TODSync)) {
					rxBufferPtr = 0;
					rxState = RX_OPERATIONAL;

					STM_EVAL_LEDOff(LED_OUT_OF_SYNC);
				}

			} else {
				rxBufferPtr = 0;
				STM_EVAL_LEDOn(LED_OUT_OF_SYNC);
			}

		} else {
			TODType msgType = todQueue[todQueueWritePtr].type;

			if (msgType == TOD_SYNC) { // This fires at receiving the first byte of the sync, the rest of the sync is handled above (in RX_SYNC)
				rxState = RX_SYNC;

			} else if (msgType < 0 || msgType >= TOD_TYPES_NUMBER) {
				rxPanic();

			} else if (rxBufferPtr == TODMsgLengths[msgType]) {
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

		if (txBufferPtr == TOHMsgLengths[msgType]) {
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
uint32_t handleTODSyncCountdown = VOLUNATARY_SYNC_PERIOD - 1;

static void handleTODSyncRequested() {
	incTODQueueReadPtr();
}

static void handleTODSync() {
	assert_param(0); // This is unimplemented method because sync is handler already in rx routine
}

static void handleTODSyncRequestNeeded() {
	tohQueue[tohQueueWritePtr].type = TOH_SYNC_REQUESTED;
	incTOHQueueWritePtr();

	incTODQueueReadPtr();
}

static void handleTODSendPacket() {
	incTODQueueReadPtr();
}

static void handleTODSetChannel() {
	incTODQueueReadPtr();
}

typedef void(*TODHandler)();

TODHandler todHandlers[TOD_TYPES_NUMBER] = {
	handleTODSyncRequested,
	handleTODSync,
	handleTODSyncRequestNeeded,
	handleTODSendPacket,
	handleTODSetChannel
};

static void handleTOD() {
	if (todQueueReadPtr != todQueueWritePtr) {
		TODType msgType = todQueue[todQueueReadPtr].type;

		assert_param(msgType >= 0 && msgType < TOD_TYPES_NUMBER);

		todHandlers[msgType]();
	}

	if (handleTODSyncCountdown == 0) {
		if (todHandlerProgress == 0) { // If we should already sync but there is a handler in progress, we simply wait by not decrementing the handleTODSyncCountdown
			tohQueue[tohQueueWritePtr].type = TOH_SYNC;
			incTOHQueueWritePtr();

			handleTODSyncCountdown = VOLUNATARY_SYNC_PERIOD - 1;
		}
	} else {
		handleTODSyncCountdown--;
	}
}


void initSM() {
	STM_EVAL_LEDInit(LED_RXTX);
	STM_EVAL_LEDInit(LED_OUT_OF_SYNC);
	STM_EVAL_LEDInit(LED_RF_RECV);
	STM_EVAL_LEDInit(LED_RF_SEND);

	uart2Init();
}

void tickSM() {
	handleRX();
	handleTOD();
	handleTX();
}

