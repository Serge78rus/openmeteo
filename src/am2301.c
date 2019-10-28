/*
 * am2301.c
 *
 *  Created on: 27 окт. 2019 г.
 *  Author: serge78rus
 *  License GPL-3.0
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>

#include "am2301.h"
#include "debug.h"

#define START_PULSE_US 1000
//TIME_THRESHOLD = 50us (short pulse = 27us, long pulse = 70us)
#if F_CPU == 16000000
#define TIME_THRESHOLD 100 /*in timer cycles (F_TIM=2MHz)*/
#elif F_CPU == 8000000
#define TIME_THRESHOLD 50 /*in timer cycles (F_TIM=2MHz)*/
#else
#error "Unsupported F_CPU value"
#endif


uint16_t _am2301_humidity;
int16_t _am2301_temp;

static volatile bool timeout_flag;
static volatile uint8_t duration;

void am2301_init(void)
{
	//configure GPIO
	PORTD &= ~(1 << am2301_BIT); //GPIO output LOW (no pullup input)
	DDRD &= ~(1 << am2301_BIT); //GPIO mode input
	EICRA = //External Interrupt Control Register A
			(0 << ISC11) | (0 << ISC10) | // ISC11, ISC10: Interrupt Sense Control 1 Bit 1 and Bit 0
			(0 << ISC01) | (1 << ISC00); // ISC01, ISC00: Interrupt Sense Control 0 Bit 1 and Bit 0
										//(Any logical change on INT0 generates an interrupt request)

	//configure TIMER2
	TCCR2A = //Timer/Counter Control Register A
			(0 << COM2A1) | (0 << COM2A0) | // COM2A1:0: Compare Match Output A Mode
			(0 << COM2B1) | (0 << COM2B0) | // COM2B1:0: Compare Match Output B Mode
			(0 << WGM21) | (0 << WGM20); // WGM21:0: Waveform Generation Mode (normal mode)
	TCCR2B = //Timer/Counter Control Register B
			(0 << FOC2A) | // FOC2A: Force Output Compare A
			(0 << FOC2B) | // FOC2B: Force Output Compare B
			(0 << WGM22) | // WGM22: Waveform Generation Mode (normal mode)
			(0 << CS22) | (1 << CS21) | (0 << CS20); // CS22:0: Clock Select (clkT2S/8 (From prescaler))
}

bool am2301_update(void)
{
	static uint8_t data[5];

	//todo start timer clock

	//low start pulse
	DDRD |= (1 << am2301_BIT); //GPIO mode output (compile to atomic command)
	_delay_us(START_PULSE_US);
	DDRD &= ~(1 << am2301_BIT); //GPIO mode input (compile to atomic command)

	timeout_flag = false;
	TCNT2 = 0;
	duration = 0;

	EIFR = (1 << INTF0); // INTF0: External Interrupt Flag 0 (write 1 - clear flag)
	EIMSK = (1 << INT0); // INT0: External Interrupt Request 0 Enable (enable)

	TIFR2 = (1 << TOV2); // TOV2: Timer/Counter2 Overflow Flag (write 1 - clear flag)
	TIMSK2 = (1 << TOIE2); // Timer/Counter2 Overflow Interrupt Enable

	//skip 2 pulse
	while (!(duration || timeout_flag)) {
	}
	duration = 0;
	while (!(duration || timeout_flag)) {
	}
	duration = 0;

	for (int i = 0; i < sizeof(data) / sizeof(data[0]) && !timeout_flag; ++i) {
		data[i] = 0;
		uint8_t mask = 0x80;
		do {
			while (!(duration || timeout_flag)) {
			}

			if (timeout_flag) {
				break;
			}

			if (duration > TIME_THRESHOLD) {
				data[i] |= mask;
			}

			duration = 0;
		} while (mask >>= 1);
	}

	TIMSK2 = (0 << TOIE2); // Timer/Counter2 Overflow Interrupt Enable (disable)
	EIMSK = (0 << INT0); // INT0: External Interrupt Request 0 Enable (disable)

	//todo stop timer clock

	/* for debug
	for (int i = 0; i < sizeof(data) / sizeof(data[0]); ++i) {
		TRACEF("data[%i]: %u", i, (unsigned)data[i]);
	}
	*/

	if (timeout_flag) {
		ERROR();
		return false;
	}

	//check checksum
	if (((data[0] + data[1] + data[2] + data[3]) & 0xff) != data[4]) {
		ERROR();
		return false;
	}

	_am2301_humidity = ((uint16_t)data[0] << 8) | data[1];
	_am2301_temp = ((int16_t)(data[2] & 0x7f) << 8) | data[3];
	if (data[2] & 0x80) {
		_am2301_temp = -_am2301_temp;
	}

	return true;
}

/*
 * ISR
 */

ISR(INT0_vect)
{
	if (!(PIND & (1 << am2301_BIT))) { //falling edge
		duration = TCNT2;
	}
	TCNT2 = 0;
}

ISR(TIMER2_OVF_vect)
{
	timeout_flag = true;
}

