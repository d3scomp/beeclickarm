/*
 * TOHQueue.h
 *
 *  Created on: 15. 9. 2013
 *      Author: Tomas Bures <bures@d3s.mff.cuni.cz>
 */

#ifndef TOHQUEUE_H_
#define TOHQUEUE_H_

#include "stm32f4xx.h"
#include <functional>
#include "TODQueue.h"

struct TOHMessage {
public:
	static constexpr auto MAX_INFO_TEXT_LENGTH = 255;
	static constexpr auto MAX_RF_PACKET_LENGTH = TODMessage::MAX_RF_PACKET_LENGTH;

	enum class Type : uint8_t {
		SYNC, RECV_PACKET, PACKET_SENT, CHANNEL_SET, ADDR_SET, INFO, count
	};

	struct Sync {
		Type type;
		uint8_t pattern[sizeof(TODMessage::SYNC_PATTERN)];
	};

	struct RecvPacket {
		Type type;
		uint8_t length;
		uint8_t rssi;
		uint8_t lqi;
		uint8_t fcs[2];
		uint8_t data[MAX_RF_PACKET_LENGTH];
	};

	struct PacketSent {
		Type type;
		uint8_t status; // 0 - OK, 1 - error
		uint8_t seq[4];
	};

	struct ChannelSet {
		Type type;
		uint8_t channel;
	};

	struct AddrSet {
		Type type;
		uint8_t panId[2];
		uint8_t sAddr[2];
	};

	struct Info {
		Type type;
		uint8_t length;
		char text[MAX_INFO_TEXT_LENGTH];
	};

	union {
		Type type;
		Sync sync;
		RecvPacket recvPacket;
		PacketSent packetSent;
		ChannelSet channelSet;
		AddrSet addrSet;
		Info info;
	};

	inline size_t getSize() {
		return sizeHandlers[static_cast<int>(type)](*this);
	}

	struct CorrectSync : public Sync {
		CorrectSync();
	};
	static CorrectSync CORRECT_SYNC;

private:
	static std::function<size_t(TOHMessage&)> sizeHandlers[static_cast<int>(Type::count)];
};


class TOHQueue {
public:
	TOHQueue(UART& uart, PulseLED& txLed);
	~TOHQueue();

	void init();

	inline TOHMessage& getCurrentMsgWrite() {
		return queue[writeIdx];
	}

	void moveToNextMsgWrite();

private:
	UART& uart;
	PulseLED& txLed;

	void moveToNextMsgRead();

	static constexpr uint32_t MAX_QUEUE_LENGTH = 8;
	TOHMessage queue[MAX_QUEUE_LENGTH];
	uint32_t readIdx; // From which position the msg is to be read (if readIdx == writeIdx then there is nothing to be read)
	uint32_t writeIdx; // To which position the msg is to be written

	uint32_t txBufferPos;
	void handleTX();
};

#endif /* TOHQUEUE_H_ */
