/*
 * twi.c
 *
 *  Created on: 27 окт. 2019 г.
 *  Author: serge78rus
 *  License GPL-3.0
 */

#include <avr/io.h>
#include <util/twi.h>

#include "twi.h"
#include "debug.h"

//select one of:
#define F_TWI 100000L
//#define F_TWI 400000L
//if use low

//static inline bool send_start(void);

void twi_init(void)
{
	TWSR = // TWSR – TWI Status Register
			(0 << TWPS1) | (0 << TWPS0); // TWPS: TWI Prescaler Bits (Prescaler = 1)
	TWBR = ((F_CPU / F_TWI) - 16) / 2; // from datasheets: F_TWI = CPU_F / (16 + (2 * TWBR))
	TWCR = //TWCR – TWI Control Register
			(0 << TWINT) | // TWINT: TWI Interrupt Flag
			(1 << TWEA) | // TWEA: TWI Enable Acknowledge Bit
			(0 << TWSTA) | // TWSTA: TWI START Condition Bit
			(0 << TWSTO) | // TWSTO: TWI STOP Condition Bit
			(0 << TWWC) | // TWWC: TWI Write Collision Flag
			(1 << TWEN) | // TWEN: TWI Enable Bit
			(0 << TWIE); // TWIE: TWI Interrupt Enable
}

bool twi_send_to(uint8_t slave, uint8_t* data, int count)
{
	uint8_t status;

	while (TWCR & (1 << TWSTO)) { //Wait and of previous STOP transmission finished
	}

	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN); //Send START condition
	while (!(TWCR & (1 << TWINT))) { //Wait for TWINT Flag set
	}
	if ((TWSR & TW_STATUS_MASK) != TW_START) {
		ERROR();
		return false;
	}

	TWDR = TW_WRITE | (slave << 1);
	TWCR = (1 << TWINT) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT))) {
	}
	status = TWSR & TW_STATUS_MASK;
	if (status != TW_MT_SLA_ACK) {
		if (status == TW_MT_SLA_NACK) {
			TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO); //Transmit STOP condition
		}
		ERROR();
		return false;
	}

	while (--count) {
		TWDR = *data++;
		TWCR = (1 << TWINT) | (1 << TWEN);
		while (!(TWCR & (1 << TWINT))) {
		}
		status = TWSR & TW_STATUS_MASK;
		if (status != TW_MT_DATA_ACK) {
			if (status == TW_MT_DATA_NACK) {
				TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO); //Transmit STOP condition
			}
			ERROR();
			return false;
		}
	}

	//last byte
	TWDR = *data++;
	TWCR = (1 << TWINT) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT))) {
	}
	status = TWSR & TW_STATUS_MASK;
	if (status != TW_MT_DATA_ACK && status != TW_MT_DATA_NACK) {
		ERROR();
		return false;
	}

	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO); //Transmit STOP condition

	return true;
}

bool twi_receive_from(uint8_t slave, uint8_t* data, int count)
{
	uint8_t status;

	while (TWCR & (1 << TWSTO)) { //Wait and of previous STOP transmission finished
	}

	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN); //Send START condition
	while (!(TWCR & (1 << TWINT))) { //Wait for TWINT Flag set
	}
	if ((TWSR & TW_STATUS_MASK) != TW_START) {
		ERROR();
		return false;
	}

	TWDR = TW_READ | (slave << 1);
	TWCR = (1 << TWINT) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT))) {
	}
	status = TWSR & TW_STATUS_MASK;
	if (status != TW_MR_SLA_ACK) {
		if (status == TW_MR_SLA_NACK) {
			TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO); //Transmit STOP condition
		}
		ERROR();
		return false;
	}

	while (--count) {
		TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
		while (!(TWCR & (1 << TWINT))) {
		}
		if ((TWSR & TW_STATUS_MASK) != TW_MR_DATA_ACK) {
			TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO); //Transmit STOP condition
			ERROR();
			return false;
		}
		*data++ = TWDR;
	}

	//last byte
	TWCR = (1 << TWINT) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT))) {
	}
	if ((TWSR & TW_STATUS_MASK) != TW_MR_DATA_NACK) {
		TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO); //Transmit STOP condition
		ERROR();
		return false;
	}
	*data++ = TWDR;

	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO); //Transmit STOP condition

	return true;
}



