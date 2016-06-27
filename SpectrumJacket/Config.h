#pragma once
#include <stdint-gcc.h>
#include <stm32f10x_gpio.h>

struct Config {
	uint16_t ledsPerStrip;
	GPIO_TypeDef* outputPort;

	GPIO_TypeDef* timingDebugPort;
	bool enableTimingDebugging;

	bool doubleBuffered;
	bool dmaBufferCopy;
};
