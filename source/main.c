#include "main.h"
#include "math.h"


#define MAX_STRLEN 6 // this is the maximum string length of our string in characters

/* Sine wave */
int SAMPLE[368];
float SAMPLE_SIZE;

/* Midi data */
FLAG message_received;
volatile uint16_t midi_raw_buffer[6]; // this will hold the recieved string
volatile static uint8_t midi_buffer_counter = 0; // this counter is used to determine the string length
Midi_basicMsg *message;

float noteNumToFreq(uint8_t noteNumber) 
{
	s8 offset = noteNumber - 69;  //difference from A4 (440Hz, MIDI-note 69)
	float freq = 440.0f;
 
	if (offset > 0) 
	{
		while (offset > 11) {
			offset -= 12;
			freq *= 2.0f;
		}
		while (offset > 0) {
			offset--;
			freq *= 1.059463;  // 2^(1/12)
		}
	} 
	else if (offset < 0)
	{
		while (offset < -11) {
			offset += 12;
			freq *= 0.5f;
		}
		while (offset < 0) {
			offset++;
			freq *= 0.9438743f; // 2^(-1/12)
		}
	}
	return freq;
}


void generateArray(int* AUDIO_SAMPLE, float freq)
{
	int amp = 1000; //amplitude of wave (within 16bit signed integer)
	int SR = 48000; //sample rate of I2S channel
	SAMPLE_SIZE = 2 * SR / freq;
	int i;
	for(i = 0; i <= SAMPLE_SIZE; i++){ //mult SR/Freq by 2 b/c of two channels
		if(i % 2) {
			AUDIO_SAMPLE[i] = amp * sin(2 * 3.14 * (i / SAMPLE_SIZE));
		}
		else
			AUDIO_SAMPLE[i] = 0; //left channel
	}
}

void toggle_light(uint32_t delay)
{
	GPIOC->ODR ^= ((uint16_t)0x0002);
	delay_for(delay);
}

void delay_for(uint32_t ms)
{
    ms *= 1075; // for 8Mhz xtal
    while(ms--)
    {
    	__asm__ volatile ("nop");
    }
}

void init()
{
	/*
	 * Init
	 */
   init_led();
   init_USART2(MIDI_USART_BAUDRATE);
   init_i2s();
}

int main()
{
   init();
   volatile int i = 0;
   for (;;) 
   {	
   		if (message_received)
   		{
   			// clear flag for midi received
   			message_received = 0;
   			// generate frequency based on note value
   			float freq = noteNumToFreq(midi_raw_buffer[1]);
   			generateArray(SAMPLE, freq);
   		} else {
   			if(SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_TXE))
   			{
   				SPI_I2S_SendData(SPI3, SAMPLE[i]);
   				i++;
   				if(i >= (uint16_t)SAMPLE_SIZE) i = 0;
   			}
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

void USART2_IRQHandler(void)
{
	while(USART_GetITStatus(MIDI_USART_PORT, USART_IT_RXNE)) {
		GPIOC->ODR |= ((uint16_t)0x0002);

		message_received = 1;
		// fill raw buffer
		if (USART_ReceiveData(MIDI_USART_PORT) == MIDI_MSG_TYPE_NOTE_OFF)
		{
			midi_buffer_counter = 6 / 2;
		}
		midi_raw_buffer[midi_buffer_counter++] = USART_ReceiveData(MIDI_USART_PORT);
		delay_for(1);

		GPIOC->ODR &= ~((uint16_t)0x0002);
	}
	midi_buffer_counter = 0;
}

// void clearMsg()
// {
// 	for (int i = 0; i < MIDI_RAW_BUFFER_SIZE; ++i)
// 	{
// 		midi_raw_buffer[i] = 0;
// 	}
// }

// void USART2_IRQHandler(void)
// {
// 	while(USART_GetITStatus(MIDI_USART_PORT, USART_IT_RXNE)) {
// 		GPIOC->ODR |= ((uint16_t)0x0002);

// 		message_received = 1;
// 		if (USART_ReceiveData(MIDI_USART_PORT) == MIDI_MSG_TYPE_NOTE_OFF)
// 		{
// 			message->status = MIDI_MSG_TYPE_NOTE_OFF;
// 		}
// 		if (USART_ReceiveData(MIDI_USART_PORT) == MIDI_MSG_TYPE_NOTE_ON)
// 		{
// 			message->status = MIDI_MSG_TYPE_NOTE_ON;
// 		}
// 		midi_raw_buffer[midi_buffer_counter++] = USART_ReceiveData(MIDI_USART_PORT);
// 		delay_for(1);

// 		GPIOC->ODR &= ~((uint16_t)0x0002);
// 	}
// 	//midi_buffer_counter = 0;
// }

