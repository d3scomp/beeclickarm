/*
 * LED.h
 *
 *  Created on: 15. 9. 2013
 *      Author: Tomas Bures <bures@d3s.mff.cuni.cz>
 */

#ifndef LED_H_
#define LED_H_

#include "stm32f4xx.h"

class LED {
public:
	static LED rxtx;
	static LED outOfSync;
	static LED rfRecv;
	static LED rfSend;

	void on();
	void off();

private:
	LED(GPIO_TypeDef* gpio, uint32_t pin, uint32_t clk);
	virtual ~LED();

	GPIO_TypeDef* gpio;
	uint32_t pin;
	uint32_t clk;
};

#endif /* LED_H_ */
