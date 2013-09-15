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
	LED(uint32_t clk, GPIO_TypeDef* gpio, uint32_t pin);
	virtual ~LED();

	uint32_t clk;
	GPIO_TypeDef* gpio;
	uint32_t pin;
};

#endif /* LED_H_ */
