/*
 * bmp180.c
 *
 *  Created on: 29 окт. 2019 г.
 *  Author: serge78rus
 *  License GPL-3.0
 */

#include <util/delay.h>

#include "bmp180.h"
#include "twi.h"
#include "debug.h"

#define SLAVE_ADDR 0x77
//#define TRACE_CALIBR_DATA
//#define TRACE_TEMP

//calibration data
static bool calibration_valid = false;
static int16_t ac1;
static int16_t ac2;
static int16_t ac3;
static uint16_t ac4;
static uint16_t ac5;
static uint16_t ac6;
static int16_t b1;
static int16_t b2;
static int16_t mb;
static int16_t mc;
static int16_t md;

int16_t _bmp180_temp;
int32_t _bmp180_press;

static inline bool read_calibration(void);
static inline bool read_temp(int32_t* ut); //ut must be 0 before call
static inline bool read_press(bmp180_Mode mode, int32_t* up); //up must be 0 before call
static inline bool read_rg8(uint8_t rg, void* data);
static inline bool read_rg16(uint8_t rg, void* data);
static inline bool read_rg24(uint8_t rg, void* data);
static inline bool write_rg8(uint8_t rg, uint8_t data);
static inline void swap_2_byte(uint8_t* data);
static inline void swap_3_byte(uint8_t* data);

void bmp180_init(void)
{
	if (read_calibration()) {
		calibration_valid = true;
	}
}

uint8_t bmp180_get_chip_id(void)
{
	uint8_t id = 0;
	if (!read_rg8(0xd0, &id)) {
		ERROR();
		return 0;
	}
	return id;
}

bool bmp180_update(bmp180_Mode mode)
{
	if (!calibration_valid) {
		if (read_calibration()) {
			calibration_valid = true;
		} else {
			ERROR();
			return false;
		}
	}

	int32_t ut = 0;
	int32_t up = 0;

	if (!read_temp(&ut)) {
		ERROR();
		return false;
	}
	if (!read_press(mode, &up)) {
		ERROR();
		return false;
	}

	int32_t x1 = ((ut - (int32_t)ac6) * (int32_t)ac5) >> 15;
	int32_t x2 = ((int32_t)mc << 11) / (x1 + (int32_t)md);
	int32_t b5 = x1 + x2;
	_bmp180_temp = (b5 + 8) >> 4;

#ifdef TRACE_TEMP
	TRACEF("ut: %li x1: %li x2: %li b5: %li",
			ut, x1, x2, b5);
#endif

	int32_t b6 = b5 - 4000;
	x1 = ((int32_t)b2 * ((b6 * b6) >> 12)) >> 11;
	x2 = ((int32_t)ac2 * b6) >> 11;
	int32_t x3 = x1 + x2;
	int32_t b3 = (((((int32_t)ac1 << 2) + x3) << mode) + 2) >> 2;
	x1 = ((int32_t)ac3 * b6) >> 13;
	x2 = ((int32_t)b1 * ((b6 * b6) >> 12)) >> 16;
	x3 = ((x1 + x2) + 2) >> 2;
	uint32_t b4 = (uint32_t)ac4 * (uint32_t)(x3 + 32768) >> 15;
	uint32_t b7 = ((uint32_t)up - b3) * (50000 >> mode);
	if (b7 < 0x80000000) {
		_bmp180_press = (b7 << 1) / b4;
	} else {
		_bmp180_press = (b7 / b4) << 1;
	}
	x1 = (_bmp180_press >> 8) * (_bmp180_press >> 8);
	x1 = (x1 * 3038) >> 16;
	x2 = (-7357 * _bmp180_press) >> 16;
	_bmp180_press = _bmp180_press + ((x1 + x2 + 3791) >> 4);

	return true;
}

