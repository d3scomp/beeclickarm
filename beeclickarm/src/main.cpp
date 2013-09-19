/*
 * main.cpp
 *
 *  Created on: 15. 9. 2013
 *      Author: Tomas Bures <bures@d3s.mff.cuni.cz>
 */

#include "main.h"

#include <cstdio>

uint32_t mainCycles;

Timer delayTimer(TIM6, RCC_APB1Periph_TIM6);

LED rxtxLed(GPIOD, GPIO_Pin_12, RCC_AHB1Periph_GPIOD);
LED outOfSyncLed(GPIOD, GPIO_Pin_14, RCC_AHB1Periph_GPIOD);
LED mrfRecvLed(GPIOD, GPIO_Pin_13, RCC_AHB1Periph_GPIOD);
LED mrfSendLed(GPIOD, GPIO_Pin_15, RCC_AHB1Periph_GPIOD);

PulseLED rxtxPulseLed(rxtxLed, 5);

Button infoButton(GPIOA, GPIO_Pin_0, RCC_AHB1Periph_GPIOA, EXTI_Line0, EXTI_PortSourceGPIOA, EXTI_PinSource0, EXTI0_IRQn);

// TODO: Redo
Button mrfPktRX(GPIOC, GPIO_Pin_2, RCC_AHB1Periph_GPIOD, EXTI_Line2, EXTI_PortSourceGPIOD, EXTI_PinSource2, EXTI2_IRQn);
MRF24J40 mrf(mrfRecvLed, mrfSendLed, RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOE, RCC_APB1Periph_SPI3, GPIOE, GPIOE, GPIOB, SPI3, GPIO_AF_SPI3,
		GPIO_PinSource4, GPIO_PinSource5, GPIO_PinSource3, GPIO_PinSource4, GPIO_PinSource5, GPIO_Pin_4, GPIO_Pin_5, GPIO_Pin_3, GPIO_Pin_4, GPIO_Pin_5);

UART uart2(RCC_AHB1Periph_GPIOA, RCC_APB1Periph_USART2, GPIOA, USART2, GPIO_PinSource2, GPIO_PinSource3, GPIO_Pin_2, GPIO_Pin_3, GPIO_AF_USART2, USART2_IRQn);

TODQueue todQueue(uart2, rxtxPulseLed, outOfSyncLed);
TOHQueue tohQueue(uart2, rxtxPulseLed);

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
			mrf.getTXCount(), mrf.getRXCount(), mrf.readPANId(), mrf.readSAddr(), mrf.readChannel(), mainCycles);

	msg.length = std::strlen(msg.text);

	tohQueue.moveToNextMsgWrite();
}

void handleMRFPktRX() {
	// TODO: Handle packet RX
}

int main(void)
{
	NVIC_SetPriorityGrouping(NVIC_PriorityGroup_2);	// 2 bits for pre-emption priority, 2 bits for non-preemptive subpriority
	uart2.setPriority(0,0);
	mrfPktRX.setPriority(1,0);
	infoButton.setPriority(1,1);
	msgHandler.setPriority(1,2);

	/* Set SysTick to fire each 10ms */
	RCC_ClocksTypeDef RCC_Clocks;
	RCC_GetClocksFreq(&RCC_Clocks);
	SysTick_Config(RCC_Clocks.HCLK_Frequency / 100);

	delayTimer.init();

	outOfSyncLed.init();
	rxtxLed.init();
	mrfSendLed.init();
	mrfRecvLed.init();
	rxtxPulseLed.init();

	uart2.init();
	mrf.init();
	tohQueue.init();
	msgHandler.init();
	todQueue.init();
	infoButton.init();
	mrfPktRX.init();

	infoButton.setPressedListener(handleInfoButtonInterrupt);
	mrfPktRX.setPressedListener(handleMRFPktRX);

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

