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

extern FILE lcd;

#endif /* LCD_H_ */
