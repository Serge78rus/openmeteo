/*
 * diff.h
 *
 *  Created on: 4 нояб. 2019 г.
 *      Author: serge78rus
 */

#ifndef DIFF_H_
#define DIFF_H_

#include <stdint.h>

/*
 * calculate pressure difference by hour
 */

void diff_init(void);
void diff_put(uint32_t value);
uint32_t diff_calc(void);

#endif /* DIFF_H_ */
