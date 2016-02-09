#include "main.h"


// ************** DAC **************

// Armstrap -> I2S3 port -> DAC -> audio output

// a function to test wave table
void test_init_wt(char for_type, Note * with_note, wt_t * table)
{
	uint16_t A = with_note->amp;

	table->size = 2*SAMPLE_RATE / with_note->freq; /* mult by 2 because 2 channels to DAC*/

	volatile uint32_t i;
	if (for_type == 'q') /* SQR */
	{
		for (i = 0; i <= table->size / 2; i++) {
			if (i % 2)
			{
				table->table_array[i] = A/2;
			}
			else
			{
				table->table_array[i] = 0;
			}
		}
		for (i = table->size / 2; i <= table->size; i++) {
			if (i % 2)
			{
				table->table_array[i] = -A/2;
			}
			else
			{
				table->table_array[i] = 0;
			}

		}
	}

}

void init()
{
	init_led();
	init_i2s();
	init_ei();
	init_button();
}

void toggle_light()
{

		GPIOC->ODR ^= GPIO_Pin_1;
		delay(90);
}

void main()
{
	init();

	Note a4;
	a4.amp = 1000;
	a4.freq = 1200;
	a4.idx = 69;
	wt_t table;

	/* test triangle wave in A4 frequency (440) */
	test_init_wt('t', &a4, &table);

	volatile int i = 0;
	while (1)
	{
		if(SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_TXE))
		{
		    SPI_I2S_SendData(SPI3, table.table_array[i]);
		    i++;
		    /* fall back to repeat period */
		    if(i>=table.size) i = 0;
		 }

		toggle_light();
	}
}


void init_ei() {
	/* Init user button (C0) interrupt */
	EXTI_InitTypeDef EXTI_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource0);

	EXTI_InitStructure.EXTI_Line = EXTI_Line0;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Init USART interrupt */
}

void init_led() {
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

	GPIO_InitTypeDef ledGPIO;
	GPIO_StructInit(&ledGPIO);
	ledGPIO.GPIO_Mode = GPIO_Mode_OUT;
	ledGPIO.GPIO_Pin = GPIO_Pin_1;
	GPIO_Init(GPIOC, &ledGPIO);
}

void init_button()
{
	// On startup, all peripheral clocks are disabled.  Before using a GPIO
	// pin, its peripheral clock must be enabled.
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

    // For the ARMstrap Eagle board, the USER Button is hooked-up to GPIOC Pin 0.
    // You can see this by looking at the freely available schematics on armstrap.org
    GPIO_InitTypeDef buttonGPIO;
	GPIO_StructInit(&buttonGPIO);
	buttonGPIO.GPIO_Mode = GPIO_Mode_IN;
	buttonGPIO.GPIO_Pin = GPIO_Pin_0;
	GPIO_Init(GPIOC, &buttonGPIO);
}

void init_i2s() {

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE);
	RCC_PLLI2SCmd(ENABLE);

		//Configure GPIOs for SCLK, SD, MCLK, WS
	GPIO_InitTypeDef PinInitStruct;
	PinInitStruct.GPIO_Mode=GPIO_Mode_AF;
	PinInitStruct.GPIO_OType=GPIO_OType_PP;
	PinInitStruct.GPIO_PuPd=GPIO_PuPd_NOPULL;
	PinInitStruct.GPIO_Speed=GPIO_Speed_50MHz;
	PinInitStruct.GPIO_Pin = I2S3_SCLK_PIN | I2S3_SD_PIN | I2S3_MCLK_PIN;

	GPIO_Init(GPIOC, &PinInitStruct);

	PinInitStruct.GPIO_Pin = I2S3_WS_PIN;
	GPIO_Init(GPIOA, &PinInitStruct);

		//prepare output ports for alternate function
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource4, GPIO_AF_SPI3); //I2S3_WS -> Word Select
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource7, GPIO_AF_SPI3); //I2S3_MCK -> Master Clock
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_SPI3);//I2S3_SCK -> Serial Clock
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource12, GPIO_AF_SPI3);//I2S3_SD -> Serial Data

		/* I2S configuration
			 *
			 *
			            - audio frequency = 48k
			            - master clk output enabled
			            - data format = 16b
			            - communication mode = master Tx
			            - standard = Phillips
			            - idle state = low
			 */
	I2S_InitTypeDef I2S_InitStruct;
	SPI_I2S_DeInit(SPI3);
	I2S_InitStruct.I2S_AudioFreq = I2S_AudioFreq_48k; // sample rate
	I2S_InitStruct.I2S_MCLKOutput = I2S_MCLKOutput_Enable;
	I2S_InitStruct.I2S_DataFormat = I2S_DataFormat_16b;
	I2S_InitStruct.I2S_Mode = I2S_Mode_MasterTx;
	I2S_InitStruct.I2S_Standard = I2S_Standard_Phillips;
	I2S_InitStruct.I2S_CPOL = I2S_CPOL_Low;

	I2S_Init(SPI3, &I2S_InitStruct);
	I2S_Cmd(SPI3, ENABLE);
}

void delay(uint32_t ms)
{
    ms *= 3360;
    while(ms--)
    {
    	__asm__ volatile ("nop");
    }
}

void EXTI0_IRQHandler()
{
    if (EXTI_GetITStatus(EXTI_Line0) != RESET) {
        GPIO_ToggleBits(GPIOC, GPIO_Pin_1);
        EXTI_ClearITPendingBit(EXTI_Line0);
    }
}
