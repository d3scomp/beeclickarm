/*
 * TOHQueue.cpp
 *
 *  Created on: 15. 9. 2013
 *      Author: Tomas Bures <bures@d3s.mff.cuni.cz>
 */

#include "TOHQueue.h"


std::function<size_t(TOHMessage&)> TOHMessage::sizeHandlers[static_cast<int>(Type::count)] {
	[](TOHMessage &msg){ return sizeof(Sync); },
	[](TOHMessage &msg){ return sizeof(Type) + sizeof(uint8_t) * 4 + msg.recvPacket.length; },
	[](TOHMessage &msg){ return sizeof(PacketSent); },
	[](TOHMessage &msg){ return sizeof(ChannelSet); },
	[](TOHMessage &msg){ return sizeof(AddrSet); },
	[](TOHMessage &msg){ return sizeof(Type) + sizeof(uint8_t) + msg.info.length; }
};

TOHMessage::CorrectSync TOHMessage::CORRECT_SYNC;

TOHMessage::CorrectSync::CorrectSync() {
	type = TOHMessage::Type::SYNC;
	std::memcpy(pattern, TODMessage::SYNC_PATTERN, sizeof(TODMessage::SYNC_PATTERN));
}


TOHQueue::TOHQueue(UART& uart, PulseLED& txLed) : uart(uart), txLed(txLed), readIdx(0), writeIdx(0), txBufferPos(0) {
}

TOHQueue::~TOHQueue() {
}

void TOHQueue::init() {
	uart.setSendListener([&,this]{ this->handleTX(); });
}

void TOHQueue::moveToNextMsgWrite() {
	writeIdx++;
	if (writeIdx == MAX_QUEUE_LENGTH) {
		writeIdx = 0;
	}

	assert_param(readIdx != writeIdx);

	uart.enableSendEvents();
}

void TOHQueue::moveToNextMsgRead() {
	assert_param(readIdx != writeIdx);

	readIdx++;
	if (readIdx == MAX_QUEUE_LENGTH) {
		readIdx = 0;
	}

	if (readIdx == writeIdx) {
		uart.disableSendEvents();	// Note that this is not a race-condition because all defined interrupt handlers
								// run in the same priority group, thus they don't preempt one another
	}
}

void TOHQueue::handleTX() {
	if (readIdx != writeIdx && uart.canSend()) {
		uint8_t *txBuffer = (uint8_t*)&queue[readIdx];
		uart.send(txBuffer[txBufferPos++]);

		TOHMessage& msg = queue[readIdx];
		assert_param(static_cast<int>(msg.type) >= 0 || static_cast<int>(msg.type) < static_cast<int>(TOHMessage::Type::count));

		if (txBufferPos == msg.getSize()) {
			txBufferPos = 0;
			moveToNextMsgRead();
		}

		txLed.pulse();
	}
}
