/*
 * diff.c
 *
 *  Created on: 4 нояб. 2019 г.
 *      Author: serge78rus
 */

#include "diff.h"
#include "def.h"
#include "debug.h"

#define BUFF_SIZE 120

static int32_t buff[BUFF_SIZE];
static int8_t last_idx;

void diff_init(void)
{
	for (last_idx = 0; last_idx < BUFF_SIZE; ++last_idx) {
		buff[last_idx] = BAD_INT32;
	}
	last_idx = -1;
}

void diff_put(uint32_t value)
{
	if (++last_idx == BUFF_SIZE) {
		last_idx = 0;
	}
	buff[last_idx] = value;
}

uint32_t diff_calc(void)
{
	int8_t idx = last_idx;
	int32_t prev_sum = 0;
	int32_t last_sum = 0;
	uint8_t prev_cnt = 0;
	uint8_t last_cnt = 0;

	for (int8_t i = 0; i < BUFF_SIZE / 2; ++i) {
		if (++idx == BUFF_SIZE) {
			idx = 0;
		}
		if (buff[idx] != BAD_INT32) {
			prev_sum += buff[idx];
			++prev_cnt;
		}
	}

	for (int8_t i = 0; i < BUFF_SIZE / 2; ++i) {
		if (++idx == BUFF_SIZE) {
			idx = 0;
		}
		if (buff[idx] != BAD_INT32) {
			last_sum += buff[idx];
			++last_cnt;
		}
	}

	if (last_cnt && prev_cnt) {
		return (last_sum / last_cnt) - (prev_sum / prev_cnt);
	} else {
		return BAD_INT32;
	}
}



