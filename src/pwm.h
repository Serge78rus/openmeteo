/*
 * pwm.h
 *
 *  Created on: 3 нояб. 2019 г.
 *      Author: serge78rus
 */

#ifndef PWM_H_
#define PWM_H_

/*
 * use TIMER2 channel A
 * PWM pin PB3 (MOSI/OC2A/PCINT3) (Arduino pin 11)
 * button pin PD3 (PCINT19/OC2B/INT1) (Arduino pin 3)
 */

void pwm_init(void);

#endif /* PWM_H_ */
