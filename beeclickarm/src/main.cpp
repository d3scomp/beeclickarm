/*
 * main.cpp
 *
 *  Created on: 15. 9. 2013
 *      Author: Tomas Bures <bures@d3s.mff.cuni.cz>
 */

#include "main.h"
#include "UART.h"
#include "LED.h"
#include "Button.h"
#include "TODQueue.h"
#include "TOHQueue.h"

#include <cstdio>

uint32_t mainCycles;

PulseLED rxtxLed = PulseLED(LED::rxtx, 5);
TODQueue todQueue(UART::uart2, rxtxLed, LED::outOfSync);
TOHQueue tohQueue(UART::uart2, rxtxLed);

void handleInfoButtonInterrupt() {

	TOHMessage& msg = tohQueue.getCurrentMsgWrite();

	msg.type = TOHMessage::Type::INFO;


	std::sprintf(msg.info.text,
//			"txCount: %d\n"
//			"rxCount: %d\n"
//			"rxState: %d\n"
//			"panId: %02x%02x\n"
//			"sAddr: %02x%02x\n"
//			"channelNo: %d\n"
			"mainCycles: %lu",
//			txCount, rxCount, rxState, panId[1], panId[0], sAddr[1], sAddr[0], channelNo,
			mainCycles);

	msg.info.length = std::strlen(msg.info.text);

	tohQueue.moveToNextMsgWrite();

//	rxtxLed.pulse();
}


int main(void)
{
	NVIC_SetPriorityGrouping(NVIC_PriorityGroup_2);	// 2 bits for pre-emption priority, 2 bits for subpriority
	// this effectively disables interrupt preemption of our interrupt handlers

	/* Set SysTick to fire each 10ms */
	RCC_ClocksTypeDef RCC_Clocks;
	RCC_GetClocksFreq(&RCC_Clocks);
	SysTick_Config(RCC_Clocks.HCLK_Frequency / 100);

	LED::outOfSync.init();
	LED::rxtx.init();
	UART::uart2.init();
	tohQueue.init();
	todQueue.init();
	Button::info.init();

	Button::info.setPressedListener([]{ handleInfoButtonInterrupt(); });
//	Button::info.setPressedListener([&rxtxLed]{ rxtxLed.pulse(); });

//	char ch = '0';
//	tohdUart.setSendListener([&tohdUart, &ch]{ch = ch>'9' ? '0' : ch+1; tohdUart.send(ch);});
//	tohdUart.enableSendEvents();

	// todHandlerInterruptInit();

//	NVIC_SystemLPConfig(NVIC_LP_SLEEPONEXIT, ENABLE);
	while (1) {
//		__WFI();
		mainCycles++; // This is to measure how many times we wake up from WFI. In fact, we should never wake up.
	}
}


/*

static void todHandlerInterruptInit() {
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	// Configure Button EXTI line
	EXTI_InitStructure.EXTI_Line = TOD_INTERRUPT_LINE;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	// Enable and set Button EXTI Interrupt to the lowest priority
	NVIC_InitStructure.NVIC_IRQChannel = TOD_INTERRUPT_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

	NVIC_Init(&NVIC_InitStructure);
}

*/

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

