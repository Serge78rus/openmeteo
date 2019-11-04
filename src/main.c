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
#include "pwm.h"
#include "diff.h"

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
	pwm_init();
	diff_init();

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

	if (am2301_update()) {

		uint16_t hum = am2301_get_humidity();
		int16_t temp = am2301_get_temp();

		char temp_sign;
		if (temp < 0) {
			temp = -temp;
			temp_sign = '-';
		} else {
			temp_sign = '+';
		}

		int16_t temp_int = temp / 10;
		int16_t temp_fract = temp % 10;
		uint16_t hum_int = hum / 10;
		uint16_t hum_fract = hum % 10;

		lcd_show_temp(temp_sign, temp_int, temp_fract);
		lcd_show_hum(hum_int, hum_fract);
		static const char FMT_STR[] PROGMEM = "t=%c%i.%01i h=%u.%01u ";
		fprintf_P(&uart, FMT_STR,
				temp_sign, temp_int, temp_fract,
				hum_int, hum_fract);

	} else {
		lcd_show_bad_temp();
		lcd_show_bad_hum();
		static const char FMT_STR[] PROGMEM = "t=error h=error ";
		fprintf_P(&uart, FMT_STR);
	}

	if (bmp180_update(bmp180_MODE_ULTRA_HIGH_RESOLUTION)) {

		int32_t press = bmp180_get_press_mm();
		diff_put(press);

		press = (press + 5) / 10;
		int16_t press_int = press / 10;
		int16_t press_fract = press % 10;

		lcd_show_press(press_int, press_fract);
		static const char FMT_STR[] PROGMEM = "p=%i.%01i\r\n";
		fprintf_P(&uart, FMT_STR,
				press_int, press_fract);

	} else {
		diff_put(BAD_INT32);
		lcd_show_bad_press();
		static const char FMT_STR[] PROGMEM = "p=error\r\n";
		fprintf_P(&uart, FMT_STR);
	}

	int32_t diff = diff_calc();
	if (diff != BAD_INT32) {

		char diff_sign;
		if (diff < 0) {
			diff = -diff;
			diff_sign = '-';
		} else {
			diff_sign = '+';
		}

		int16_t diff_int = diff / 100;
		int16_t diff_fract = diff % 100;

		lcd_show_diff(diff_sign, diff_int, diff_fract);

	} else {
		lcd_show_bad_diff();
	}
}

