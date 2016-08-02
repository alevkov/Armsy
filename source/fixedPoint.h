/*
 * fixedPoint.h
 *
 *  Created on: Jul 22, 2016
 *      Author: lexlevi
 */

#ifndef SOURCE_FIXEDPOINT_H_
#define SOURCE_FIXEDPOINT_H_

#include <stdint.h>

typedef union {
	struct {
		uint16_t f;
		uint16_t i;
	} p;
	uint32_t c;
} FixedPoint;

#endif /* SOURCE_FIXEDPOINT_H_ */
