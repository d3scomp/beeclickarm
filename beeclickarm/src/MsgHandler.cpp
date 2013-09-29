/*
 * MsgHandler.cpp
 *
 *  Created on: 17. 9. 2013
 *      Author: Tomas Bures <bures@d3s.mff.cuni.cz>
 */

#include "MsgHandler.h"
#include <cstdio>

MsgHandler::MsgHandler(Properties& initProps, MRF24J40 &mrf, GPSL10& gps, GPSL10& gps2, TODQueue& todQueue, TOHQueue& tohQueue) :
		props(initProps), mrf(mrf), gps(gps), gps2(gps2), todQueue(todQueue), tohQueue(tohQueue) {
}

MsgHandler::~MsgHandler() {
}

void MsgHandler::setPriority(uint8_t irqPreemptionPriority, uint8_t irqSubPriority) {
	this->irqPreemptionPriority = irqPreemptionPriority;
	this->irqSubPriority = irqSubPriority;
}

void MsgHandler::init() {
	todQueue.setMessageAvailableListener(messageAvailableListenerStatic, this);
	mrf.setRecvListener(recvListenerStatic, this);
	mrf.setBroadcastCompleteListener(broadcastCompleteListenerStatic, this);
	gps.setSentenceListener(sentenceListenerStatic, this);
	gps2.setSentenceListener(sentenceListenerStatic, this);

	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	// Configure trigger EXTI line
	EXTI_InitStructure.EXTI_Line = props.extiLine;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	// Enable and set trigger EXTI Interrupt to the lowest priority
	NVIC_InitStructure.NVIC_IRQChannel = props.irqn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = irqPreemptionPriority;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = irqSubPriority;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

	NVIC_Init(&NVIC_InitStructure);
}

void MsgHandler::messageAvailableListenerStatic(void *obj) {
	MsgHandler* ths = static_cast<MsgHandler*>(obj);
	ths->wakeup();
}

void MsgHandler::sendGPSSentence() {
	TOHMessage::Info& msg = tohQueue.getCurrentMsgWrite().info;

	msg.type = TOHMessage::Type::INFO;
	std::sprintf(msg.text,
		"GPSL10: %s\n"
		"GPSL30: %s\n",
		gps.getSentence(),
		gps2.getSentence());
	msg.length = std::strlen(msg.text);

	tohQueue.moveToNextMsgWrite();
}


MsgHandler::MsgHandlerOne MsgHandler::msgHandlers[static_cast<int>(TODMessage::Type::count)] {
	&MsgHandler::handleSync,
	&MsgHandler::handleSendPacket,
	&MsgHandler::handleSetChannel,
	&MsgHandler::handleSetAddr
};

void MsgHandler::runInterruptHandler() {
	if (isNewGPSSentenceAvailable) {
		sendGPSSentence();

		isNewGPSSentenceAvailable = false;
	}

	if (todQueue.isMsgReadAvailable()) {
		int msgTypeOrd = static_cast<int>(todQueue.getCurrentMsgRead().type);
		assert_param(msgTypeOrd >= 0 || msgTypeOrd < static_cast<int>(TODMessage::Type::count));

		(this->*(msgHandlers[msgTypeOrd]))();
	}

	waitOnReturn();

	if (!isSendPacketInProgress && todQueue.isMsgReadAvailable()) {
		wakeup();
	}
}

void MsgHandler::handleSync() {
	std::memcpy(&tohQueue.getCurrentMsgWrite(), &TOHMessage::CORRECT_SYNC, sizeof(TOHMessage::Sync));

	tohQueue.moveToNextMsgWrite();
	todQueue.moveToNextMsgRead();
}

void MsgHandler::handleSendPacket() {
	if (!isSendPacketInProgress) {
		TODMessage::SendPacket& inMsg = todQueue.getCurrentMsgRead().sendPacket;

		mrf.broadcastPacket(inMsg.data, inMsg.length);

		isSendPacketInProgress = true;
	}
}

void MsgHandler::sentenceListenerStatic(void* obj) {
	MsgHandler* ths = static_cast<MsgHandler*>(obj);
	ths->isNewGPSSentenceAvailable = true;
	ths->wakeup();
}

void MsgHandler::broadcastCompleteListenerStatic(void *obj, bool isSuccessful) {
	MsgHandler* ths = static_cast<MsgHandler*>(obj);
	TODMessage::SendPacket& inMsg = ths->todQueue.getCurrentMsgRead().sendPacket;
	TOHMessage::PacketSent& outMsg = ths->tohQueue.getCurrentMsgWrite().packetSent;
	outMsg.type = TOHMessage::Type::PACKET_SENT;
	outMsg.seq[0] = inMsg.seq[0];
	outMsg.seq[1] = inMsg.seq[1];
	outMsg.seq[2] = inMsg.seq[2];
	outMsg.seq[3] = inMsg.seq[3];
	outMsg.status = !isSuccessful;

	ths->tohQueue.moveToNextMsgWrite();
	ths->todQueue.moveToNextMsgRead();

	ths->isSendPacketInProgress = false;
	ths->wakeup();
}

void MsgHandler::recvListenerStatic(void* obj) {
	MsgHandler* ths = static_cast<MsgHandler*>(obj);

	TOHMessage::RecvPacket& outMsg = ths->tohQueue.getCurrentMsgWrite().recvPacket;
	outMsg.type = TOHMessage::Type::RECV_PACKET;
	bool successfulReception = ths->mrf.recvPacket(outMsg.data, outMsg.length, outMsg.srcPanId, outMsg.srcSAddr, outMsg.fcs, outMsg.lqi, outMsg.rssi);

	if (successfulReception) {
		ths->tohQueue.moveToNextMsgWrite();
	}
}

void MsgHandler::handleSetChannel() {
	TODMessage::SetChannel& inMsg = todQueue.getCurrentMsgRead().setChannel;

	mrf.setChannel(inMsg.channel);

	TOHMessage::ChannelSet& outMsg = tohQueue.getCurrentMsgWrite().channelSet;
	outMsg.type = TOHMessage::Type::CHANNEL_SET;
	outMsg.channel = inMsg.channel;

	tohQueue.moveToNextMsgWrite();
	todQueue.moveToNextMsgRead();
}

void MsgHandler::handleSetAddr() {
	TODMessage::SetAddr& inMsg = todQueue.getCurrentMsgRead().setAddr;

	mrf.setPANId(inMsg.panId);
	mrf.setSAddr(inMsg.sAddr);

	TOHMessage::AddrSet& outMsg = tohQueue.getCurrentMsgWrite().addrSet;
	outMsg.type = TOHMessage::Type::ADDR_SET;
	outMsg.panId[0] = inMsg.panId[0];
	outMsg.panId[1] = inMsg.panId[1];
	outMsg.sAddr[0] = inMsg.sAddr[0];
	outMsg.sAddr[1] = inMsg.sAddr[1];

	tohQueue.moveToNextMsgWrite();
	todQueue.moveToNextMsgRead();
}
