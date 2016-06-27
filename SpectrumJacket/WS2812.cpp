#include "WS2812.h"
#include <cstring>


#include <stdlib.h>
#include <stm32f10x.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_tim.h>
#include <misc.h>
#include <stm32f10x_dma.h>

#include "colors.h"
#include "Config.h"
#include "Util.h"

volatile bool WS2812::dmaBusy = false;
WS2812* WS2812::lastInstance = NULL;

WS2812::WS2812(Config* config) {
	this->config = config;
	this->bufferSize = config->ledsPerStrip * 24;

	this->selfAllocated = true;
	this->frontBuffer = (uint8_t*)malloc(this->bufferSize);
	memset(this->frontBuffer, 0, this->bufferSize);
	if(this->config->doubleBuffered)
		this->backBuffer = (uint8_t*)malloc(this->bufferSize);

	lastInstance = this;

	WS2812_IO_High = 0xFFFF;
	WS2812_IO_Low = 0;

	initGPIO();
	initTimer();
	initDMA();
}

WS2812::WS2812(Config* config, void* buffer) {
	this->config = config;
	this->bufferSize = config->ledsPerStrip * 24;
	this->config->doubleBuffered = false;

	this->selfAllocated = false;
	this->frontBuffer = (uint8_t*)buffer;

	lastInstance = this;

	WS2812_IO_High = 0xFFFF;
	WS2812_IO_Low = 0;

	initGPIO();
	initTimer();
	initDMA();
}

WS2812::~WS2812() {
	if(this->selfAllocated) {
		free(this->frontBuffer);
		if (config->doubleBuffered)
			free(this->backBuffer);
	}
}


Config* WS2812::getConfig() {
	return config;
}


void WS2812::initGPIO() {
	Util::initGPIOClock(config->outputPort);

	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(config->outputPort, &GPIO_InitStructure);

	config->outputPort->ODR = 0xFFFFFFFF;

	if(config->enableTimingDebugging) {
		Util::initGPIOClock(config->timingDebugPort);
		GPIO_Init(config->timingDebugPort, &GPIO_InitStructure);
		config->timingDebugPort->ODR = 0;
	}
}

void WS2812::initTimer() {
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_OCInitTypeDef TIM_OCInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	uint16_t PrescalerValue;

	// TIM4 Periph clock enable
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

	PrescalerValue = (uint16_t)(SystemCoreClock / 24000000) - 1;

	/* Time base configuration */
	TIM_TimeBaseStructure.TIM_Period = 29; // 800kHz
	TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

	TIM_ARRPreloadConfig(TIM4, DISABLE);

	/* Timing Mode configuration: Channel 2 */
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Timing;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Disable;
	TIM_OCInitStructure.TIM_Pulse = 8;
	TIM_OC2Init(TIM4, &TIM_OCInitStructure);
	TIM_OC2PreloadConfig(TIM4, TIM_OCPreload_Disable);

	/* Timing Mode configuration: Channel 3 */
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Disable;
	TIM_OCInitStructure.TIM_Pulse = 17;
	TIM_OC3Init(TIM4, &TIM_OCInitStructure);
	TIM_OC3PreloadConfig(TIM4, TIM_OCPreload_Disable);

	/* configure TIM4 interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void WS2812::initDMA() {
	DMA_InitTypeDef DMA_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	//RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);


	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&config->outputPort->ODR;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
	DMA_InitStructure.DMA_BufferSize = 0;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	//DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

	// TIM4 Update event
	/* DMA1 Channel7 configuration ----------------------------------------------*/
	DMA_DeInit(DMA1_Channel7);
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&WS2812_IO_High;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
	DMA_Init(DMA1_Channel7, &DMA_InitStructure);

	// TIM4 CC2 event
	/* DMA1 Channel4 configuration ----------------------------------------------*/
	DMA_DeInit(DMA1_Channel4);
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)frontBuffer;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_Init(DMA1_Channel4, &DMA_InitStructure);

	// TIM4 CC3 event
	/* DMA1 Channel5 configuration ----------------------------------------------*/
	DMA_DeInit(DMA1_Channel5);
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&WS2812_IO_Low;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
	DMA_Init(DMA1_Channel5, &DMA_InitStructure);

	if(config->dmaBufferCopy) {
		// Back-to-Frontbuffer copy
		/* DMA2 Channel2 configuration ----------------------------------------------*/
		DMA_DeInit(DMA2_Channel2);
		DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)backBuffer;
		DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)frontBuffer;
		DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
		DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
		DMA_InitStructure.DMA_M2M = DMA_M2M_Enable;
		DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
		DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Enable;
		DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;;
		DMA_Init(DMA2_Channel2, &DMA_InitStructure);

		/* configure DMA2 Channel2 interrupt */
		//NVIC_InitStructure.NVIC_IRQChannel = DMA2_Channel2_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);

		DMA_ITConfig(DMA2_Channel2, DMA_IT_TC, ENABLE);
	}

	/* configure DMA1 Channel5 interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* enable DMA1 Channel7 transfer complete interrupt */
	DMA_ITConfig(DMA1_Channel5, DMA_IT_TC, ENABLE);
}


