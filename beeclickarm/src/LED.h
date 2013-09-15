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
	LED();
	virtual ~LED();

	virtual void on() = 0;
	virtual void off() = 0;
};

class PhysicalLED : public LED {
public:
	static PhysicalLED rxtx;
	static PhysicalLED outOfSync;
	static PhysicalLED rfRecv;
	static PhysicalLED rfSend;

	virtual void on();
	virtual void off();

private:
	PhysicalLED(GPIO_TypeDef* gpio, uint32_t pin, uint32_t clk);
	virtual ~PhysicalLED();

	GPIO_TypeDef* gpio;
	uint32_t pin;
	uint32_t clk;
};

class PulseLED {
public:
	PulseLED(LED& led, int minimalOnTimeTicks);
	~PulseLED();

	void pulse();

private:
	LED& led;
	int minimalOnTimeTicks;
	int onTicks;

	void tick();

	static std::list<PulseLED *> tickListeners;
	static void tickInterruptHandler();

	friend void SysTick_Handler();
};

#endif /* LED_H_ */
