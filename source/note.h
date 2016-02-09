/*
 * note.h

 *
 *  Created on: Sep 24, 2015
 *      Author: lexlevi
 */

#ifndef SOURCE_NOTE_H_
#define SOURCE_NOTE_H_

#include "stdint.h"

/*
 *
 * Well, this is a note.
 */
typedef struct {
	uint16_t freq;
	uint16_t idx;
	uint16_t amp;
} Note;

#endif /* SOURCE_NOTE_H_ */
