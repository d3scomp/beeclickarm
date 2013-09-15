/*
 * LED.cpp
 *
 *  Created on: 15. 9. 2013
 *      Author: Tomas Bures <bures@d3s.mff.cuni.cz>
 */

#include "LED.h"

LED LED::rxtx(RCC_AHB1Periph_GPIOD, GPIOD, GPIO_Pin_12);
LED LED::outOfSync(RCC_AHB1Periph_GPIOD, GPIOD, GPIO_Pin_14);
LED LED::rfRecv(RCC_AHB1Periph_GPIOD, GPIOD, GPIO_Pin_13);
LED LED::rfSend(RCC_AHB1Periph_GPIOD, GPIOD, GPIO_Pin_15);

void LED::on() {
	gpio->BSRRL = pin;
}

void LED::off() {
	gpio->BSRRH = pin;
}

LED::LED(uint32_t clk, GPIO_TypeDef* gpio, uint32_t pin):
		clk(clk), gpio(gpio), pin(pin) {

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

LED::~LED() {
}

