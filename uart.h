#ifndef UART_H
#define UART_H

#include <stdint.h>

void UART_init(void);
void UART_putChar(uint8_t c);
uint8_t UART_getChar(void);
uint8_t UART_getString(uint8_t* buf);
void UART_putString(uint8_t* buf);
void UART_putBuffer(uint8_t* buf, uint32_t size);

#endif 
