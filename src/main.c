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
#include <avr/pgmspace.h>
#include <avr/sleep.h>
//#include <avr/interrupt.h>
#include <util/delay.h>
#include <util/atomic.h>

#include "debug.h"
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

#ifdef USE_SLEEP
	set_sleep_mode(SLEEP_MODE_IDLE);
#endif

	sei();

	TRACEF("%s", "start");

	_delay_ms(LOGO_DELAY_MS);
	lcd_clear();
	cycle();

	static uint8_t sec_count = 0;
	for (;;) {
		if (tim1_flag) {
			tim1_flag = false;

#ifdef USE_WDT
			wdt_reset();
#endif

			if (++sec_count == CYCLE_S) {
				sec_count = 0;
				cycle();
			}

		}

#ifdef USE_SLEEP
		sleep_mode();
#endif

	}

	return 0;
}

static inline void show_logo(void)
{
	static const char TITLE_STR[] PROGMEM = "* OpenMETEO *";
	static const char VERSION_STR[] PROGMEM = "version %02X.%02X";

	lcd_move_cursor(1, 0);
	fprintf_P(&lcd, TITLE_STR);
	lcd_move_cursor(1, 1);
	fprintf_P(&lcd, VERSION_STR, VERSION >> 8, VERSION & 0xff);
}

static void cycle(void)
{
	TRACEF("%s", "cycle");

	//todo stub
	static int cnt = 0;
	lcd_move_cursor(0, 0);
	fprintf(&lcd, "cycle: %i", cnt);
	TRACEF("Cycle %i", cnt);
	++cnt;
}

