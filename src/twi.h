/*
 * twi.h
 *
 *  Created on: 27 окт. 2019 г.
 *  Author: serge78rus
 *  License GPL-3.0
 */

#ifndef TWI_H_
#define TWI_H_

/*
 * TWI master mode only
 * send and receive
 */

#include <stdint.h>
#include <stdbool.h>

void twi_init(void);
bool twi_send_to(uint8_t slave, uint8_t* data, int count);
bool twi_receive_from(uint8_t slave, uint8_t* data, int count);

#endif /* TWI_H_ */
