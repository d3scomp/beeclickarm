#ifndef UART_IO_H_
#define UART_IO_H_

#include "stm32f4xx.h"

void uart2Init();
uint8_t uart2RecvChar();
void uart2SendChar(uint8_t ch);
int uart2CanSend();
int uart2CanRecv();
int uart2SendComplete();
int uart2IsBreakOrError();
void uart2ClearBreakOrError();

void uart2TXIntEnable();
void uart2TXIntDisable();

#endif /* UART_IO_H_ */
