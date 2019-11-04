/*
 * lcd.h
 *
 *  Created on: 25 окт. 2019 г.
 *  Author: serge78rus
 *  License GPL-3.0
 */

#ifndef LCD_H_
#define LCD_H_

#include <stdio.h>
#include <stdint.h>

/*
 * LCD 1602
 *
 * Circuit		GPIO	Anduino pin
 *
 * RS			PB0		8
 * EN			PB1		9
 * D4			PD4		4
 * D5			PD5		5
 * D6			PD6		6
 * D7			PD7		7
 *
 */

#define lcd_RS_BIT 0
#define lcd_EN_BIT 1
#define lcd_D_MASK 0xf0
#define lcd_D_SHIFT 4

void lcd_init(void);
void lcd_clear(void); //after clear cursor automatic moved to home
void lcd_home(void);
void lcd_move_cursor(uint8_t col, uint8_t row);
void lcd_fill_space(uint8_t cnt);

void lcd_show_temp(char sign, int16_t int_part, int16_t fract_part);
void lcd_show_bad_temp(void);
void lcd_show_hum(uint16_t int_part, uint16_t fract_part);
void lcd_show_bad_hum(void);
void lcd_show_press(int16_t int_part, int16_t fract_part);
void lcd_show_bad_press(void);
void lcd_show_diff(char sign, int16_t int_part, int16_t fract_part);
void lcd_show_bad_diff(void);

extern FILE lcd;

#endif /* LCD_H_ */
