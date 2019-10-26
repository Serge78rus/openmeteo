/*
 * uart.c
 *
 *  Created on: 24 окт. 2019 г.
 *      Author: serge78rus
 */

/*
 * See http://microsin.net/programming/avr/stdio-standard-io-facilities.html
 * for linker options for printf & scanf
 */

#include <stdint.h>
#include <avr/io.h>

#include "uart.h"

#define BAUD 9600
#define DIVIDER (F_CPU / 16 / BAUD)

static int uart_putchar(char c, FILE *stream);
FILE uart = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);

void uart_init(void)
{
	//set baud rate
	UBRR0 = DIVIDER;
	//UBRR0H = (uint8_t)(DIVIDER >> 8);
	//UBRR0L = (uint8_t)(DIVIDER);

	//set mode
	UCSR0A =
			(0 << U2X0) | // U2Xn: Double the USART Transmission Speed
			(0 << MPCM0); // MPCMn: Multi-processor Communication Mode
	UCSR0B =
			(0 << RXCIE0) | // RXCIEn: RX Complete Interrupt Enable
			(0 << TXCIE0) | // TXCIEn: TX Complete Interrupt Enable
			(0 << UDRIE0) | // UDRIEn: USART Data Register Empty Interrupt Enable
			(0 << RXEN0) | // RXENn: Receiver Enable
			(1 << TXEN0) | // TXENn: Transmitter Enable
			(0 << UCSZ02); // UCSZn2: Character Size (set to 8)
	UCSR0C =
			(0 << UMSEL01) | (0 << UMSEL00) | // UMSELn1:0 USART Mode Select (set to Asynchronous USART)
			(0 << UPM01) | (0 << UPM00) | // UPMn1:0: Parity Mode (set to Disabled)
			(0 << USBS0) | // USBSn: Stop Bit Select (set to 0)
			(1 << UCSZ01) | (1 << UCSZ00) | // UCSZn1:0: Character Size (set to 8)
			(0 << UCPOL0); // UCPOLn: Clock Polarity (for synchronous mode only)
}

static int uart_putchar(char c, FILE *stream)
{
	while (!(UCSR0A & (1 << UDRE0))) { //Wait for empty transmit buffer
	}

	UDR0 = (uint8_t)c; //Put data into buffer, sends the data

	return 0;
}

