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
	Timer(TIM_TypeDef* tim, uint32_t clk);
	virtual ~Timer();

	void init();

	inline void uDelay(uint16_t us) {
		tim->CNT = 0;
		while (tim->CNT < us);
	}

	inline void mDelay(uint16_t ms) {
		while (ms-- > 0) {
			uDelay(1000);
		}
	}

private:
	TIM_TypeDef* tim;
	uint32_t clk;
};

#endif /* TIMER_H_ */
