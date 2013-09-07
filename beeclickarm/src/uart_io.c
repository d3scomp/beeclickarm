#include "uart_io.h"
#include "stm32f4xx.h"

void uart2Init() {
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2); // PA2 - alternative function USART2_TX
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2); // PA3 - alternative function USART2_RX

	GPIO_InitTypeDef gpioInitStruct;
	gpioInitStruct.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
	gpioInitStruct.GPIO_Mode = GPIO_Mode_AF;
	gpioInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	gpioInitStruct.GPIO_OType = GPIO_OType_PP;
	gpioInitStruct.GPIO_PuPd = GPIO_PuPd_UP;

	GPIO_Init(GPIOA, &gpioInitStruct);


	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

	USART_InitTypeDef usartInitStruct;
	usartInitStruct.USART_BaudRate = 921600;
//	usartInitStruct.USART_BaudRate = 2400;
	usartInitStruct.USART_WordLength = USART_WordLength_8b;
	usartInitStruct.USART_StopBits = USART_StopBits_1;
	usartInitStruct.USART_Parity = USART_Parity_No;
	usartInitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	usartInitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;

	USART_Init(USART2, &usartInitStruct);


	NVIC_InitTypeDef nvicInitStruct;

	/* Enable the USART2 Interrupt */
	nvicInitStruct.NVIC_IRQChannel = USART2_IRQn;
	nvicInitStruct.NVIC_IRQChannelPreemptionPriority = 0;
	nvicInitStruct.NVIC_IRQChannelSubPriority = 1;
	nvicInitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvicInitStruct);

	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);


	USART_Cmd(USART2, ENABLE);
}

uint8_t uart2RecvChar() {
	return (uint8_t)USART_ReceiveData(USART2);
}

void uart2SendChar(uint8_t ch) {
	USART_SendData(USART2, ch);
}

int uart2CanSend() {
	return USART_GetFlagStatus(USART2, USART_FLAG_TXE) == SET;
}

int uart2CanRecv() {
	return USART_GetFlagStatus(USART2, USART_FLAG_RXNE) == SET;
}

int uart2SendComplete() {
	return USART_GetFlagStatus(USART2, USART_FLAG_TC) == SET;
}

int uart2IsBreakOrError() {
	return USART2->SR & (USART_FLAG_FE | USART_FLAG_ORE | USART_FLAG_PE);
}

void uart2ClearBreakOrError() {
	volatile int32_t dummy = USART2->SR;
	USART2->DR = 0;
}

void uart2TXIntEnable() {
	USART_ITConfig(USART2, USART_IT_TXE, ENABLE);
}

void uart2TXIntDisable() {
	USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
}
