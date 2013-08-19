#include "main.h"
#include <stm32f4_discovery.h>
#include <stdio.h>
#include "uart_io.h"

static void Delay(__IO uint32_t nTime);

int main(void)
{
	/* Set SysTick to fire each 10ms */
	RCC_ClocksTypeDef RCC_Clocks;
	RCC_GetClocksFreq(&RCC_Clocks);
	SysTick_Config(RCC_Clocks.HCLK_Frequency / 100);


	STM_EVAL_LEDInit(LED4);
	uartIOInit();

	int idx = 0;

	while (1)
	{
		STM_EVAL_LEDOn(LED4);
		printf("Hello %03d.\n", idx++);

		Delay(50);

		STM_EVAL_LEDOff(LED4);

		Delay(50);
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
