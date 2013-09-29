/*
 * GPS.cpp
 *
 *  Created on: 28. 9. 2013
 *      Author: Tomas Bures <bures@d3s.mff.cuni.cz>
 */

#include "GPS.h"
#include "main.h"
#include <cstring>

GPSL10::GPSL10(UART& uart) : uart(uart) {
}

GPSL10::~GPSL10() {
}

void GPSL10::init() {
	gpsSentence[0][0] = 0;
	gpsSentence[1][0] = 0;
	uart.setRecvListener(uartRecvListenerStatic, this);
	uart.enableRecvEvents();
}

void GPSL10::uartRecvListenerStatic(void* obj) {
	static_cast<GPSL10*>(obj)->uartRecvListener();
}

void GPSL10::uartRecvListener() {
	uint8_t data = uart.recv();
	char *workBuf = gpsSentence[workGPSSentenceIdx];

	if (data == 10) {
	} else if (data == 13) {
		workBuf[workGPSSentenceBufPos] = 0;
		workGPSSentenceBufPos = 0;

		if (!std::strncmp(workBuf, "$GPRMC", 6)) {
			validGPSSentenceIdx = workGPSSentenceIdx;
			workGPSSentenceIdx = (workGPSSentenceIdx == 0 ? 1 : 0);

			sentenceListener(sentenceListenerObj);
		}
	} else {
		if (workGPSSentenceBufPos < MAX_GPS_SENTENCE_LENGTH - 1) {
			workBuf[workGPSSentenceBufPos++] = data;
		}
	}
}

void GPSL10::setSentenceListener(SentenceListener sentenceListener, void* obj) {
	this->sentenceListener = sentenceListener;
	sentenceListenerObj = obj;
}

GPSL30::GPSL30(Properties& initProps, UART& uart) : GPSL10(uart), props(initProps) {
}

GPSL30::~GPSL30() {
}

void GPSL30::init() {
	GPIO_InitTypeDef  gpioInitStruct;

	RCC_AHB1PeriphClockCmd(props.clkPWR, ENABLE);
	RCC_AHB1PeriphClockCmd(props.clkRST, ENABLE);
	RCC_AHB1PeriphClockCmd(props.clkWUP, ENABLE);

	gpioInitStruct.GPIO_Mode = GPIO_Mode_OUT;
	gpioInitStruct.GPIO_OType = GPIO_OType_PP;
	gpioInitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	gpioInitStruct.GPIO_Speed = GPIO_Speed_50MHz;

	gpioInitStruct.GPIO_Pin = props.pinRST;
	GPIO_Init(props.gpioRST, &gpioInitStruct);
	GPIO_ResetBits(props.gpioRST, props.pinRST);
	delayTimer.mDelay(1);
	GPIO_SetBits(props.gpioRST, props.pinRST);
	delayTimer.mDelay(1);

	gpioInitStruct.GPIO_Pin = props.pinPWR;
	GPIO_Init(props.gpioPWR, &gpioInitStruct);

	gpioInitStruct.GPIO_Mode = GPIO_Mode_IN;
	gpioInitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;

	gpioInitStruct.GPIO_Pin = props.pinWUP;
	GPIO_Init(props.gpioWUP, &gpioInitStruct);


	GPSL10::init();

	while (!GPIO_ReadInputDataBit(props.gpioWUP, props.pinWUP)) {
		GPIO_ResetBits(props.gpioPWR, props.pinPWR);
		delayTimer.mDelay(1);
		GPIO_SetBits(props.gpioPWR, props.pinPWR);
		delayTimer.mDelay(1);
	}
}
