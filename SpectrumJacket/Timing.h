#pragma once
#include <stdint-gcc.h>
#include <stm32f10x_rcc.h>

class Timing{
protected:
	static RCC_ClocksTypeDef clockTypeDef;

public:
	static volatile uint64_t sysTicks;

	static void init();

	static void delay(uint64_t milliseconds);
	static void delayMicroseconds(uint64_t microseconds);

	static long millis();
	static long micros();
};

