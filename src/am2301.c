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

#define USE_TIMER_0
//#define USE_TIMER_2

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

#if defined USE_TIMER_0
	//configure TIMER0
	TCCR0A = //Timer/Counter Control Register A
			(0 << COM0A1) | (0 << COM0A0) | // COM0A1:0: Compare Match Output A Mode
			(0 << COM0B1) | (0 << COM0B0) | // COM0B1:0: Compare Match Output B Mode
			(0 << WGM01) | (0 << WGM00); // WGM01:0: Waveform Generation Mode (normal mode)
	TCCR0B = //Timer/Counter Control Register B
			(0 << FOC0A) | // FOC0A: Force Output Compare A
			(0 << FOC0B) | // FOC0B: Force Output Compare B
			(0 << WGM02) | // WGM02: Waveform Generation Mode (normal mode)
			(0 << CS02) | (1 << CS01) | (0 << CS00); // CS02:0: Clock Select (clkT2S/8 (From prescaler))
#elif defined USE_TIMER_2
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
#else
#error "No timer selected"
#endif
}

bool am2301_update(void)
{
	static uint8_t data[5];

	//low start pulse
	DDRD |= (1 << am2301_BIT); //GPIO mode output (compile to atomic command)
	_delay_us(START_PULSE_US);
	DDRD &= ~(1 << am2301_BIT); //GPIO mode input (compile to atomic command)

#if defined USE_TIMER_0
	TCNT0 = 0;
#elif defined USE_TIMER_2
	TCNT2 = 0;
#endif
	duration = 0;
	timeout_flag = false;

	EIFR = (1 << INTF0); // INTF0: External Interrupt Flag 0 (write 1 - clear flag)
	EIMSK = (1 << INT0); // INT0: External Interrupt Request 0 Enable (enable)

#if defined USE_TIMER_0
	TIFR0 = (1 << TOV0); // TOV0: Timer/Counter0 Overflow Flag (write 1 - clear flag)
	TIMSK0 = (1 << TOIE0); // Timer/Counter0 Overflow Interrupt Enable
#elif defined USE_TIMER_2
	TIFR2 = (1 << TOV2); // TOV2: Timer/Counter2 Overflow Flag (write 1 - clear flag)
	TIMSK2 = (1 << TOIE2); // Timer/Counter2 Overflow Interrupt Enable
#endif

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

#if defined USE_TIMER_0
	TIMSK0 = (0 << TOIE0); // Timer/Counter0 Overflow Interrupt Enable (disable)
#elif defined USE_TIMER_2
	TIMSK2 = (0 << TOIE2); // Timer/Counter2 Overflow Interrupt Enable (disable)
#endif
	EIMSK = (0 << INT0); // INT0: External Interrupt Request 0 Enable (disable)

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
#if defined USE_TIMER_0
	if (!(PIND & (1 << am2301_BIT))) { //falling edge
		duration = TCNT0;
	}
	TCNT0 = 0;
#elif defined USE_TIMER_2
	if (!(PIND & (1 << am2301_BIT))) { //falling edge
		duration = TCNT2;
	}
	TCNT2 = 0;
#endif
}

#if defined USE_TIMER_0
ISR(TIMER0_OVF_vect)
{
	timeout_flag = true;
}
#elif defined USE_TIMER_2
ISR(TIMER2_OVF_vect)
{
	timeout_flag = true;
}
#endif

