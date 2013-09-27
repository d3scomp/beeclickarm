/*
 * UART.cpp
 *
 *  Created on: 14. 9. 2013
 *      Author: Tomas Bures <bures@d3s.mff.cuni.cz>
 */

#include <UART.h>

uint8_t UART::recv() {
	return (uint8_t)USART_ReceiveData(props.usart);
}

void UART::send(uint8_t ch) {
	USART_SendData(props.usart, ch);
}

bool UART::canSend() {
	return USART_GetFlagStatus(props.usart, USART_FLAG_TXE) == SET;
}

bool UART::canRecv() {
	return USART_GetFlagStatus(props.usart, USART_FLAG_RXNE) == SET;
}

bool UART::isSendComplete() {
	return USART_GetFlagStatus(props.usart, USART_FLAG_TC) == SET;
}

bool UART::isBreakOrError() {
	return props.usart->SR & (USART_FLAG_FE | USART_FLAG_ORE | USART_FLAG_PE);
}

void UART::clearBreakOrError() {
	volatile int32_t dummy = props.usart->SR;
	props.usart->DR = 0;
}

void UART::enableSendEvents() {
	USART_ITConfig(USART2, USART_IT_TXE, ENABLE);
}

void UART::disableSendEvents() {
	USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
}

void UART::setSendListener(Listener sendReadyListener) {
	this->sendListener = sendReadyListener;
}

void UART::enableRecvEvents() {
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
}

void UART::disableRecvEvents() {
	USART_ITConfig(USART2, USART_IT_RXNE, DISABLE);
}

void UART::setRecvListener(Listener recvReadyListener) {
	this->recvListener = recvReadyListener;
}

void UART::txrxInterruptHandler() {
	if (USART_GetITStatus(props.usart, USART_IT_TXE)) {
		if (sendListener) {
			sendListener();
		}
	}

	if (USART_GetITStatus(props.usart, USART_IT_RXNE)) {
		if (recvListener) {
			recvListener();
		}
	}
}


UART::UART(Properties& initProps) : props(initProps) {
}

UART::~UART() {
	USART_DeInit(props.usart);
}


void UART::setPriority(uint8_t irqPreemptionPriority, uint8_t irqSubPriority) {
	this->irqPreemptionPriority = irqPreemptionPriority;
	this->irqSubPriority = irqSubPriority;
}


void UART::init() {
	RCC_AHB1PeriphClockCmd(props.clkGPIO, ENABLE);

	GPIO_PinAFConfig(props.gpio, props.pinSourceTX, props.afConfig); // alternative function USARTx_TX
	GPIO_PinAFConfig(props.gpio, props.pinSourceRX, props.afConfig); // alternative function USARTx_RX

	GPIO_InitTypeDef gpioInitStruct;
	gpioInitStruct.GPIO_Pin = props.pinTX | props.pinRX;
	gpioInitStruct.GPIO_Mode = GPIO_Mode_AF;
	gpioInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	gpioInitStruct.GPIO_OType = GPIO_OType_PP;
	gpioInitStruct.GPIO_PuPd = GPIO_PuPd_UP;

	GPIO_Init(props.gpio, &gpioInitStruct);


	props.clkUSARTCmdFun(props.clkUSART, ENABLE);

	USART_InitTypeDef usartInitStruct;
	usartInitStruct.USART_BaudRate = 921600;
//	usartInitStruct.USART_BaudRate = 2400;
	usartInitStruct.USART_WordLength = USART_WordLength_8b;
	usartInitStruct.USART_StopBits = USART_StopBits_1;
	usartInitStruct.USART_Parity = USART_Parity_No;
	usartInitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	usartInitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;

	USART_Init(props.usart, &usartInitStruct);


	NVIC_InitTypeDef nvicInitStruct;

	// Enable the USART2 Interrupt
	nvicInitStruct.NVIC_IRQChannel = props.nvicIRQChannel;
	nvicInitStruct.NVIC_IRQChannelPreemptionPriority = irqPreemptionPriority;
	nvicInitStruct.NVIC_IRQChannelSubPriority = irqSubPriority;
	nvicInitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvicInitStruct);

	USART_Cmd(props.usart, ENABLE);
}

