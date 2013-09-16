/*
 * LED.h
 *
 *  Created on: 15. 9. 2013
 *      Author: Tomas Bures <bures@d3s.mff.cuni.cz>
 */

#ifndef LED_H_
#define LED_H_

#include "stm32f4xx.h"
#include <list>

class LED {
public:
	static LED rxtx;
	static LED outOfSync;
	static LED rfRecv;
	static LED rfSend;

	void on();
	void off();
	void init();

private:
	LED(GPIO_TypeDef* gpio, uint32_t pin, uint32_t clk);
	~LED();

	GPIO_TypeDef* gpio;
	uint32_t pin;
	uint32_t clk;
};

class PulseLED {
public:
	PulseLED(LED& led, int minimalOnTimeTicks);
	~PulseLED();

	void pulse();
	void init();

private:
	LED& led;
	int minimalOnTimeTicks;
	int onTicks;

	void tick();

	static std::list<PulseLED*> tickListeners;
	static void tickInterruptHandler();

	friend void SysTick_Handler();
};

#endif /* LED_H_ */
