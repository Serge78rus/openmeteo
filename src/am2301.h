/*
 * am2301.h
 *
 *  Created on: 27 окт. 2019 г.
 *  Author: serge78rus
 *  License GPL-3.0
 */

#ifndef AM2301_H_
#define AM2301_H_

#include <stdint.h>
#include <stdbool.h>

// AM2301 connected to PD2 (INT0)
#define am2301_BIT 2

void am2301_init(void);
bool am2301_update(void);
inline uint16_t am2301_get_humidity(void);
inline int16_t am2301_get_temp(void);

extern uint16_t _am2301_humidity;
extern int16_t _am2301_temp;

inline uint16_t am2301_get_humidity(void)
{
	return _am2301_humidity;
}

inline int16_t am2301_get_temp(void)
{
	return _am2301_temp;
}

#endif /* AM2301_H_ */
