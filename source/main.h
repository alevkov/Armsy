/*
The MIT License (MIT)

Copyright (c) 2014 ARMstrap Community
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

/*
 * INCLUDES
 */


#include "defines.h"
#include "midi.h"
#include "misc.h"


/*
 * ~INCLUDES
 */


/*
 * PROTOTYPES
 */



/* initialize pins, UART, I2S, etc. */
void init();

/* initialize external interrupts */
void init_ei();

/* initialize LED */
void init_led();

/* initialize i2s port */
void init_i2s();

/* initialize button */
void init_button();

/* delay function for general bumming around */
void delay(uint32_t ms);

void test_note();
