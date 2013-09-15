/*
 * LED.cpp
 *
 *  Created on: 15. 9. 2013
 *      Author: Tomas Bures <bures@d3s.mff.cuni.cz>
 */

#include "LED.h"

LED::LED() {
}

LED::~LED() {
}


PhysicalLED PhysicalLED::rxtx(GPIOD, GPIO_Pin_12, RCC_AHB1Periph_GPIOD);
PhysicalLED PhysicalLED::outOfSync(GPIOD, GPIO_Pin_14, RCC_AHB1Periph_GPIOD);
PhysicalLED PhysicalLED::rfRecv(GPIOD, GPIO_Pin_13, RCC_AHB1Periph_GPIOD);
PhysicalLED PhysicalLED::rfSend(GPIOD, GPIO_Pin_15, RCC_AHB1Periph_GPIOD);

PhysicalLED::PhysicalLED(GPIO_TypeDef* gpio, uint32_t pin, uint32_t clk) :	gpio(gpio), pin(pin), clk(clk) {

	GPIO_InitTypeDef  GPIO_InitStructure;

	/* Enable the GPIO_LED Clock */
	RCC_AHB1PeriphClockCmd(clk, ENABLE);

	/* Configure the GPIO_LED pin */
	GPIO_InitStructure.GPIO_Pin = pin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(gpio, &GPIO_InitStructure);
}

PhysicalLED::~PhysicalLED() {
}

void PhysicalLED::on() {
	gpio->BSRRL = pin;
}

void PhysicalLED::off() {
	gpio->BSRRH = pin;
}


std::list<PulseLED*> PulseLED::tickListeners;

void PulseLED::tickInterruptHandler() {
	for (PulseLED* listener : tickListeners) {
		listener->tick();
	}
}


PulseLED::PulseLED(LED& led, int minimalOnTimeTicks) : led(led), minimalOnTimeTicks(minimalOnTimeTicks), onTicks(-1) {
	tickListeners.push_back(this);
}

PulseLED::~PulseLED() {
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

