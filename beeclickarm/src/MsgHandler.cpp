/*
 * MsgHandler.cpp
 *
 *  Created on: 17. 9. 2013
 *      Author: Tomas Bures <bures@d3s.mff.cuni.cz>
 */

#include "MsgHandler.h"
#include <cstdio>

MsgHandler::MsgHandler(Properties& initProps, MRF24J40 &mrf, TODQueue& todQueue, TOHQueue& tohQueue) :
		props(initProps), mrf(mrf), todQueue(todQueue), tohQueue(tohQueue) {
}

MsgHandler::~MsgHandler() {
}

void MsgHandler::setPriority(uint8_t irqPreemptionPriority, uint8_t irqSubPriority) {
	this->irqPreemptionPriority = irqPreemptionPriority;
	this->irqSubPriority = irqSubPriority;
}

void MsgHandler::init() {
	todQueue.setMessageAvailableListener([&,this]{ this->messageAvailableListener(); });
	mrf.setBroadcastCompleteListener([&,this](bool isSuccessful){ this->broadcastCompleteListener(isSuccessful); });

	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	// Configure Button EXTI line
	EXTI_InitStructure.EXTI_Line = props.extiLine;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	// Enable and set Button EXTI Interrupt to the lowest priority
	NVIC_InitStructure.NVIC_IRQChannel = props.irqn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = irqPreemptionPriority;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = irqSubPriority;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

	NVIC_Init(&NVIC_InitStructure);
}

inline void MsgHandler::waitOnReturn() {
	EXTI_ClearITPendingBit(props.extiLine);
}

inline void MsgHandler::wakeup() {
	EXTI_GenerateSWInterrupt(props.extiLine);
}

void MsgHandler::messageAvailableListener() {
	wakeup();
}

std::function<void(MsgHandler*)> MsgHandler::msgHandlers[static_cast<int>(TODMessage::Type::count)] {
	std::mem_fn(&MsgHandler::handleSync),
	std::mem_fn(&MsgHandler::handleSendPacket),
	std::mem_fn(&MsgHandler::handleSetChannel),
	std::mem_fn(&MsgHandler::handleSetAddr)
};

void MsgHandler::runInterruptHandler() {
	if (todQueue.isMsgReadAvailable()) {
		int msgTypeOrd = static_cast<int>(todQueue.getCurrentMsgRead().type);
		assert_param(msgTypeOrd >= 0 || msgTypeOrd < static_cast<int>(TODMessage::Type::count));

		msgHandlers[msgTypeOrd](this);
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

void MsgHandler::broadcastCompleteListener(bool isSuccessful) {
	TODMessage::SendPacket& inMsg = todQueue.getCurrentMsgRead().sendPacket;
	TOHMessage::PacketSent& outMsg = tohQueue.getCurrentMsgWrite().packetSent;
	outMsg.type = TOHMessage::Type::PACKET_SENT;
	outMsg.seq[0] = inMsg.seq[0];
	outMsg.seq[1] = inMsg.seq[1];
	outMsg.seq[2] = inMsg.seq[2];
	outMsg.seq[3] = inMsg.seq[3];
	outMsg.status = !isSuccessful;

	tohQueue.moveToNextMsgWrite();
	todQueue.moveToNextMsgRead();

	isSendPacketInProgress = false;
	wakeup();
}

void MsgHandler::recvListener() {
	TOHMessage::RecvPacket& outMsg = tohQueue.getCurrentMsgWrite().recvPacket;
	outMsg.type = TOHMessage::Type::RECV_PACKET;
	mrf.recvPacket(outMsg.data, outMsg.length, outMsg.fcs, outMsg.lqi, outMsg.rssi);

	tohQueue.moveToNextMsgWrite();
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
