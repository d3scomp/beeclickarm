/*
 * Timer.h
 *
 *  Created on: 18. 9. 2013
 *      Author: Tomas Bures <bures@d3s.mff.cuni.cz>
 */

#ifndef TIMER_H_
#define TIMER_H_

#include "stm32f4xx.h"

class Timer {
public:
	struct Properties {
		TIM_TypeDef* tim;
		void (*clkCmdFun)(uint32_t periph, FunctionalState newState);
		uint32_t clk;
		IRQn irqn;
	};

	Timer(Properties& initProps);
	virtual ~Timer();

	void setPriority(uint8_t irqPreemptionPriority, uint8_t irqSubPriority);
	void init();

	inline void uDelay(uint16_t us) {
		props.tim->CNT = 0;
		while (props.tim->CNT < us);
	}

	inline void mDelay(uint16_t ms) {
		while (ms-- > 0) {
			uDelay(1000);
		}
	}

private:
	Properties props;

	uint8_t irqPreemptionPriority;
	uint8_t irqSubPriority;
};

#endif /* TIMER_H_ */
