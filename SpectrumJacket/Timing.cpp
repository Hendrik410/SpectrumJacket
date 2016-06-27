#include "Timing.h"

RCC_ClocksTypeDef Timing::clockTypeDef;
volatile uint64_t Timing::sysTicks = 0;

void Timing::init() {
	RCC_GetClocksFreq(&clockTypeDef);
	SysTick_Config(clockTypeDef.HCLK_Frequency / 1000000 - 1); //1Mhz => 1 µs
}

void Timing::delayMicroseconds(uint64_t microseconds) {
	uint64_t start = sysTicks;
	while (sysTicks - start <= microseconds) ;
}

void Timing::delay(uint64_t milliseconds) {
	delayMicroseconds(milliseconds * 1000);
}

long Timing::micros() {
	return sysTicks;
}

long Timing::millis() {
	return sysTicks / 1000;
}

extern "C" void SysTick_Handler(void) {
	Timing::sysTicks++;
}
