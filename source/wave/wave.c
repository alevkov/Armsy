/*
 * wave.c


 *
 *  Created on: Sep 24, 2015
 *      Author: lexlevi
 */

#include "wave.h"

/*
 * NAME: init_wt()
 * Precondition: MIDI packed into Note struct, wt_t empty instance created
 * Main Units: Sample Rate, Frequency, table->size (number of samples per period), Amplitude
 * Postcondition: wt_t struct initialized with a period wavetable and a char indicating wave type
 */

void init_wt(char for_type, Note * with_note, wt_t * table)
{
	uint16_t A = with_note->amp;

	table->size = 2*(SAMPLE_RATE / with_note->freq); /* mult by 2 because 2 channels to DAC*/

	volatile uint32_t i;

	if (for_type == 's') /* SINE */
	{
		for (i = 0; i <= table->size; i++)
		{
			if (i % 2)
			{
				table->table_array[i] = (A) * sin(2 * PI * (i / (table->size)));
			}
			else
			{
				table->table_array[i] = 0;
			}
		}
	}
	else if (for_type == 'q') /* SQR */
	{
		for (i = 0; i <= table->size / 2; i++) {
			if (i % 2)
			{
				table->table_array[i] = A;
			}
			else
			{
				table->table_array[i] = 0;
			}
		}
		for (i = table->size / 2; i <= table->size; i++) {
			table->table_array[i] = 0;
		}
	}
	else if (for_type == 't') /* TRI */
	{
		/*
		 * I swear the triangle equation
		 * shouldn't be this complicated
		 */
		uint16_t peak_value, diff = 0;

		for (i = 0; i < (table->size) / 2; i++)
		{
			table->table_array[i] = (A / (table->size)) * i;
			peak_value = table->table_array[i];
		}

		diff = table->table_array[1] - table->table_array[0];

		for (i = (table->size) / 2; i < table->size; i++)
		{
			table->table_array[i] = peak_value;
			peak_value -= diff;
		}
	}
}
