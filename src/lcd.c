/*
 * lcd.c
 *
 *  Created on: 25 окт. 2019 г.
 *  Author: serge78rus
 *  License GPL-3.0
 */

#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

#include "lcd.h"

#define LINE_LEN 16

#define TEMP_COL 0
#define TEMP_ROW 0
#define HUM_COL 10
#define HUM_ROW 0
#define PRESS_COL 0
#define PRESS_ROW 1
#define DIFF_COL 8
#define DIFF_ROW 1

#define TEMP_LEN (HUM_COL)
#define HUM_LEN (LINE_LEN - HUM_COL)
#define PRESS_LEN (DIFF_COL)
#define DIFF_LEN (LINE_LEN - DIFF_COL)

// commands
#define CLEARDISPLAY	0x01
#define RETURNHOME		0x02
#define ENTRYMODESET	0x04
#define DISPLAYCONTROL	0x08
#define CURSORSHIFT		0x10
#define FUNCTIONSET		0x20
#define SETCGRAMADDR	0x40
#define SETDDRAMADDR	0x80

// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

//private functions
static inline void write4bits(uint8_t value);
static inline void write_cmd(uint8_t cmd);
static int lcd_putchar(char c, FILE *stream);

FILE lcd = FDEV_SETUP_STREAM(lcd_putchar, NULL, _FDEV_SETUP_WRITE);

void lcd_init(void)
{
	DDRB |= //GPIO mode output
			(1 << lcd_RS_BIT) |
			(1 << lcd_EN_BIT);
	DDRD |= //GPIO mode output
			lcd_D_MASK; //LCD D4...D7

	_delay_ms(50);

	PORTB &= ~((1 << lcd_RS_BIT) | (1 << lcd_EN_BIT));

	write4bits(0x03);
	_delay_ms(5);
	write4bits(0x03);
	_delay_us(100);
	write4bits(0x03);

	write4bits(0x02);

	write_cmd(LCD_FUNCTIONSET | (LCD_4BITMODE | LCD_2LINE | LCD_5x8DOTS));
	write_cmd(LCD_DISPLAYCONTROL | (LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF));
	lcd_clear();
	write_cmd(LCD_ENTRYMODESET | (LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT));
}

void lcd_clear(void)
{
	write_cmd(LCD_CLEARDISPLAY);
	_delay_ms(2);
}

void lcd_home(void)
{
	write_cmd(LCD_RETURNHOME);
	_delay_ms(2);
}

void lcd_move_cursor(uint8_t col, uint8_t row)
{
	if (row) {
		col += 0x40;
	}
	write_cmd(LCD_SETDDRAMADDR | col);
}

void lcd_fill_space(uint8_t cnt)
{
	while (cnt--) {
		lcd_putchar(' ', &lcd);
	}
}

void lcd_show_temp(char sign, int16_t int_part, int16_t fract_part)
{
	static const char FMT_STR[] PROGMEM = "%c%i.%01i""\xdf""C";

	lcd_move_cursor(TEMP_COL, TEMP_ROW);
	int len = fprintf_P(&lcd, FMT_STR, sign, int_part, fract_part);
	if (len < TEMP_LEN) {
		lcd_fill_space(TEMP_LEN - len);
	}
}

void lcd_show_bad_temp(void)
{
	static const char FMT_STR[] PROGMEM = "---""\xdf""C";

	lcd_move_cursor(TEMP_COL, TEMP_ROW);
	int len = fprintf_P(&lcd, FMT_STR);
	if (len < TEMP_LEN) {
		lcd_fill_space(TEMP_LEN - len);
	}
}

void lcd_show_hum(uint16_t int_part, uint16_t fract_part)
{
	static const char FMT_STR[] PROGMEM = "%3u.%01u%%";

	lcd_move_cursor(HUM_COL, HUM_ROW);
	int len = fprintf_P(&lcd, FMT_STR, int_part, fract_part);
	if (len < HUM_LEN) {
		lcd_fill_space(HUM_LEN - len);
	}
}

void lcd_show_bad_hum(void)
{
	static const char FMT_STR[] PROGMEM = "---%%";

	lcd_move_cursor(HUM_COL, HUM_ROW);
	int len = fprintf_P(&lcd, FMT_STR);
	if (len < HUM_LEN) {
		lcd_fill_space(HUM_LEN - len);
	}
}

void lcd_show_press(int16_t int_part, int16_t fract_part)
{
	static const char FMT_STR[] PROGMEM = "%i.%01imm";

	lcd_move_cursor(PRESS_COL, PRESS_ROW);
	int len = fprintf_P(&lcd, FMT_STR, int_part, fract_part);
	if (len < PRESS_LEN) {
		lcd_fill_space(PRESS_LEN - len);
	}
}

void lcd_show_bad_press(void)
{
	static const char FMT_STR[] PROGMEM = "---mm";

	lcd_move_cursor(PRESS_COL, PRESS_ROW);
	int len = fprintf_P(&lcd, FMT_STR);
	if (len < PRESS_LEN) {
		lcd_fill_space(PRESS_LEN - len);
	}
}

void lcd_show_diff(char sign, int16_t int_part, int16_t fract_part)
{
	static const char FMT_STR[] PROGMEM = "%c%i.%02i/h";
	static const char NULL_FMT_STR[] PROGMEM = " stable";

	lcd_move_cursor(DIFF_COL, DIFF_ROW);

	//TODO clear unused code
	//int len = 0;
	if (int_part < 10) {
		lcd_fill_space(1);
		//len = 1;
	}

	if (int_part || fract_part) {
		/*len +=*/ fprintf_P(&lcd, FMT_STR, sign, int_part, fract_part);
	} else {
		/*len +=*/ fprintf_P(&lcd, NULL_FMT_STR);
	}

	/*
	if (len < DIFF_LEN) {
		lcd_fill_space(DIFF_LEN - len);
	}
	*/
}

void lcd_show_bad_diff(void)
{
	static const char FMT_STR[] PROGMEM = "   ---/h";

	lcd_move_cursor(DIFF_COL, DIFF_ROW);
	//TODO clear unused code
	/*int len =*/ fprintf_P(&lcd, FMT_STR);
	/*
	if (len < DIFF_LEN) {
		lcd_fill_space(DIFF_LEN - len);
	}
	*/
}

/*
 * private functions
 */

static inline void write4bits(uint8_t value)
{
	PORTD = (PORTD & ~lcd_D_MASK) | ((value << lcd_D_SHIFT) & lcd_D_MASK);

	//EN pulse
	PORTB |= (1 << lcd_EN_BIT);
	_delay_us(1);
	PORTB &= ~(1 << lcd_EN_BIT);
	_delay_us(60);
}

static inline void write_cmd(uint8_t cmd)
{
	PORTB &= ~(1 << lcd_RS_BIT);
    write4bits(cmd >> 4);
    write4bits(cmd);
}

static int lcd_putchar(char c, FILE *stream)
{
	PORTB |= (1 << lcd_RS_BIT);
    write4bits((uint8_t)c >> 4);
    write4bits((uint8_t)c);

    return 0;
}
