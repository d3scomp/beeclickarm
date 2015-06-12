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
#include "GPS.h"
#include "SHT1x.h"
#include "TODQueue.h"
#include "TOHQueue.h"

class MsgHandler {
public:
	struct Properties {
		uint32_t extiLine;
		IRQn irqn;
	};

	MsgHandler(Properties& initProps, MRF24J40 &mrf, GPSL30& gps, SHT1x& sht1x, TODQueue& todQueue, TOHQueue& tohQueue);
	~MsgHandler();

	void setPriority(uint8_t irqPreemptionPriority, uint8_t irqSubPriority);
	void init();

	void runInterruptHandler();
private:
	Properties props;

	MRF24J40& mrf;
	GPSL30& gps;
	SHT1x& sht1x;
	TODQueue& todQueue;
	TOHQueue& tohQueue;

	uint8_t irqPreemptionPriority;
	uint8_t irqSubPriority;

	static void messageAvailableListenerStatic(void* obj);
	static void broadcastCompleteListenerStatic(void* obj, bool isSuccessful);
	static void recvListenerStatic(void* obj);
	static void sentenceListenerStatic(void *obj);

	void sendGPSSentence();

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
	void handleSetTxPower();
	void handleSetAddr();

	bool isSendPacketInProgress;

	bool isNewGPSSentenceAvailable;
};

#endif /* MSGHANDLER_H_ */
