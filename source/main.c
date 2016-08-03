#include "main.h"
#include "math.h"

/* Sine wave */
float SAMPLE_SIZE;
int * sample;

/* Midi data */
FLAG message_received;
Midi_basicMsg message;

#pragma mark - Misc

void toggle_light(uint32_t delay)
{
	GPIOC->ODR ^= ((uint16_t)0x0002);
	delay_for(delay);
}

void delay_for(uint32_t d)
{
    d *= 500;
    while(d--)
    {
    	__asm__ volatile ("nop");
    }
}

#pragma mark - Sound and Midi

float note_num_to_freq(uint8_t noteNumber) 
{
	int offset = noteNumber - 69;  //difference from A4 (440Hz, MIDI-note 69)
	float freq = 440.0;
 
	if (offset > 0) 
	{
		while (offset > 11) {
			offset -= 12;
			freq *= 2.0;
		}
		while (offset > 0) {
			offset--;
			freq *= SEMITONE_UP;  // 2^(1/12)
		}
	} 
	else if (offset < 0)
	{
		while (offset < -11) {
			offset += 12;
			freq *= 0.5;
		}
		while (offset < 0) {
			offset++;
			freq *= SEMITONE_DOWN; // 2^(-1/12)
		}
	}
	return freq;
}

float sample_size(float freq)
{
	SAMPLE_SIZE = (2 * SAMPLE_RATE / freq);
	return SAMPLE_SIZE;
}

int * generate_sample_array(float freq, uint16_t size)
{
	int s[size];
	int i;
	for(i = 0; i <= size; i++) {
		s[i] = (sin(2 * PI * (i / (2 * SAMPLE_RATE / freq)))) >= 0 ? (BASE_AMP * 1) : (BASE_AMP * -1);
	}
	return s;
}

#pragma mark - Init

void init()
{
   init_led();
   init_USART2(MIDI_USART_BAUDRATE);
   init_i2s();
}

#pragma mark - Main

int main()
{
   init();
   clearMidiMsg();

   volatile int i = 0;
   for (;;) 
   {	
   		if (message.status == MIDI_MSG_STATUS_UNREAD)
   		{
   			message.status = MIDI_MSG_STATUS_CLEAR;
   			/* generate frequency based on note value */
   			if (message.msgType == MIDI_MSG_TYPE_NOTE_ON)
   			{
   				float freq = note_num_to_freq(message.dataBytes[0]);

   				uint16_t size = (uint16_t)sample_size(freq);
   				sample = generate_sample_array(freq, size);
   			}
   		}
   		if(SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_TXE) && message.msgType == MIDI_MSG_TYPE_NOTE_ON && message.status == MIDI_MSG_STATUS_CLEAR)
   		{
   			if (i % 2)
   			{
   				SPI_I2S_SendData(SPI3, *(sample + i));
   			}
   			else
   			{
   				SPI_I2S_SendData(SPI3, 0x00);
   			}
   			
   			i++;
   			if(i >= (uint16_t)SAMPLE_SIZE) i = 0;
   		}
	}
	return 0;
}

void init_led() 
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

	GPIO_InitTypeDef ledGPIO;
	GPIO_StructInit(&ledGPIO);
	ledGPIO.GPIO_Mode = GPIO_Mode_OUT;
	ledGPIO.GPIO_Pin = GPIO_Pin_1;
	GPIO_Init(GPIOC, &ledGPIO);
}

