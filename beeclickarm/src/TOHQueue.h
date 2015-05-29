/*
 * TOHQueue.h
 *
 *  Created on: 15. 9. 2013
 *      Author: Tomas Bures <bures@d3s.mff.cuni.cz>
 */

#ifndef TOHQUEUE_H_
#define TOHQUEUE_H_

#include "stm32f4xx.h"
#include "TODQueue.h"
#include "GPS.h"

struct TOHMessage {
public:
	static constexpr auto MAX_INFO_TEXT_LENGTH = 255;
	static constexpr auto MAX_RF_PACKET_LENGTH = TODMessage::MAX_RF_PACKET_LENGTH;

	enum class Type : uint8_t {
		SYNC, RECV_PACKET, PACKET_SENT, CHANNEL_SET, TXPOWER_SET, ADDR_SET, GPS, INFO, TEMPERATURE, HUMIDITY, count
	};

	struct Sync {
		Type type;
		uint8_t pattern[sizeof(TODMessage::SYNC_PATTERN)];
	} __attribute__((packed));

	struct RecvPacket {
		Type type;
		uint8_t length;
		uint8_t srcPanId[2];
		uint8_t srcSAddr[2];
		uint8_t rssi;
		uint8_t lqi;
		uint8_t fcs[2];
		uint8_t data[MAX_RF_PACKET_LENGTH];
	} __attribute__((packed));

	struct PacketSent {
		Type type;
		uint8_t seq[4];
		uint8_t status; // 0 - OK, 1 - error
	} __attribute__((packed));

	struct ChannelSet {
		Type type;
		uint8_t channel;
	} __attribute__((packed));

	struct TxPowerSet {
		Type type;
		short power;
	} __attribute__((packed));

	struct AddrSet {
		Type type;
		uint8_t panId[2];
		uint8_t sAddr[2];
	} __attribute__((packed));

	struct GPS {
		Type type;
		uint8_t length;
		char text[GPSL10::MAX_GPS_SENTENCE_LENGTH];
	} __attribute__((packed));

	struct Info {
		Type type;
		uint8_t length;
		char text[MAX_INFO_TEXT_LENGTH];
	} __attribute__((packed));

	struct Temperateure {
		Type type;
		int16_t temperature;
	} __attribute__((packed));

	struct Humidity {
		Type type;
		uint16_t humidity;
	} __attribute__((packed));

	union {
		Type type;
		Sync sync;
		RecvPacket recvPacket;
		PacketSent packetSent;
		ChannelSet channelSet;
		TxPowerSet txPowerSet;
		AddrSet addrSet;
		GPS gps;
		Info info;
		Temperateure temperature;
		Humidity humidity;
	};

	inline size_t getSize() {
		return sizeHandlers[static_cast<int>(type)](*this);
	}

	struct CorrectSync : public Sync {
		CorrectSync();
	};
	static CorrectSync CORRECT_SYNC;

private:
	static size_t (*sizeHandlers[static_cast<int>(Type::count)])(TOHMessage&);
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
	static void sendListenerStatic(void *obj);
};

#endif /* TOHQUEUE_H_ */
