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
#include "MsgHandler.h"
#include "MRF24J40.h"

#include <cstdio>

uint32_t mainCycles;

PulseLED rxtxLed = PulseLED(LED::rxtx, 5);
TODQueue todQueue(UART::uart2, rxtxLed, LED::outOfSync);
TOHQueue tohQueue(UART::uart2, rxtxLed);
MRF24J40 mrf;
MsgHandler msgHandler(mrf, todQueue, tohQueue, EXTI_Line1, EXTI1_IRQn);

void handleInfoButtonInterrupt() {

	TOHMessage::Info& msg = tohQueue.getCurrentMsgWrite().info;

	msg.type = TOHMessage::Type::INFO;


	std::sprintf(msg.text,
			"txCount: %d\n"
			"rxCount: %d\n"
			"panId: %04x\n"
			"sAddr: %04x\n"
			"channelNo: %d\n"
			"mainCycles: %lu\n",
			mrf.getTXCount(), mrf.getRXCount(), mrf.getPanId(), mrf.getSAddr(), mrf.getChannel(), mainCycles);

	msg.length = std::strlen(msg.text);

	tohQueue.moveToNextMsgWrite();
}


int main(void)
{
	NVIC_SetPriorityGrouping(NVIC_PriorityGroup_2);	// 2 bits for pre-emption priority, 2 bits for non-preemptive subpriority
	// TODO: Here comes MRF24J40 communication with priority (0,0)
	UART::uart2.setPriority(1,0);
	// TODO: Here comes MRF24J40 packet receive interrupt with priority (2,0)
	Button::info.setPriority(2,1);
	msgHandler.setPriority(2,2);

	/* Set SysTick to fire each 10ms */
	RCC_ClocksTypeDef RCC_Clocks;
	RCC_GetClocksFreq(&RCC_Clocks);
	SysTick_Config(RCC_Clocks.HCLK_Frequency / 100);

	LED::outOfSync.init();
	LED::rxtx.init();
	rxtxLed.init();

	UART::uart2.init();
	tohQueue.init();
	msgHandler.init();
	todQueue.init();
	Button::info.init();

	Button::info.setPressedListener([]{ handleInfoButtonInterrupt(); });

	NVIC_SystemLPConfig(NVIC_LP_SLEEPONEXIT, ENABLE);
	while (1) {
		__WFI();
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

