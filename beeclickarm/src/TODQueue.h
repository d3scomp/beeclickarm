/*
 * TODQueue.h
 *
 *  Created on: 15. 9. 2013
 *      Author: Tomas Bures <bures@d3s.mff.cuni.cz>
 */

#ifndef TODQUEUE_H_
#define TODQUEUE_H_

#include "stm32f4xx.h"
#include <cstddef>
#include <cstring>
#include "UART.h"
#include "LED.h"


struct TODMessage {
public:
	static constexpr uint8_t SYNC_PATTERN[] {'H', 'e', 'l', 'l', 'o', ' ', 't', 'h', 'e', 'r', 'e', '!'};
	static constexpr auto MAX_RF_PACKET_LENGTH = 128;

	enum class Type : uint8_t {
		SYNC, SEND_PACKET, SET_CHANNEL, SET_TXPOWER, SET_ADDR, count
	};

	struct Sync {
		Type type;
		uint8_t pattern[sizeof(SYNC_PATTERN)];
	} __attribute__((packed));

	struct SendPacket {
		Type type;
		uint8_t length;
		uint8_t seq[4];
		uint8_t data[MAX_RF_PACKET_LENGTH];
	} __attribute__((packed));

	struct SetChannel {
		Type type;
		uint8_t channel;
	} __attribute__((packed));

	struct SetTxPower {
		Type type;
		short power;
	} __attribute__((packed));

	struct SetAddr {
		Type type;
		uint8_t panId[2];
		uint8_t sAddr[2];
	};

	union {
		Type type;
		Sync sync;
		SendPacket sendPacket;
		SetChannel setChannel;
		SetTxPower setTxPower;
		SetAddr setAddr;
	};

	inline size_t getExpectedSizeLowerBound() {
		return expectedSizeHandlers[static_cast<int>(type)](*this);
	}


	struct CorrectSync : public Sync {
		CorrectSync();
	};
	static CorrectSync CORRECT_SYNC;

private:
	static size_t (*expectedSizeHandlers[static_cast<int>(Type::count)])(TODMessage&);
};


class TODQueue {
public:
	TODQueue(UART& uart, PulseLED& rxLed, LED& outOfSyncLed);
	~TODQueue();

	void init();

	inline bool isMsgReadAvailable() {
		return readIdx != writeIdx;
	}

	inline TODMessage& getCurrentMsgRead() {
		return queue[readIdx];
	}

	void moveToNextMsgRead();

	typedef void (*Listener)(void *);
	void setMessageAvailableListener(Listener messageAvailableListener, void *obj);

private:
	enum class RXState {
		SYNC, OPERATIONAL, count
	};

	UART& uart;
	PulseLED& rxLed;
	LED& outOfSyncLed;

	void moveToNextMsgWrite();

	static constexpr uint32_t MAX_QUEUE_LENGTH = 8;
	TODMessage queue[MAX_QUEUE_LENGTH];
	uint32_t readIdx; // From which position the msg is to be read (if readIdx == writeIdx then there is nothing to be read)
	uint32_t writeIdx; // To which position the msg is to be written

	Listener messageAvailableListener;
	void* messageAvailableListenerObj;

	RXState rxState;
	uint32_t rxBufferPos;
	void handleRX();
	static void recvListenerStatic(void *obj);
};

#endif /* TODQUEUE_H_ */
