/*
 * MsgHandler.h
 *
 *  Created on: 17. 9. 2013
 *      Author: Tomas Bures <bures@d3s.mff.cuni.cz>
 */

#ifndef MSGHANDLER_H_
#define MSGHANDLER_H_

#include "stm32f4xx.h"
#include "MRF24J40.h"
#include "TODQueue.h"
#include "TOHQueue.h"

class MsgHandler {
public:
	struct Properties {
		uint32_t extiLine;
		IRQn irqn;
	};

	MsgHandler(Properties& initProps, MRF24J40 &mrf, TODQueue& todQueue, TOHQueue& tohQueue);
	~MsgHandler();

	void setPriority(uint8_t irqPreemptionPriority, uint8_t irqSubPriority);
	void init();

	void runInterruptHandler();
private:
	Properties props;

	MRF24J40& mrf;
	TODQueue& todQueue;
	TOHQueue& tohQueue;

	uint8_t irqPreemptionPriority;
	uint8_t irqSubPriority;

	static void messageAvailableListenerStatic(void* obj);
	static void broadcastCompleteListenerStatic(void* obj, bool isSuccessful);
	static void recvListenerStatic(void* obj);

	inline void waitOnReturn() {
		EXTI_ClearITPendingBit(props.extiLine);
	}

	inline void wakeup() {
		EXTI_GenerateSWInterrupt(props.extiLine);
	}

	typedef void (MsgHandler::*MsgHandlerOne)();
	static MsgHandlerOne msgHandlers[static_cast<int>(TODMessage::Type::count)];

	void handleSync();
	void handleSendPacket();
	void handleSetChannel();
	void handleSetAddr();

	bool isSendPacketInProgress;
};

#endif /* MSGHANDLER_H_ */
