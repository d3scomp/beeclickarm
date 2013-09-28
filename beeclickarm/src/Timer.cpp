/*
 * Timer.cpp
 *
 *  Created on: 18. 9. 2013
 *      Author: Tomas Bures <bures@d3s.mff.cuni.cz>
 */

#include "Timer.h"

Timer::Timer(Properties& initProps) : props(initProps) {
}

Timer::~Timer() {
}

void Timer::setPriority(uint8_t irqPreemptionPriority, uint8_t irqSubPriority) {
	this->irqPreemptionPriority = irqPreemptionPriority;
	this->irqSubPriority = irqSubPriority;
}

void Timer::init() {
	props.clkCmdFun(props.clk, ENABLE);

	TIM_TimeBaseInitTypeDef timInitStruct;
	timInitStruct.TIM_Prescaler = 82;
	timInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	timInitStruct.TIM_Period = 0xFFFF;
	timInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	timInitStruct.TIM_RepetitionCounter = 0x00;
	TIM_TimeBaseInit(props.tim, &timInitStruct);

	NVIC_InitTypeDef NVIC_InitStructure;

	NVIC_InitStructure.NVIC_IRQChannel = props.irqn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = irqPreemptionPriority;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = irqSubPriority;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

	NVIC_Init(&NVIC_InitStructure);

	TIM_Cmd(props.tim, ENABLE);
}
