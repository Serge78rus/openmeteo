/*
 * main.c
 *
 *  Created on: 24 окт. 2019 г.
 *      Author: serge78rus
 */

#include <util/delay.h>

#include "gpio.h"

#define FLASH_COUNT 3

int main(void)
{
	gpio_init();


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

