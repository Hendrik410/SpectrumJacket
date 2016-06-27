#pragma once

#include <stdint-gcc.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>

class Util
{
public:
	static void initGPIOClock(GPIO_TypeDef* gpio_type_def);
};

