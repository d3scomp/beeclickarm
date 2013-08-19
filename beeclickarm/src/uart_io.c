#include "uart_io.h"
#include "stm32f4xx.h"

void uartIOInit() {
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2); // PA2 - alternative function USART2_TX

	GPIO_InitTypeDef gpioInitStruct;
	gpioInitStruct.GPIO_Pin = GPIO_Pin_2;
	gpioInitStruct.GPIO_Mode = GPIO_Mode_AF;
	gpioInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	gpioInitStruct.GPIO_OType = GPIO_OType_PP;
	gpioInitStruct.GPIO_PuPd = GPIO_PuPd_UP;

	GPIO_Init(GPIOA, &gpioInitStruct);


	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

	USART_InitTypeDef usartInitStruct;
	usartInitStruct.USART_BaudRate = 921600;
	usartInitStruct.USART_WordLength = USART_WordLength_8b;
	usartInitStruct.USART_StopBits = USART_StopBits_1;
	usartInitStruct.USART_Parity = USART_Parity_No;
	usartInitStruct.USART_Mode = USART_Mode_Tx;
	usartInitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;

	USART_Init(USART2, &usartInitStruct);

	USART_Cmd(USART2, ENABLE);
}

int syscallWriteHandler(char *ptr, int len) {
	int DataIdx;

	for (DataIdx = 0; DataIdx < len; DataIdx++)	{
		USART_SendData(USART2, (uint8_t)*ptr);
		ITM_SendChar((uint8_t)*ptr); // Sends it to ST-Link SWO as well so that it can be observed in ST-Link Utility

		while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET) {
		}

		ptr++;
	}

	while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET) {
	}

	return len;
}

