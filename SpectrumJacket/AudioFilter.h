#pragma once

#include <stdint-gcc.h>
#include <stm32f10x_adc.h>
#include <stm32f10x_dma.h>
#include <stm32f10x_gpio.h>
#include <misc.h>

#include "Util.h"
#include "Timing.h"
#include <cstddef>

class AudioFilter {
protected:


	void initPeriph();
	void readValues();

	uint16_t readAnalog(uint8_t channel);

	void setRST(uint8_t val);
	void setSTROBE(uint8_t val);

	uint16_t get(uint16_t* buffer, float normFreq); // normFreq: [0, 1]
public:
	AudioFilter();
	~AudioFilter();

	void trigger();


	uint16_t leftBuffer[7];
	uint16_t rightBuffer[7];

	uint16_t getLeft(float normFreq); // normFreq: [0, 1]
	uint16_t getRight(float normFreq); // normFreq: [0, 1]

	static AudioFilter* lastInstance;
};

