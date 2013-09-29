/*
 * main.cpp
 *
 *  Created on: 15. 9. 2013
 *      Author: Tomas Bures <bures@d3s.mff.cuni.cz>
 */

#include "main.h"

#include <cstdio>

uint32_t mainCycles;

Timer::Properties tim6Props {
	TIM6, RCC_APB1PeriphClockCmd, RCC_APB1Periph_TIM6, TIM6_DAC_IRQn
};
Timer delayTimer(tim6Props);

/*
LED::Properties test1LedProps {
	GPIOA, GPIO_Pin_1, RCC_AHB1Periph_GPIOA
};
LED test1Led(test1LedProps);

LED::Properties test2LedProps {
	GPIOA, GPIO_Pin_5, RCC_AHB1Periph_GPIOA
};
LED test2Led(test2LedProps);
*/

LED::Properties greenLedProps {
	GPIOD, GPIO_Pin_12, RCC_AHB1Periph_GPIOD
};
LED::Properties redLedProps {
	GPIOD, GPIO_Pin_14, RCC_AHB1Periph_GPIOD
};
LED::Properties orangeLedProps {
	GPIOD, GPIO_Pin_13, RCC_AHB1Periph_GPIOD
};
LED::Properties blueLedProps {
	GPIOD, GPIO_Pin_15, RCC_AHB1Periph_GPIOD
};
LED rxtxLed(greenLedProps);
LED outOfSyncLed(redLedProps);
LED mrfRecvLed(blueLedProps);
LED mrfSendLed(orangeLedProps);

PulseLED rxtxPulseLed(rxtxLed, 1);
PulseLED mrfRecvPulseLed(mrfRecvLed, 1);
PulseLED mrfSendPulseLed(mrfSendLed, 1);

Button::Properties userButtonProps {
	GPIOA, GPIO_Pin_0, RCC_AHB1Periph_GPIOA, EXTI_Line0, EXTI_PortSourceGPIOA, EXTI_PinSource0, EXTI0_IRQn
};
Button infoButton(userButtonProps);

MRF24J40::Properties mrfProps {
	GPIOE, GPIOE, GPIOB, GPIOD,
	SPI3,
	GPIO_Pin_4, GPIO_Pin_5, GPIO_Pin_3, GPIO_Pin_4, GPIO_Pin_5, GPIO_Pin_2,
	GPIO_PinSource4, GPIO_PinSource5, GPIO_PinSource3, GPIO_PinSource4, GPIO_PinSource5,
	RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOE | RCC_AHB1Periph_GPIOD,
	RCC_APB1PeriphClockCmd, RCC_APB1Periph_SPI3,
	GPIO_AF_SPI3,
	EXTI_Line2, EXTI_PortSourceGPIOD, EXTI_PinSource2, EXTI2_IRQn,
	SPI3_IRQn
};
MRF24J40 mrf(mrfProps, mrfRecvPulseLed, mrfSendPulseLed);

UART::Properties uart2Props {
	GPIOA, USART2,
	GPIO_Pin_2, GPIO_Pin_3, GPIO_PinSource2, GPIO_PinSource3,
	RCC_APB1PeriphClockCmd, RCC_AHB1Periph_GPIOA, RCC_APB1Periph_USART2, GPIO_AF_USART2, USART2_IRQn,
	921600
};
UART uartTOHD(uart2Props);

UART::Properties uart6Props {
	GPIOC, USART6,
	GPIO_Pin_6, GPIO_Pin_7, GPIO_PinSource6, GPIO_PinSource7,
	RCC_APB2PeriphClockCmd, RCC_AHB1Periph_GPIOC, RCC_APB2Periph_USART6, GPIO_AF_USART6, USART6_IRQn,
	9600
};
UART uartGPS(uart6Props);

UART::Properties uart3Props {
	GPIOB, USART3,
	GPIO_Pin_10, GPIO_Pin_11, GPIO_PinSource10, GPIO_PinSource11,
	RCC_APB1PeriphClockCmd, RCC_AHB1Periph_GPIOB, RCC_APB1Periph_USART3, GPIO_AF_USART3, USART3_IRQn,
	4800
};
UART uartGPS2(uart3Props);

GPSL10 gps(uartGPS);

GPSL30::Properties gps2Props {
	GPIOC, GPIOE, GPIOE,
	GPIO_Pin_2, GPIO_Pin_12, GPIO_Pin_13,
	RCC_AHB1Periph_GPIOC, RCC_AHB1Periph_GPIOE, RCC_AHB1Periph_GPIOE
};
GPSL30 gps2(gps2Props, uartGPS2);

TODQueue todQueue(uartTOHD, rxtxPulseLed, outOfSyncLed);
TOHQueue tohQueue(uartTOHD, rxtxPulseLed);

MsgHandler::Properties msgHandlerProps {
	EXTI_Line1, EXTI1_IRQn
};
MsgHandler msgHandler(msgHandlerProps, mrf, gps, gps2, todQueue, tohQueue);

void handleInfoButtonInterrupt(void*) {
	TOHMessage::Info& msg = tohQueue.getCurrentMsgWrite().info;

	msg.type = TOHMessage::Type::INFO;

	std::sprintf(msg.text,
			"txCount: %d\n"
			"rxCount: %d\n"
			"panId: %04x\n"
			"sAddr: %04x\n"
			"channelNo: %d\n"
			"mainCycles: %lu\n"
			"GPS: %s\n"
			"GPS2: %s\n",
			mrf.getTXCount(), mrf.getRXCount(), mrf.readPANId(), mrf.readSAddr(), mrf.readChannel(), mainCycles, gps.getSentence(), gps2.getSentence());

	msg.length = std::strlen(msg.text);

	tohQueue.moveToNextMsgWrite();
}

int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	// 2 bits for pre-emption priority, 2 bits for non-preemptive subpriority
	mrf.setSPIPriority(0,0);
	uartTOHD.setPriority(1,0);
	delayTimer.setPriority(1,1);
	uartGPS.setPriority(1,2);
	mrf.setRFPriority(2,0);
	NVIC_SetPriority(SysTick_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 2, 1));
	infoButton.setPriority(2,1);
	msgHandler.setPriority(2,2);

	/* Set SysTick to fire each 10ms */
	RCC_ClocksTypeDef RCC_Clocks;
	RCC_GetClocksFreq(&RCC_Clocks);
	SysTick_Config(RCC_Clocks.HCLK_Frequency / 100);


	delayTimer.init();

//	test1Led.init();
//	test2Led.init();

	outOfSyncLed.init();
	rxtxLed.init();
	mrfSendLed.init();
	mrfRecvLed.init();
	rxtxPulseLed.init();
	mrfSendPulseLed.init();
	mrfRecvPulseLed.init();

	uartTOHD.init();
	mrf.init();
	tohQueue.init();
	msgHandler.init();
	todQueue.init();

	uartGPS.init();
	gps.init();

	uartGPS2.init();
	gps2.init();

	infoButton.setPressedListener(handleInfoButtonInterrupt, nullptr);
	infoButton.init();


	NVIC_SystemLPConfig(NVIC_LP_SLEEPONEXIT, ENABLE); // This ..
	while (1) {
		__WFI(); // ... and this has to be commented out when debugging.
		mainCycles++; // This is to measure how many times we wake up from WFI. In fact, we should never wake up.
	}
}


#ifdef  USE_FULL_ASSERT

/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t* file, uint32_t line)
{
	/* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

	/* Infinite loop */
	while (1)
	{
	}
}
#endif

