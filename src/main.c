/*
 * main.c
 *
 *  Created on: 24 окт. 2019 г.
 *      Author: serge78rus
 */

#include <stdio.h>

#include <avr/io.h>
//#include <avr/interrupt.h>
#include <util/delay.h>
#include <util/atomic.h>

#include "def.h"
#include "lcd.h"
#include "gpio.h"
#include "uart.h"

#define FLASH_COUNT 3

static inline void show_logo(void);

int main(void) {

	cli();

	lcd_init();
	show_logo();

	gpio_init();
	uart_init();

	sei();

	for (;;) {
		for (int i = 0; i < FLASH_COUNT; ++i) {
			if (i) {
				_delay_ms(200);
			}
			gpio_led_on();
			_delay_ms(100);
			gpio_led_off();
		}
		_delay_ms(1000);
	}

	return 0;
}

static inline void show_logo(void)
{
	lcd_move_cursor(1, 0);
	fprintf(&lcd, "* OpenMETEO *");
	lcd_move_cursor(1, 1);
	fprintf(&lcd, "version %02X.%02X", VERSION >> 8, VERSION & 0xff);
}


