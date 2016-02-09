/*
 * midi.h
 *
 *
 *  Created on: Sep 23, 2015
 *      Author: lexlevi
 *
 *
 * MIDI OUT --> UART2 RX, PIN D6
 *
 *
 *
 * Status Byte	Data Byte 1	Data Byte 2	Message	Legend
	1000nnnn	0kkkkkkk	0vvvvvvv	Note Off			n=channel* 	k=key # 0-127(60=middle C) v=velocity (0-127)
	1001nnnn	0kkkkkkk	0vvvvvvv	Note On				n=channel 	k=key # 0-127(60=middle C) v=velocity (0-127)
	1010nnnn	0kkkkkkk	0ppppppp	Poly Key Pressure	n=channel 	k=key # 0-127(60=middle C) p=pressure (0-127)
	1011nnnn	0ccccccc	0vvvvvvv	Controller Change	n=channel 	c=controller v=controller value(0-127)
	1100nnnn	0ppppppp	[none]	    Program Change		n=channel 	p=preset number (0-127)
	1101nnnn	0ppppppp	[none]		Channel Pressure	n=channel 	p=pressure (0-127)
	1110nnnn	0fffffff	0ccccccc	Pitch Bend			n=channel 	c=coarse f=fine (c+f = 14-bit resolution)
 *
 *
 *  A sample message for turning on a note (middle C) on MIDI channel #5 very loudly (with a velocity or force of 127, the maximum):
 *	    status byte	data byte	data byte
 *		10010100	00111100	01111111
 *
 */


#ifndef SOURCE_MIDI_H_
#define SOURCE_MIDI_H_

//pin for UART2
#define USART2_RX_PORT GPIOD
#define USART2_RX_PIN GPIO_Pin_6   //port D
#define USART2_RX_PIN_SRC GPIO_PinSource6
#define USART2_PIN_AF GPIO_AF_USART2

#define MIDI_BAUD_RATE 31250 // standard MIDI Baud rate

#define MIDI_DATABYTES_ARRAY_SIZE 2

#define MIDI_MSG_TYPE_NOTE_OFF          0x80
#define MIDI_MSG_TYPE_NOTE_ON           0x90
#define MIDI_MSG_TYPE_AFTERTOUCH        0xA0
#define MIDI_MSG_TYPE_CTRL_CHANGE       0xB0
#define MIDI_MSG_TYPE_PROGRAM_CHANGE    0xC0
#define MIDI_MSG_TYPE_CHAN_PRESSURE     0xD0
#define MIDI_MSG_TYPE_PITCH_WHEEL       0xE0

#define MIDI_MSG_STATE_CLEAR       0x00
#define MIDI_MSG_STATE_INITIALIZED 0x01
#define MIDI_MSG_STATE_UNREAD      0x02

#define MIDI_RAW_BUFFER_SIZE 30
#define MIDI_MSG_BUFFER_SIZE 8

#define MIDI_NOTE_TABLE_SIZE 128
#define MIDI_NOTE_A4_INDEX 69
#define MIDI_NOTE_A4_FREQUENCY 440

#include "stm32f4xx_rcc.h"
#include "stm32f4xx_usart.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_syscfg.h"
#include "stm32f4xx_dma.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_exti.h"
#include "stm32f4xx_spi.h"
#include "stm32f4xx_i2c.h"

#include "wave/wave.h"

typedef struct { // defines MIDI message type
	/*
	 * 1) Array of data bytes, max size = 2
	 * 2) State (CLEAR, INIT, UNREAD)
	 * 3) Message type 0x80 - 0xE0
	 * 4) Data byte idx [0], [1]
	 * 5) Low nibble of status byte (channel)
	 * 6) Number of data bytes
	 */
	uint8_t data[MIDI_DATABYTES_ARRAY_SIZE];
	uint8_t state;
	uint8_t msg_type;
	uint8_t data_byte_index;
	uint8_t low_nibble;
	uint8_t number_of_data_bytes;
} midi_msg_t;

extern volatile uint8_t midi_raw_buffer[MIDI_RAW_BUFFER_SIZE]; // to be parsed

extern midi_msg_t midi_msg_buffer[MIDI_MSG_BUFFER_SIZE];

extern uint8_t midi_raw_buffer_idx;
extern uint8_t midi_msg_buffer_write_idx;
extern uint8_t midi_msg_buffer_read_idx;

extern Note midi_notes[MIDI_NOTE_TABLE_SIZE];

void midi_init_buffers();
void midi_init_notes_table();
void midi_init_input_gpio();
void midi_init_USART();
void midi_init_DMA();
void midi_init();

void add_databyte_to_msg(midi_msg_t * msg, uint8_t byte); // if possible!
_Bool msg_is_full(midi_msg_t * msg);

uint8_t get_num_of_databytes_by_msg_type(uint8_t msg_type);
uint8_t get_midi_message_type(midi_msg_t * msg);

void sync_raw_buffer();

#endif /* SOURCE_MIDI_H_ */



