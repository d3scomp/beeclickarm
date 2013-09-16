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
#include <list>

class MsgHandler {
public:
	// TODO: Add priorities
	MsgHandler(TODQueue& todQueue, TOHQueue& tohQueue);
	~MsgHandler();

	void init();

private:
	void run();

	void newTODMsg();
	void moveToNextTODMsg();

	static std::list<MsgHandler*> runListeners;
	static void runInterruptHandler();

	friend void EXTI1_IRQHandler();

	static std::function<void()> msgHandlers[static_cast<int>(TODMessage::Type::count)];
};

#endif /* MSGHANDLER_H_ */
