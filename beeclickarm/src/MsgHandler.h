/*
 * MsgHandler.h
 *
 *  Created on: 17. 9. 2013
 *      Author: Tomas Bures <bures@d3s.mff.cuni.cz>
 */

#ifndef MSGHANDLER_H_
#define MSGHANDLER_H_

#include "stm32f4xx.h"
#include "TODQueue.h"
#include "TOHQueue.h"
#include <functional>

class MsgHandler {
public:
	MsgHandler(TODQueue& todQueue, TOHQueue& tohQueue, uint32_t extiLine, IRQn irqn);
	~MsgHandler();

	void setPriority(uint8_t irqPreemptionPriority, uint8_t irqSubPriority);
	void init();

private:
	TODQueue& todQueue;
	TOHQueue& tohQueue;

	uint8_t irqPreemptionPriority;
	uint8_t irqSubPriority;

	uint32_t extiLine;
	IRQn irqn;

	void messageAvailableListener();

	void moveToNextTODMsg();

	static MsgHandler* runListener;
	static void runInterruptHandler();

	friend void EXTI1_IRQHandler();

	static std::function<void(MsgHandler*)> msgHandlers[static_cast<int>(TODMessage::Type::count)];

	void run();

	void handleSync();
	void handleSendPacket();
	void handleSetChannel();
	void handleSetAddr();
};

#endif /* MSGHANDLER_H_ */
