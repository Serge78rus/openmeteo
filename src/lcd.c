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

#include "lcd.h"

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
