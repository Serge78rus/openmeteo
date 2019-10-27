/*
 * tim1.h
 *
 *  Created on: 25 окт. 2019 г.
 *      Author: serge78rus
 */

#ifndef TIM1_H_
#define TIM1_H_

#include <stdint.h>

/*
 * Use timer 1 for generate 1s interrupt
 */

void tim1_init(void);

extern volatile uint8_t tim1_flag;

#endif /* TIM1_H_ */
