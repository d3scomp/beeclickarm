/*
 * Button.cpp
 *
 *  Created on: 15. 9. 2013
 *      Author: Tomas Bures <bures@d3s.mff.cuni.cz>
 */

#include "Button.h"


Button Button::info(GPIOA, GPIO_Pin_0, RCC_AHB1Periph_GPIOA, EXTI_Line0, EXTI_PortSourceGPIOA, EXTI_PinSource0, EXTI0_IRQn);

Button::Button(GPIO_TypeDef* gpio, uint32_t pin, uint32_t clk, uint32_t extiLine, uint8_t extiPortSource, uint8_t extiPinSource, IRQn irqn) :
		gpio(gpio), pin(pin), clk(clk), irqPreemptionPriority(0), irqSubPriority(0), extiLine(extiLine), extiPortSource(extiPortSource), extiPinSource(extiPinSource), irqn(irqn) {
}

Button::~Button() {
}

void Button::setPriority(uint8_t irqPreemptionPriority, uint8_t irqSubPriority) {
	this->irqPreemptionPriority = irqPreemptionPriority;
	this->irqSubPriority = irqSubPriority;
}

void Button::init() {
	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	/* Enable the BUTTON Clock */
	RCC_AHB1PeriphClockCmd(clk, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	/* Configure Button pin as input */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Pin = pin;
	GPIO_Init(gpio, &GPIO_InitStructure);

	SYSCFG_EXTILineConfig(extiPortSource, extiPinSource);

	/* Configure Button EXTI line */
	EXTI_InitStructure.EXTI_Line = extiLine;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	/* Enable and set Button EXTI Interrupt to the lowest priority */
	NVIC_InitStructure.NVIC_IRQChannel = irqn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

	NVIC_Init(&NVIC_InitStructure);
}


bool Button::isPressed() {
	return GPIO_ReadInputDataBit(gpio, pin);
}

void Button::setPressedListener(Listener pressedListener) {
	this->pressedListener = pressedListener;
}

void Button::pressedInterruptHandler() {
	if (pressedListener) {
		pressedListener();
	}

	EXTI_ClearITPendingBit(extiLine);
}

