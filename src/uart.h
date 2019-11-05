/*
 * uart.h
 *
 *  Created on: 24 окт. 2019 г.
 *  Author: serge78rus
 *  License GPL-3.0
 */

#ifndef UART_H_
#define UART_H_

#include <stdio.h>

#include <avr/io.h>

void uart_init(void);
inline void uart_wait_empty(void);

extern FILE uart;

inline void uart_wait_empty(void)
{
	while (!(UCSR0A & (1 << UDRE0))) { //Wait for empty transmit buffer
	}
}

#endif /* UART_H_ */
