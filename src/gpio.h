/*
 * gpio.h
 *
 *  Created on: 25 окт. 2019 г.
 *      Author: serge78rus
 *      License: GPLv3
 */

#ifndef GPIO_H_
#define GPIO_H_

#include <avr/io.h>

// Arduino LED connected to PB5
#define gpio_LED_BIT 5

void gpio_init(void);
inline void gpio_led_on(void);
inline void gpio_led_off(void);

inline void gpio_led_on(void)
{
	PORTB |= (1 << gpio_LED_BIT);
}

inline void gpio_led_off(void)
{
	PORTB &= ~(1 << gpio_LED_BIT);
}

#endif /* GPIO_H_ */
