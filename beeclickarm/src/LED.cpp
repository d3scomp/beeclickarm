/*
 * LED.cpp
 *
 *  Created on: 15. 9. 2013
 *      Author: Tomas Bures <bures@d3s.mff.cuni.cz>
 */

#include "LED.h"

LED::LED(GPIO_TypeDef* gpio, uint32_t pin, uint32_t clk) :	gpio(gpio), pin(pin), clk(clk) {
}

LED::~LED() {
}

void LED::init() {
	GPIO_InitTypeDef  gpioInitStruct;

	/* Enable the GPIO_LED Clock */
	RCC_AHB1PeriphClockCmd(clk, ENABLE);

	/* Configure the GPIO_LED pin */
	gpioInitStruct.GPIO_Pin = pin;
	gpioInitStruct.GPIO_Mode = GPIO_Mode_OUT;
	gpioInitStruct.GPIO_OType = GPIO_OType_PP;
	gpioInitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	gpioInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(gpio, &gpioInitStruct);
}

void LED::on() {
	gpio->BSRRL = pin;
}

void LED::off() {
	gpio->BSRRH = pin;
}


std::list<PulseLED*> PulseLED::tickListeners;

void PulseLED::tickInterruptHandler() {
	for (PulseLED* led : tickListeners) {
		led->tick();
	}
}


PulseLED::PulseLED(LED& led, int minimalOnTimeTicks) : led(led), minimalOnTimeTicks(minimalOnTimeTicks), onTicks(-1) {
}

PulseLED::~PulseLED() {
}

void PulseLED::init() {
	tickListeners.push_back(this);
}

void PulseLED::pulse() {
	onTicks = 0;
	led.on();
}

void PulseLED::tick() {
	if (onTicks != -1) {
		onTicks++;

		if (onTicks > minimalOnTimeTicks) {
			onTicks = -1;
			led.off();
		}
	}
}

