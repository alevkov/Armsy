/*
 * wave.h



 *
 *  Created on: Sep 24, 2015
 *      Author: lexlevi
 */

#ifndef WAVE_H_
#define WAVE_H_

#define WT_SIZE 4096

#define TEST_AMPLITUDE 1000
#define SAMPLE_RATE 48000

#define PI 3.14169265

#include "../note.h"

typedef uint8_t WT_ARRAY[WT_SIZE];

typedef struct {
	/*
	 * type: 's' : sine
	 *  	 'q' : square
	 *  	 't' : tri
	 */
	char type;
	/* max size is 4096
	 * however, depending on the frequency,
	 * the size of table_array will vary
	 */
	uint16_t size;
	WT_ARRAY table_array;
}wt_t;

void init_wt(char for_type, Note * with_note, wt_t * table);

extern uint16_t freqs[128];
extern wt_t wt_sine_test;

#endif /* WAVE_H_ */
