/*
 * bmp180.h
 *
 *  Created on: 29 окт. 2019 г.
 *  Author: serge78rus
 *  License GPL-3.0
 */

#ifndef BMP180_H_
#define BMP180_H_

#include <stdint.h>
#include <stdbool.h>

typedef enum { //oversampling
	bmp180_MODE_ULTRA_LOW_POWER			= 0,
	bmp180_MODE_STANDARD 				= 1,
	bmp180_MODE_HIGH_RESOLUTION			= 2,
	bmp180_MODE_ULTRA_HIGH_RESOLUTION	= 3
} bmp180_Mode;

void bmp180_init(void);
uint8_t bmp180_get_chip_id(void); //if error - return 0, chip_id must be 0x55 for BMP180
bool bmp180_update(bmp180_Mode mode);
inline int16_t bmp180_get_temp(void); //in 0.1 grad
inline bool bmp180_is_temp_negative(void);
inline int32_t bmp180_get_press(void); //in Pa
inline int32_t bmp180_get_press_mm(void); //in 0.01 mm

extern int16_t _bmp180_temp;
extern int32_t _bmp180_press;

inline int16_t bmp180_get_temp(void)
{
	return _bmp180_temp;
}

inline bool bmp180_is_temp_negative(void)
{
	return _bmp180_temp < 0;
}

inline int32_t bmp180_get_press(void)
{
	return _bmp180_press;
}

inline int32_t bmp180_get_press_mm(void)
{
	return (_bmp180_press * 10000) / 13332;
}

#endif /* BMP180_H_ */
