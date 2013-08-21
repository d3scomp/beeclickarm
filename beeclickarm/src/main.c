#include "main.h"
#include "machine.h"
#include <stdio.h>

static void Delay(__IO uint32_t nTime);

int main(void)
{
	/* Set SysTick to fire each 10ms */
	RCC_ClocksTypeDef RCC_Clocks;
	RCC_GetClocksFreq(&RCC_Clocks);
	SysTick_Config(RCC_Clocks.HCLK_Frequency / 100);

	initSM();

	while (1)
	{
		tickSM();
//		printf("Hello %03d.\n", idx++);
	}
}



static __IO uint32_t uwTimingDelay;
void Delay(__IO uint32_t nTime)
{ 

	uwTimingDelay = nTime;

	while(uwTimingDelay != 0);
}

/* This function is called from SysTick handler */
void TimingDelay_Decrement(void)
{
	if (uwTimingDelay != 0x00)
	{
		uwTimingDelay--;
	}
}

/* This is called from syscalls.c as the implementation of _write syscall */
int syscallWriteHandler(char *ptr, int len) {
	int DataIdx;

	for (DataIdx = 0; DataIdx < len; DataIdx++)	{
//		USART_SendData(USART2, (uint8_t)*ptr);
		ITM_SendChar((uint8_t)*ptr); // Sends it to ST-Link SWO as well so that it can be observed in ST-Link Utility

//		while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET) {}

		ptr++;
	}

//	while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET) {}

	return len;
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
