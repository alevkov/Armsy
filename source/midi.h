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

#define MIDI_USART_PORT         USART2
#define MIDI_USART_PORT_RCC     RCC_APB1Periph_USART2
#define MIDI_USART_BAUDRATE     31250
#define MIDI_USART_DMA_REQ      USART_DMAReq_Rx

#define MIDI_GPIO_PORT          GPIOD
#define MIDI_GPIO_PORT_RCC      RCC_AHB1Periph_GPIOD
#define MIDI_GPIO_PIN_RX        GPIO_Pin_6
#define MIDI_GPIO_PIN_RX_SRC    GPIO_PinSource6
#define MIDI_GPIO_PIN_RX_AF     GPIO_AF_USART2

#define MIDI_DMA_STREAM         DMA1_Stream5
#define MIDI_DMA_RCC            RCC_AHB1Periph_DMA1
#define MIDI_DMA_CHANNEL        DMA_Channel_4

#define MIDI_BASIC_MSG_DATABYTES_MAX 2
#define MIDI_MSG_TYPE_NOTE_OFF          0x80
#define MIDI_MSG_TYPE_NOTE_ON           0x90
#define MIDI_MSG_TYPE_AFTERTOUCH        0xA0
#define MIDI_MSG_TYPE_CTRL_CHANGE       0xB0
#define MIDI_MSG_TYPE_PROGRAM_CHANGE    0xC0
#define MIDI_MSG_TYPE_CHAN_PRESSURE     0xD0
#define MIDI_MSG_TYPE_PITCH_WHEEL       0xE0

#define MIDI_MSG_STATUS_CLEAR       0x00
#define MIDI_MSG_STATUS_INITIALIZED 0x01
#define MIDI_MSG_STATUS_UNREAD      0x02

#define MIDI_RAW_BUFFER_SIZE 6
#define MIDI_MSG_BUFFER_SIZE 8

#define MIDI_NOTE_TABLE_SIZE 128
#define MIDI_NOTE_A4_INDEX 69

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

#include "fixedPoint.h"

typedef struct {
    uint8_t msgType;
    uint8_t lowNibble;
    uint8_t dataBytes[MIDI_BASIC_MSG_DATABYTES_MAX];
    uint8_t numberOfDataBytes;
    uint8_t dataByteIndex;
    uint8_t status;
} Midi_basicMsg;

extern __IO uint8_t midi_rawBuffer[MIDI_RAW_BUFFER_SIZE];

extern uint8_t midi_rawBufferIndex;

extern Midi_basicMsg midi_msgBuffer[MIDI_MSG_BUFFER_SIZE];
extern uint8_t midi_msgBufferWriteIndex;
extern uint8_t midi_msgBufferReadIndex;

extern FixedPoint midi_notes[MIDI_NOTE_TABLE_SIZE];

//void midi_initBuffers();
void midi_initNotesTable();
void midi_initGpio();
void midi_initUSART();
void midi_initDMA();
void midi_init();

uint8_t midi_getNumberOfDataBytesForMsgType(uint8_t msgType);

void midi_initMsg(Midi_basicMsg * msg, uint8_t byte);
void midi_addDataByteToMsgIfAble(Midi_basicMsg * msg, uint8_t byte);

void midi_catchUpWithRawBuffer();

int midi_getMsgIfAble(Midi_basicMsg * msg);

#endif /* SOURCE_MIDI_H_ */



