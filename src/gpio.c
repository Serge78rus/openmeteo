/*
 * gpio.c
 *
 *  Created on: 25 окт. 2019 г.
 *      Author: serge78rus
 *      License: GPLv3
 */

#include <avr/io.h>

#include "gpio.h"

void gpio_init(void)
{
	DDRB |= //GPIO mode output
			(1 << gpio_LED_BIT); //LED GPIO
}

