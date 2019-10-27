/*
 * main.c
 *
 *  Created on: 24 окт. 2019 г.
 *  Author: serge78rus
 *  License GPL-3.0
 */

#include <stdio.h>
#include <stdint.h>

#include <avr/io.h>
#include <avr/wdt.h>
//#include <avr/interrupt.h>
#include <util/delay.h>
#include <util/atomic.h>

#include "def.h"
#include "lcd.h"
#include "gpio.h"
#include "uart.h"
#include "tim1.h"


static inline void show_logo(void);
static void cycle(void);

int main(void) {
#ifdef USE_WDT
	wdt_enable(WDTO_8S);
#endif

	cli();

	lcd_init();
	show_logo();

	gpio_init();
	uart_init();
	tim1_init();

	sei();

	_delay_ms(LOGO_DELAY_MS);
	lcd_clear();
	cycle();

	static uint8_t sec_count = 0;
	for (;;) {
		if (tim1_flag) {
			tim1_flag = 0;
#ifdef USE_WDT
			wdt_reset();
#endif
			if (++sec_count == CYCLE_S) {
				sec_count = 0;
				cycle();
			}
			//slep mode
		}
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

static void cycle(void)
{
	//todo stub
	static int cnt = 0;
	lcd_move_cursor(0, 0);
	fprintf(&lcd, "cycle: %i", cnt);
	fprintf(&uart, "cycle: %i\r\n", cnt);
	++cnt;
}