static inline bool read_calibration(void)
{
	if (!read_rg16(0xaa, &ac1)) {
		ERROR();
		return false;
	}
	if (!read_rg16(0xac, &ac2)) {
		ERROR();
		return false;
	}
	if (!read_rg16(0xae, &ac3)) {
		ERROR();
		return false;
	}
	if (!read_rg16(0xb0, &ac4)) {
		ERROR();
		return false;
	}
	if (!read_rg16(0xb2, &ac5)) {
		ERROR();
		return false;
	}
	if (!read_rg16(0xb4, &ac6)) {
		ERROR();
		return false;
	}
	if (!read_rg16(0xb6, &b1)) {
		ERROR();
		return false;
	}
	if (!read_rg16(0xb8, &b2)) {
		ERROR();
		return false;
	}
	if (!read_rg16(0xba, &mb)) {
		ERROR();
		return false;
	}
	if (!read_rg16(0xbc, &mc)) {
		ERROR();
		return false;
	}
	if (!read_rg16(0xbe, &md)) {
		ERROR();
		return false;
	}

#ifdef TRACE_CALIBR_DATA
	TRACEF("ac1: 0x%04X ac2: 0x%04X ac3: 0x%04X", (unsigned)ac1, (unsigned)ac2, (unsigned)ac3);
	TRACEF("ac4: 0x%04X ac5: 0x%04X ac6: 0x%04X", (unsigned)ac4, (unsigned)ac5, (unsigned)ac6);
	TRACEF("b1: 0x%04X b2: 0x%04X", (unsigned)b1, (unsigned)b2);
	TRACEF("mb: 0x%04X mc: 0x%04X md: 0x%04X", (unsigned)mb, (unsigned)mc, (unsigned)md);
#endif //TRACE_CALIBR_DATA

	return true;
}

static inline bool read_temp(int32_t* ut)
{
	if (!write_rg8(0xf4, 0x2e)) {
		ERROR();
		return false;
	}

	uint8_t status;
	do {
		if (!read_rg8(0xf4, &status)) {
			ERROR();
			return false;
		}
	} while (status & (1 << 5));

	return read_rg16(0xf6, ut);
}

static inline bool read_press(bmp180_Mode mode, int32_t* up)
{
	if (!write_rg8(0xf4, 0x34 | (mode << 6))) {
		ERROR();
		return false;
	}

	uint8_t status;
	do {
		if (!read_rg8(0xf4, &status)) {
			ERROR();
			return false;
		}
	} while (status & (1 << 5));

	if (!read_rg24(0xf6, up)) {
		ERROR();
		return false;
	}

	*up >>= (8 - mode);

	return true;
}

static inline bool read_rg8(uint8_t rg, void* data)
{
	if (twi_send_to(SLAVE_ADDR, &rg, sizeof(rg))) {
		return twi_receive_from(SLAVE_ADDR, data, 1);
	}
	return false;
}

static inline bool read_rg16(uint8_t rg, void* data)
{
	if (twi_send_to(SLAVE_ADDR, &rg, sizeof(rg))) {
		if (twi_receive_from(SLAVE_ADDR, data, 2)) {
			swap_2_byte(data);
			return true;
		}
	}
	return false;
}

static inline bool read_rg24(uint8_t rg, void* data)
{
	if (twi_send_to(SLAVE_ADDR, &rg, sizeof(rg))) {
		if (twi_receive_from(SLAVE_ADDR, data, 3)) {
			swap_3_byte(data);
			return true;
		}
	}
	return false;
}

static inline bool write_rg8(uint8_t rg, uint8_t data)
{
	uint8_t d[2] = {rg, data};
	return twi_send_to(SLAVE_ADDR, d, sizeof(d));
}

static inline void swap_2_byte(uint8_t* data)
{
	uint8_t tmp = data[0];
	data[0] = data[1];
	data[1] = tmp;
}

static inline void swap_3_byte(uint8_t* data)
{
	uint8_t tmp = data[0];
	data[0] = data[2];
	data[2] = tmp;
}
