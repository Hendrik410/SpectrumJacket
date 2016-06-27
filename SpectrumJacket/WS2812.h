#pragma once

#include "colors.h"
#include "Config.h"

class WS2812
{
protected:
	Config* config;

	uint16_t bufferSize;

	uint16_t WS2812_IO_High;
	uint16_t WS2812_IO_Low;

	bool selfAllocated;
	uint8_t* backBuffer;
	uint8_t* frontBuffer;


	void initGPIO();
	void initTimer();
	void initDMA();

public:
	WS2812(Config* config);
	WS2812(Config* config, void* buffer);
	~WS2812();

	Config* getConfig();

	void flush();

	void setLed(uint8_t strip, uint16_t position, uint8_t red, uint8_t green, uint8_t blue);
	void setLed(uint8_t strip, uint16_t position, RGBColor color);
	void setLed(uint8_t strip, uint16_t position, HSLColor color);

	void setLedOnAllStrips(uint16_t position, uint8_t red, uint8_t green, uint8_t blue);
	void setLedOnAllStrips(uint16_t position, RGBColor color);
	void setLedOnAllStrips(uint16_t position, HSLColor color);

	void fillStrip(uint8_t strip, uint8_t red, uint8_t green, uint8_t blue);
	void fillStrip(uint8_t strip, RGBColor color);
	void fillStrip(uint8_t strip, HSLColor color);

	void fillAll(uint8_t red, uint8_t green, uint8_t blue);
	void fillAll(RGBColor color);
	void fillAll(HSLColor color);

	void clearStrip(uint8_t strip);
	void clearAll();

	static volatile bool dmaBusy;
	static WS2812* lastInstance;
};
