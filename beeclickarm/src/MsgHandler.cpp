/*
 * MsgHandler.cpp
 *
 *  Created on: 17. 9. 2013
 *      Author: Tomas Bures <bures@d3s.mff.cuni.cz>
 */

#include "MsgHandler.h"
#include <cstdio>

MsgHandler* MsgHandler::runListener;

MsgHandler::MsgHandler(TODQueue& todQueue, TOHQueue& tohQueue, uint32_t extiLine, IRQn irqn) :
		todQueue(todQueue), tohQueue(tohQueue), irqPreemptionPriority(0), irqSubPriority(0), extiLine(extiLine), irqn(irqn) {
}

MsgHandler::~MsgHandler() {
}

void MsgHandler::setPriority(uint8_t irqPreemptionPriority, uint8_t irqSubPriority) {
	this->irqPreemptionPriority = irqPreemptionPriority;
	this->irqSubPriority = irqSubPriority;
}

void MsgHandler::init() {
	runListener = this;
	todQueue.setMessageAvailableListener([&,this]{ this->messageAvailableListener(); });

	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	// Configure Button EXTI line
	EXTI_InitStructure.EXTI_Line = extiLine;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	// Enable and set Button EXTI Interrupt to the lowest priority
	NVIC_InitStructure.NVIC_IRQChannel = irqn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

	NVIC_Init(&NVIC_InitStructure);
}

void MsgHandler::messageAvailableListener() {
	EXTI_GenerateSWInterrupt(extiLine);
}

void MsgHandler::moveToNextTODMsg() {
	todQueue.moveToNextMsgRead();

	__disable_irq();
	if (!todQueue.isMsgReadAvailable()) {
		EXTI_ClearITPendingBit(extiLine);
	}
	__enable_irq();
}

void MsgHandler::runInterruptHandler() {
	assert_param(runListener);
	runListener->run();
}

std::function<void(MsgHandler*)> MsgHandler::msgHandlers[static_cast<int>(TODMessage::Type::count)] {
	std::mem_fn(&MsgHandler::handleSync),
	std::mem_fn(&MsgHandler::handleSendPacket),
	std::mem_fn(&MsgHandler::handleSetChannel),
	std::mem_fn(&MsgHandler::handleSetAddr)
};

void MsgHandler::run() {
	int msgTypeOrd = static_cast<int>(todQueue.getCurrentMsgRead().type);
	assert_param(msgTypeOrd >= 0 || msgTypeOrd < static_cast<int>(TODMessage::Type::count));

	msgHandlers[msgTypeOrd](this);
}

void MsgHandler::handleSync() {
	std::memcpy(&tohQueue.getCurrentMsgWrite(), &TOHMessage::CORRECT_SYNC, sizeof(TOHMessage::Sync));

	tohQueue.moveToNextMsgWrite();
	moveToNextTODMsg();
}

void MsgHandler::handleSendPacket() {
	TODMessage::SendPacket& inMsg = todQueue.getCurrentMsgRead().sendPacket;

	// TODO

	TOHMessage::PacketSent& outMsg = tohQueue.getCurrentMsgWrite().packetSent;
	outMsg.type = TOHMessage::Type::PACKET_SENT;
	outMsg.seq[0] = inMsg.seq[0];
	outMsg.seq[1] = inMsg.seq[1];
	outMsg.seq[2] = inMsg.seq[2];
	outMsg.seq[3] = inMsg.seq[3];

	tohQueue.moveToNextMsgWrite();
	moveToNextTODMsg();
}

void MsgHandler::handleSetChannel() {
	TODMessage::SetChannel& inMsg = todQueue.getCurrentMsgRead().setChannel;

	// TODO

	TOHMessage::ChannelSet& outMsg = tohQueue.getCurrentMsgWrite().channelSet;
	outMsg.type = TOHMessage::Type::CHANNEL_SET;
	outMsg.channel = inMsg.channel;

	tohQueue.moveToNextMsgWrite();
	moveToNextTODMsg();
}

void MsgHandler::handleSetAddr() {
	TODMessage::SetAddr& inMsg = todQueue.getCurrentMsgRead().setAddr;

	// TODO

	TOHMessage::AddrSet& outMsg = tohQueue.getCurrentMsgWrite().addrSet;
	outMsg.type = TOHMessage::Type::ADDR_SET;
	outMsg.panId[0] = inMsg.panId[0];
	outMsg.panId[1] = inMsg.panId[1];
	outMsg.sAddr[0] = inMsg.sAddr[0];
	outMsg.sAddr[1] = inMsg.sAddr[1];

	tohQueue.moveToNextMsgWrite();
	moveToNextTODMsg();
}
