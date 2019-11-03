/*
 * pwm.c
 *
 *  Created on: 3 нояб. 2019 г.
 *      Author: serge78rus
 */

#include <stdint.h>
#include <stdbool.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sfr_defs.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>

#include "pwm.h"
#include "def.h"
#include "debug.h"

#define PWM_BIT 3
#define BUTTON_BIT 3
#define DEFAULT_MODE 0
#define DEBOUNCE 50 /*in timer cycle (~1ms)*/
#define MAX_TIME_COUNT 5000 /*if button activate and long time not changed*/
#define EEPROM_ADDR 0

//private functions
static inline uint8_t restore_mode(void);
static inline void save_mode(uint8_t mode);
static inline void set_mode(uint8_t mode);
static inline void on_button_pressed(void);

//private variables
static uint8_t mode;
static const uint8_t MODE_VALUES[] PROGMEM = {255, 127, 63, 31, 15, 7, 3, 1};
static uint16_t time_count = 0;

void pwm_init(void)
{
	TCCR2A = // Timer/Counter Control Register A
			(1 << COM2A1) | (0 << COM2A0) | // COM2A1:0: Compare Match Output A Mode
			(0 << COM2B1) | (0 << COM2B0) | // COM2B1:0: Compare Match Output B Mode
			(1 << WGM21) | (1 << WGM20); //WGM21:0: Waveform Generation Mode (mode 3 - Fast PWM)
	TCCR2B = // Timer/Counter Control Register B
			(0 << FOC2A) | // FOC2A: Force Output Compare A
			(0 << FOC2B) | // FOC2B: Force Output Compare B
			(0 << WGM22) | // WGM22: Waveform Generation Mode (mode 3 - Fast PWM)
#if F_CPU == 16000000
			(1 << CS22) | (0 << CS21) | (0 << CS20); // CS22:0: Clock Select (F_CPU/64=976.5625kHz (T~=1ms))
#elif F_CPU == 8000000
			(0 << CS22) | (1 << CS21) | (1 << CS20); // CS22:0: Clock Select (F_CPU/32=976.5625kHz (T~=1ms))
#else
#error "Unsupported F_CPU value"
#endif

	DDRB |= (1 << PWM_BIT); //setup output pin

	PORTD |= (1 << BUTTON_BIT); //set pull up of button pin
	DDRD &= ~(1 << BUTTON_BIT); //GPIO mode input
	EICRA |= //External Interrupt Control Register A
			(0 << ISC11) | (1 << ISC10) | // ISC11, ISC10: Interrupt Sense Control 1 Bit 1 and Bit 0
			//(Any logical change on INT1 generates an interrupt request)
			(0 << ISC01) | (0 << ISC00); // ISC01, ISC00: Interrupt Sense Control 0 Bit 1 and Bit 0
	EIFR = (1 << INTF1); // INTF1: External Interrupt Flag 1 (write 1 - clear flag)
	EIMSK |= (1 << INT1); // INT1: External Interrupt Request 1 Enable

	mode = restore_mode();
	set_mode(mode);
}

/*
 * private functions
 */

static inline uint8_t restore_mode(void)
{
	eeprom_busy_wait();
	uint16_t data = eeprom_read_word(EEPROM_ADDR);
	uint8_t mode = (uint8_t)data;
	if ((uint8_t)(~data >> 8) == mode) {
		return mode;
	} else {
		return DEFAULT_MODE;
	}
}

static inline void save_mode(uint8_t mode)
{
	uint16_t data = mode | ((uint16_t)~mode << 8);
	eeprom_busy_wait();
	eeprom_write_word(EEPROM_ADDR, data);
}

static inline void set_mode(uint8_t mode)
{
	OCR2A = pgm_read_byte(MODE_VALUES + mode);
}

static inline void on_button_pressed(void)
{
	if (++mode == sizeof(MODE_VALUES) / sizeof(MODE_VALUES[0])) {
		mode = 0;
	}
	set_mode(mode);
	save_mode(mode);
}

/*
 * ISR
 */

ISR(TIMER2_OVF_vect)
{
	if (++time_count == MAX_TIME_COUNT) { //time count and return to passive state if button press timeout
		TIMSK2 &= // Timer/Counter2Interrupt Mask Register
				~(1 << TOIE2); //TOIE2: Timer/Counter2 Overflow Interrupt Enable (disable)
		time_count = 0;
	}
}

ISR(INT1_vect)
{
	static bool button_state_0 = true; //released
	bool button_state = PIND & (1 << BUTTON_BIT);
	if (button_state != button_state_0) {
		button_state_0 = button_state;

		if (button_state) { //button released
			if (TIMSK2 & (1 << TOIE2)) { //timer interrupt enabled (active state)
				if (time_count >= DEBOUNCE) {
					on_button_pressed();
				}
				TIMSK2 &= // Timer/Counter2Interrupt Mask Register
						~(1 << TOIE2); //TOIE2: Timer/Counter2 Overflow Interrupt Enable (disable)
			}
			time_count = 0;
		} else { //button pressed
			time_count = 0;
			if (!(TIMSK2 & (1 << TOIE2))) { //timer interrupt disabled (passive state)
				TIMSK2 |= // Timer/Counter2Interrupt Mask Register
						(1 << TOIE2); //TOIE2: Timer/Counter2 Overflow Interrupt Enable (enable)
			}
		}
	}
}

