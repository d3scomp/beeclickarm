/*
 * TODQueue.cpp
 *
 *  Created on: 15. 9. 2013
 *      Author: Tomas Bures <bures@d3s.mff.cuni.cz>
 */

#include "TODQueue.h"

size_t (*TODMessage::expectedSizeHandlers[static_cast<int>(Type::count)])(TODMessage&) {
	[](TODMessage &msg){ return sizeof(TODMessage::Sync); },
	[](TODMessage &msg){ return sizeof(TODMessage::Type) + sizeof(uint8_t) + sizeof(uint32_t) + msg.sendPacket.length; },
	[](TODMessage &msg){ return sizeof(TODMessage::SetChannel); },
	[](TODMessage &msg){ return sizeof(TODMessage::SetAddr); }
};

constexpr uint8_t TODMessage::SYNC_PATTERN[];
TODMessage::CorrectSync TODMessage::CORRECT_SYNC;

TODMessage::CorrectSync::CorrectSync() {
	type = TODMessage::Type::SYNC;
	std::memcpy(pattern, SYNC_PATTERN, sizeof(SYNC_PATTERN));
}


TODQueue::TODQueue(UART& uart, PulseLED& rxLed, LED& outOfSyncLed) : uart(uart), rxLed(rxLed), outOfSyncLed(outOfSyncLed), rxState(RXState::SYNC) {
}

TODQueue::~TODQueue() {
}

void TODQueue::init() {
	outOfSyncLed.on();

	uart.setRecvListener(recvListenerStatic, this);
	uart.enableRecvEvents();
}

void TODQueue::moveToNextMsgRead() {
	assert_param(readIdx != writeIdx);

	readIdx++;
	if (readIdx == MAX_QUEUE_LENGTH) {
		readIdx = 0;
	}
}

void TODQueue::moveToNextMsgWrite() {
	writeIdx++;
	if (writeIdx == MAX_QUEUE_LENGTH) {
		writeIdx = 0;
	}

	assert_param(readIdx != writeIdx);

	assert_param(messageAvailableListener);
	messageAvailableListener(messageAvailableListenerObj);
}

void TODQueue::setMessageAvailableListener(Listener messageAvailableListener, void* obj) {
	this->messageAvailableListener = messageAvailableListener;
	messageAvailableListenerObj = obj;
}

void TODQueue::recvListenerStatic(void *obj) {
	static_cast<TODQueue*>(obj)->handleRX();
}

void TODQueue::handleRX() {
	uint8_t *rxBuffer = (uint8_t*)&queue[writeIdx];
	rxBuffer[rxBufferPos++] = uart.recv();

	if (rxState == RXState::SYNC) {
		if (rxBuffer[rxBufferPos - 1] == ((uint8_t*)&TODMessage::CORRECT_SYNC)[rxBufferPos - 1]) {
			if (rxBufferPos == sizeof(TODMessage::Sync)) {
				rxBufferPos = 0;
				rxState = RXState::OPERATIONAL;

				moveToNextMsgWrite();
				outOfSyncLed.off();
			}

		} else {
			rxBufferPos = 0;
		}

	} else {
		TODMessage& msg = queue[writeIdx];
		TODMessage::Type msgType = msg.type;

		if (msgType == TODMessage::Type::SYNC) { // This fires at receiving the first byte of the sync, the rest of the sync is handled above (in RX_SYNC)
			rxState = RXState::SYNC;

		} else if (static_cast<int>(msgType) < 0 || static_cast<int>(msgType) >= static_cast<int>(TODMessage::Type::count)) {
			rxBufferPos = 0;
			rxState = RXState::SYNC;

		} else if (rxBufferPos == msg.getExpectedSizeLowerBound()) {
			// Note that we call above a handler while the message is still only partially received. That is essentially wrong, but we assume the handlers
			// won't return size which is smaller than the preamble of the message containing the length of the the message data. Once we read the length, the
			// handler returns a correct value. Essentially it means that messages have to store the data length at the beginning and the length should be unsigned.

			moveToNextMsgWrite();
			rxBufferPos = 0;
		}
	}

	rxLed.pulse();
}