void WS2812::flush() {
	while (WS2812::dmaBusy) ;

	WS2812::dmaBusy = true;

	if(config->doubleBuffered) {
		if (config->enableTimingDebugging)
			GPIO_WriteBit(config->timingDebugPort, GPIO_Pin_1, Bit_SET);

		if(config->dmaBufferCopy) {
			// clear all relevant DMA flags
			DMA_ClearFlag(DMA2_FLAG_TC2 | DMA2_FLAG_HT2 | DMA2_FLAG_GL2 | DMA2_FLAG_TE2);

			DMA_SetCurrDataCounter(DMA2_Channel2, this->bufferSize / 4);
			DMA_Cmd(DMA2_Channel2, ENABLE);

		} else {
			
			memcpy(this->frontBuffer, this->backBuffer, this->bufferSize);

			if (config->enableTimingDebugging)
				GPIO_WriteBit(config->timingDebugPort, GPIO_Pin_1, Bit_RESET);
		}
	}

	// clear all relevant DMA flags
	DMA_ClearFlag(DMA1_FLAG_TC7 | DMA1_FLAG_HT7 | DMA1_FLAG_GL7 | DMA1_FLAG_TE7);
	DMA_ClearFlag(DMA1_FLAG_TC4 | DMA1_FLAG_HT4 | DMA1_FLAG_GL4 | DMA1_FLAG_TE4);
	DMA_ClearFlag(DMA1_FLAG_HT5 | DMA1_FLAG_GL5 | DMA1_FLAG_TE5);

	// configure the number of bytes to be transferred by the DMA controller
	DMA_SetCurrDataCounter(DMA1_Channel7, this->bufferSize);
	DMA_SetCurrDataCounter(DMA1_Channel4, this->bufferSize);
	DMA_SetCurrDataCounter(DMA1_Channel5, this->bufferSize);

	// clear all TIM4 flags
	TIM4->SR = 0;

	// enable the corresponding DMA channels
	DMA_Cmd(DMA1_Channel7, ENABLE);
	DMA_Cmd(DMA1_Channel4, ENABLE);
	DMA_Cmd(DMA1_Channel5, ENABLE);

	// IMPORTANT: enable the TIM4 DMA requests AFTER enabling the DMA channels!
	TIM_DMACmd(TIM4, TIM_DMA_CC2, ENABLE);
	TIM_DMACmd(TIM4, TIM_DMA_CC3, ENABLE);
	TIM_DMACmd(TIM4, TIM_DMA_Update, ENABLE);

	
	// preload counter with 29 so TIM4 generates UEV directly to start DMA transfer
	TIM_SetCounter(TIM4, 29);


	if (config->enableTimingDebugging)
		GPIO_WriteBit(config->timingDebugPort, GPIO_Pin_0, Bit_SET);

	// start TIM4
	TIM_Cmd(TIM4, ENABLE);
}


void WS2812::setLed(uint8_t strip, uint16_t position, uint8_t red, uint8_t green, uint8_t blue) {
	uint8_t* bufferToWrite = this->frontBuffer;
	if (config->doubleBuffered)
		bufferToWrite = this->backBuffer;

	uint16_t stripMask = 1 << strip;
	uint16_t pixelOffset = position * 24;

	for (uint8_t i = 0; i < 8; i++)
	{
		// clear the data for pixel 
		bufferToWrite[pixelOffset + i] &= ~stripMask;
		bufferToWrite[pixelOffset + 8 + i] &= ~stripMask;
		bufferToWrite[pixelOffset + 16 + i] &= ~stripMask;
		// write new data for pixel
		bufferToWrite[pixelOffset + i] |= (green << i & 0x80) >> 7 << strip;
		bufferToWrite[pixelOffset + 8 + i] |= (red << i & 0x80) >> 7 << strip;
		bufferToWrite[pixelOffset + 16 + i] |= (blue << i & 0x80) >> 7 << strip;
	}
}

void WS2812::setLed(uint8_t strip, uint16_t position, RGBColor color) {
	setLed(strip, position, color.red, color.green, color.blue);
}

void WS2812::setLed(uint8_t strip, uint16_t position, HSLColor color) {
	setLed(strip, position, ColorUtil::HSL2RGB(color));
}


void WS2812::setLedOnAllStrips(uint16_t position, uint8_t red, uint8_t green, uint8_t blue) {
	uint8_t* bufferToWrite = this->frontBuffer;
	if (config->doubleBuffered)
		bufferToWrite = this->backBuffer;

	uint16_t pixelOffset = position * 24;

	for(uint8_t i = 0; i < 8; i++) {
		bufferToWrite[pixelOffset + i] = (green << i & 0x80) >> 7 == 0 ? 0 : 0xFFFF;
		bufferToWrite[pixelOffset + i + 8] = (red << i & 0x80) >> 7 == 0 ? 0 : 0xFFFF;
		bufferToWrite[pixelOffset + i + 16] = (blue << i & 0x80) >> 7 == 0 ? 0 : 0xFFFF;
	}
}

