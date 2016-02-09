/*
 * midi.c


 *
 *  Created on: Sep 23, 2015
 *      Author: lexlevi
 */

#include "midi.h"

void midi_init_buffers()
{
	volatile uint8_t i, j = 0;

	while (i < MIDI_RAW_BUFFER_SIZE)
	{
		midi_raw_buffer[i] = 0xFF;
		i++;
	}


	while (j < MIDI_MSG_BUFFER_SIZE)
	{
		//..1000nnnn	0kkkkkkk	0vvvvvvv	Note Off
		midi_msg_buffer[j].state = MIDI_MSG_STATE_CLEAR;
		j++;
	}

	midi_msg_buffer_write_idx = 0;
	midi_msg_buffer_read_idx = 0;
}

void midi_init_notes_table()
{ // generate wavetable based on note
	/* Values from 0 to 127 that indicate pitch */
//	volatile int i;
//	Note note;
//
//	note.amp = 1000;
//	note.idx = MIDI_NOTE_A4_INDEX;
//	calc_freq_for_note(&note, MIDI_NOTE_A4_INDEX); // n->freq = 440
//
//	midi_notes[MIDI_NOTE_A4_INDEX] = note;
}

void midi_init_input_gpio()
{
	/* Enable D6 Pin clock (Rx) */
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

	/*	Only Rx because we are only receiving MIDI */
		GPIO_InitTypeDef USARTGPIOStruct;
		USARTGPIOStruct.GPIO_Pin = USART2_RX_PIN;
		USARTGPIOStruct.GPIO_Mode = GPIO_Mode_AF;
		USARTGPIOStruct.GPIO_OType = GPIO_OType_PP;
		USARTGPIOStruct.GPIO_Speed = GPIO_Speed_50MHz;
		USARTGPIOStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
		GPIO_Init(GPIOD, &USARTGPIOStruct);

		GPIO_PinAFConfig(USART2_RX_PORT, USART2_RX_PIN_SRC, USART2_PIN_AF);
}

void midi_init_USART()
{
	/* USART configuration data
		            - BaudRate = 31250
		            - Word Length = 8 Bits
		            - One Stop Bit
		            - No parity
		            - Hardware flow control disabled (RTS and CTS signals)
		            - Rx enabled only
		 */
		/* Enable USART2 clock */
		RCC_APB2PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

		/* Init USART type */
		USART_InitTypeDef USART_InitStruct;

		/* USART Config */
		USART_InitStruct.USART_BaudRate = MIDI_BAUD_RATE;
		USART_InitStruct.USART_WordLength = USART_WordLength_8b;
		USART_InitStruct.USART_StopBits = USART_StopBits_1;
		USART_InitStruct.USART_Parity = USART_Parity_No;
		USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
		USART_InitStruct.USART_Mode = USART_Mode_Rx;
		USART_Init(USART2, &USART_InitStruct);

		/* Enable USART2 */
		USART_Cmd(USART2, ENABLE);
}

void midi_init_DMA()
{
	//..
}

void midi_init()
{
	midi_init_buffers();
	midi_init_input_gpio();
	midi_init_USART();
}

uint8_t get_num_of_databytes_by_msg_type (uint8_t type)
{
	switch(type) {
	    case MIDI_MSG_TYPE_NOTE_OFF:
	        return 2;
	    case MIDI_MSG_TYPE_NOTE_ON:
	        return 2;
	    case MIDI_MSG_TYPE_AFTERTOUCH:
	        return 2;
	    case MIDI_MSG_TYPE_CTRL_CHANGE:
	        return 2;
	    case MIDI_MSG_TYPE_PROGRAM_CHANGE:
	        return 1;
	    case MIDI_MSG_TYPE_CHAN_PRESSURE:
	        return 1;
	    case MIDI_MSG_TYPE_PITCH_WHEEL:
	        return 2;
	    default:
	        return 0;
	    }
}

/*
 * usart_getchar()
 * This function will provide an interface for receiving a byte of MIDI information
 */
uint16_t usart_getchar(USART_TypeDef* USARTx)
{
	while( !(USARTx->SR & USART_FLAG_RXNE) );
	return USART_ReceiveData(USARTx);
}

_Bool msg_is_full(midi_msg_t * msg)
{
	return msg->number_of_data_bytes == 2;
}
