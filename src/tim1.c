/*
 * tim1.c
 *
 *  Created on: 25 окт. 2019 г.
 *  Author: serge78rus
 *  License GPL-3.0
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sfr_defs.h>

#include "tim1.h"

#if F_CPU == 16000000
#define OCR_VAL (62500 - 1)
#elif F_CPU == 8000000
#define OCR_VAL (31250 - 1)
#else
#error "Unsupported F_CPU value"
#endif

volatile uint8_t tim1_flag = 0;

void tim1_init(void)
{


	TCNT1 = 0;
	OCR1A = OCR_VAL;

	TCCR1A =
			(0 << COM1A0) |	(0 << COM1A1) | // COM1A1:0: Compare Output Mode for Channel A
			(0 << COM1B0) |	(0 << COM1B1) | // COM1B1:0: Compare Output Mode for Channel B
			(0 << WGM10) | (0 << WGM11); // WGM11:0: Waveform Generation Mode (mode 4: CTC use OCR1A)
	TCCR1B =
			(0 << ICNC1) | // ICNC1: Input Capture Noise Canceler
			(0 << ICES1) | // ICES1: Input Capture Edge Select
			(0 << WGM13) | (1 << WGM12) | // WGM13:2: Waveform Generation Mode (mode 4: CTC use OCR1A)
			(1 << CS12) | (0 << CS11) | (0 << CS10); // CS12:0: Clock Select (clkI/O / 256 From prescaler)
	TCCR1C =
			(0 << FOC1A) | // FOC1A: Force Output Compare for Channel A
			(0 << FOC1B); // FOC1B: Force Output Compare for Channel B

	TIMSK1 =
			(0 << ICIE1) | // ICIE1: Timer/Counter1, Input Capture Interrupt Enable
			(0 << OCIE1B) | // OCIE1B: Timer/Counter1, Output Compare B Match Interrupt Enable
			(1 << OCIE1A) | // OCIE1A: Timer/Counter1, Output Compare A Match Interrupt Enable
			(0 << TOIE1); // TOIE1: Timer/Counter1, Overflow Interrupt Enable

}

/*
 * ISR
 */

ISR(TIMER1_COMPA_vect)
{
	tim1_flag = 1;
}


