#ifndef MACHINE_H_
#define MACHINE_H_

#include <stm32f4xx.h>
#include <stm32f4_discovery.h>

#define LED_RXTX LED4
#define LED_OUT_OF_SYNC LED5
#define LED_RF_RECV LED3
#define LED_RF_SEND LED6

#define TOD_INTERRUPT_LINE EXTI_Line1
#define TOD_INTERRUPT_IRQn EXTI1_IRQn

void handleRFRecvInterrupt();
void handleInfoButtonInterrupt();
void handleTXRXInterrupt();
void handleTODInterrupt();

extern int mainCycles;

#endif /* MACHINE_H_ */
