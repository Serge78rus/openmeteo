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
#include "am2301.h"
#include "twi.h"
#include "bmp180.h"

#define DISPLAY_LINE_LEN 16

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
	am2301_init();
	twi_init();
	bmp180_init();

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
	static const char TEMP_RH_FMT_STR[] PROGMEM = "%c%i.%01i""\xdf""C   %u.%01u%%";
	static const char PRESS_FMT_STR[] PROGMEM = "%i.%02imm";
	static const char ERROR_STR[] PROGMEM = "Error";

	TRACEF("%s", "cycle");

	lcd_move_cursor(0, 0);
	if (am2301_update()) {

		uint16_t hum = am2301_get_humidity();
		int16_t temp = am2301_get_temp();
		bool neg_temp = false;
		if (temp < 0) {
			temp = -temp;
			neg_temp = true;
		}
		int len = fprintf_P(&lcd, TEMP_RH_FMT_STR,
				neg_temp ? '-' : '+',
				temp / 10, temp % 10,
				hum / 10, hum % 10);
		if (len > 0) {
			lcd_fill_space(DISPLAY_LINE_LEN - len);
		}
		//todo fprintf(&uart, ...
	} else {
		int len = fprintf_P(&lcd, ERROR_STR);
		lcd_fill_space(DISPLAY_LINE_LEN - len);
		//todo fprintf(&uart, ...
	}

	lcd_move_cursor(0, 1);
	if (bmp180_update(bmp180_MODE_ULTRA_HIGH_RESOLUTION)) {

		int32_t press = bmp180_get_press_mm();
		int len = fprintf_P(&lcd, PRESS_FMT_STR,
				(int)(press / 100), (int)(press % 100));
		if (len > 0) {
			lcd_fill_space(DISPLAY_LINE_LEN - len);
		}
		//todo fprintf(&uart, ...
	} else {
		int len = fprintf_P(&lcd, ERROR_STR);
		lcd_fill_space(DISPLAY_LINE_LEN - len);
		//todo fprintf(&uart, ...
	}
}

