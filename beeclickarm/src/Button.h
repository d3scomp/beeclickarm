/*
 * Button.h
 *
 *  Created on: 15. 9. 2013
 *      Author: Tomas Bures <bures@d3s.mff.cuni.cz>
 */

#ifndef BUTTON_H_
#define BUTTON_H_

#include "stm32f4xx.h"
#include <functional>

class Button {
public:
	Button(GPIO_TypeDef* gpio, uint32_t pin, uint32_t clk, uint32_t extiLine, uint8_t extiPortSource, uint8_t extiPinSource, IRQn irqn);
	~Button();

	typedef std::function<void()> Listener;

	bool isPressed();
	void setPressedListener(Listener pressedListener);

	void setPriority(uint8_t irqPreemptionPriority, uint8_t irqSubPriority);
	void init();

	void pressedInterruptHandler();

private:
	GPIO_TypeDef* gpio;
	uint32_t pin;
	uint32_t clk;

	uint8_t irqPreemptionPriority;
	uint8_t irqSubPriority;

	uint32_t extiLine;
	uint8_t extiPortSource;
	uint8_t extiPinSource;
	IRQn irqn;

	Listener pressedListener;
};

#endif /* BUTTON_H_ */