void init_USART2() 
{
	GPIO_InitTypeDef GPIO_InitStruct; // this is for the GPIO pins used as TX and RX
	USART_InitTypeDef USART_InitStruct; // this is for the USART1 initilization
	NVIC_InitTypeDef NVIC_InitStructure; // this is used to configure the NVIC (nested vector interrupt controller)

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6;// RX
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF; 			// the pins are configured as alternate function so the USART peripheral has access to them
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;		// this defines the IO speed and has nothing to do with the baudrate!
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;			// this defines the output type as push pull mode (as opposed to open drain)
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOD, &GPIO_InitStruct);					// now all the values are passed to the GPIO_Init() function which sets the GPIO registers
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource6, GPIO_AF_USART2);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

	USART_InitStruct.USART_BaudRate = MIDI_USART_BAUDRATE;				// the baudrate is set to the value we passed into this init function
	USART_InitStruct.USART_WordLength = USART_WordLength_8b;// we want the data frame size to be 8 bits (standard)
	USART_InitStruct.USART_StopBits = USART_StopBits_1;		// we want 1 stop bit (standard)
	USART_InitStruct.USART_Parity = USART_Parity_No;		// we don't want a parity bit (standard)
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // we don't want flow control (standard)
	USART_InitStruct.USART_Mode = USART_Mode_Rx; // we want to enable the transmitter and the receiver
	USART_Init(MIDI_USART_PORT, &USART_InitStruct);
	USART_Cmd(MIDI_USART_PORT, ENABLE);// again all the properties are passed to the USART_Init function which takes care of all the bit setting
	USART_ITConfig(MIDI_USART_PORT, USART_IT_RXNE, ENABLE); // enable the USART1 receive interrupt

	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;		 // we want to configure the USART1 interrupts
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;// this sets the priority group of the USART1 interrupts
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		 // this sets the subpriority inside the group
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			 // the USART1 interrupts are globally enabled
	NVIC_Init(&NVIC_InitStructure);							 // the properties are passed to the NVIC_Init function which takes care of the low level stuff

}

void init_i2s() {
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE);
	RCC_PLLI2SCmd(ENABLE);

	GPIO_InitTypeDef PinInitStruct;
	PinInitStruct.GPIO_Mode=GPIO_Mode_AF;
	PinInitStruct.GPIO_OType=GPIO_OType_PP;
	PinInitStruct.GPIO_PuPd=GPIO_PuPd_NOPULL;
	PinInitStruct.GPIO_Speed=GPIO_Speed_50MHz;
	PinInitStruct.GPIO_Pin = I2S3_SCLK_PIN | I2S3_SD_PIN | I2S3_MCLK_PIN;
	GPIO_Init(GPIOC, &PinInitStruct);

	PinInitStruct.GPIO_Pin = I2S3_WS_PIN;
	GPIO_Init(GPIOA, &PinInitStruct);

	GPIO_PinAFConfig(GPIOA, GPIO_PinSource4, GPIO_AF_SPI3); //I2S3_WS
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource7, GPIO_AF_SPI3); //I2S3_MCK
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_SPI3);//I2S3_SCK
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource12, GPIO_AF_SPI3);//I2S3_SD

	I2S_InitTypeDef I2S_InitType;
	SPI_I2S_DeInit(SPI3);
	I2S_InitType.I2S_AudioFreq = I2S_AudioFreq_48k;
	I2S_InitType.I2S_MCLKOutput = I2S_MCLKOutput_Enable;
	I2S_InitType.I2S_DataFormat = I2S_DataFormat_16b;
	I2S_InitType.I2S_Mode = I2S_Mode_MasterTx;
	I2S_InitType.I2S_Standard = I2S_Standard_Phillips;
	I2S_InitType.I2S_CPOL = I2S_CPOL_Low;

	I2S_Init(SPI3, &I2S_InitType);
	I2S_Cmd(SPI3, ENABLE);
}

void clearMidiMsg()
{
	message.status = MIDI_MSG_STATUS_CLEAR;
	message.msgType = 0;
	message.dataByteIndex = 0;
	int i = 0;
	for (; i < MIDI_BASIC_MSG_DATABYTES_MAX; ++i) {
		message.dataBytes[i] = 0;
	}

}

void USART2_IRQHandler(void)
{
	while(USART_GetITStatus(MIDI_USART_PORT, USART_IT_RXNE)) {
		GPIOC->ODR |= ((uint16_t)0x0002);

		if (USART_ReceiveData(MIDI_USART_PORT) == MIDI_MSG_TYPE_NOTE_OFF)
		{
			message.msgType = MIDI_MSG_TYPE_NOTE_OFF;
			/* resets data byte counter for next message */
			message.dataByteIndex = 0;
			break;
		}
		else if (USART_ReceiveData(MIDI_USART_PORT) == MIDI_MSG_TYPE_NOTE_ON)
		{
			message.msgType = MIDI_MSG_TYPE_NOTE_ON;
			message.dataByteIndex = 0;
		}
		else {
			/* data byte received */
			message.status = MIDI_MSG_STATUS_UNREAD;
			message.dataBytes[message.dataByteIndex++] = USART_ReceiveData(MIDI_USART_PORT);
		}

		GPIOC->ODR &= ~((uint16_t)0x0002);
	}
}