void WS2812::setLedOnAllStrips(uint16_t position, RGBColor color) {
	setLedOnAllStrips(position, color.red, color.green, color.blue);
}

void WS2812::setLedOnAllStrips(uint16_t position, HSLColor color) {
	setLedOnAllStrips(position, ColorUtil::HSL2RGB(color));
}


void WS2812::fillStrip(uint8_t strip, uint8_t red, uint8_t green, uint8_t blue) {
	for(uint16_t i = 0; i < this->config->ledsPerStrip; i++) {
		setLed(strip, i, red, green, blue);
	}
}

void WS2812::fillStrip(uint8_t strip, RGBColor color) {
	fillStrip(strip, color.red, color.green, color.blue);
}

void WS2812::fillStrip(uint8_t strip, HSLColor color) {
	fillStrip(strip, ColorUtil::HSL2RGB(color));
}


void WS2812::fillAll(uint8_t red, uint8_t green, uint8_t blue) {
	for(uint16_t i = 0; i < this->config->ledsPerStrip; i++) {
		setLedOnAllStrips(i, red, green, blue);
	}
}

void WS2812::fillAll(RGBColor color) {
	fillAll(color.red, color.green, color.blue);
}

void WS2812::fillAll(HSLColor color) {
	fillAll(ColorUtil::HSL2RGB(color));
}


void WS2812::clearStrip(uint8_t strip) {
	fillStrip(strip, 0, 0, 0);
}

void WS2812::clearAll() {
	if(this->config->doubleBuffered)
		memset(this->backBuffer, 0, this->bufferSize);

	memset(this->frontBuffer, 0, this->bufferSize);
}



/* DMA1 Channel2 Interrupt Handler gets executed once the complete framebuffer has been transmitted to the LEDs */
extern "C" void DMA1_Channel5_IRQHandler(void) {
	// clear DMA7 transfer complete interrupt flag
	DMA_ClearITPendingBit(DMA1_IT_TC5);
	// enable TIM4 Update interrupt to append 50us dead period
	TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
	// disable the DMA channels
	DMA_Cmd(DMA1_Channel7, DISABLE);
	DMA_Cmd(DMA1_Channel4, DISABLE);
	DMA_Cmd(DMA1_Channel5, DISABLE);
	// IMPORTANT: disable the DMA requests, too!
	TIM_DMACmd(TIM4, TIM_DMA_CC2, DISABLE);
	TIM_DMACmd(TIM4, TIM_DMA_CC3, DISABLE);
	TIM_DMACmd(TIM4, TIM_DMA_Update, DISABLE);

}

/* TIM4 Interrupt Handler gets executed on every TIM4 Update if enabled */
extern "C" void TIM4_IRQHandler(void) {
	static uint16_t overflowCount = 0;
	uint16_t maxOverflowCount = 19;

	// Clear TIM4 Interrupt Flag
	TIM_ClearITPendingBit(TIM4, TIM_IT_Update);

	/* check if certain number of overflows has occured yet
	* this ISR is used to guarantee a 50us dead time on the data lines
	* before another frame is transmitted */
	if (overflowCount < maxOverflowCount)
	{
		// count the number of occured overflows
		overflowCount++;
	}
	else
	{
		// clear the number of overflows
		overflowCount = 0;
		// stop TIM4 now because dead period has been reached
		TIM_Cmd(TIM4, DISABLE);
		/* disable the TIM4 Update interrupt again
		* so it doesn't occur while transmitting data */
		TIM_ITConfig(TIM4, TIM_IT_Update, DISABLE);
		// finally indicate that the data frame has been transmitted
		WS2812::dmaBusy = false;

		if(WS2812::lastInstance != NULL) {
			Config* config = WS2812::lastInstance->getConfig();

			if (config->enableTimingDebugging)
				GPIO_WriteBit(config->timingDebugPort, GPIO_Pin_0, Bit_RESET);
		}
	}
}

/* DMA2 Channel2 Interrupt Handler get executed after the copying of the back to the front buffer is complete */
extern "C" void DMA2_Channel2_IRQHandler(void) {
	// clear DMA1_Channel7 transfer complete interrupt flag
	DMA_ClearITPendingBit(DMA2_IT_TC2);

	DMA_Cmd(DMA2_Channel2, DISABLE);

	if (WS2812::lastInstance != NULL) {
		Config* config = WS2812::lastInstance->getConfig();

		if (config->enableTimingDebugging)
			GPIO_WriteBit(config->timingDebugPort, GPIO_Pin_1, Bit_RESET);
	}
}