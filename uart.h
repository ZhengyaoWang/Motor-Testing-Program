#ifndef UART_H
#define UART_H

#include "stm32f10x.h"
#include "stm32f10x_usart.h"

void UART_Init(uint32_t baudrate);
void UART_SendByte(uint8_t byte);
void UART_SendString(char* string);
uint8_t UART_ReceiveByte(void);
void USART1_IRQHandler(void);

#endif // UART_H
