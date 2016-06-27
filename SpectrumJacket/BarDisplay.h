#pragma once

#include "WS2812.h"

class BarDisplay {
protected:
	WS2812* ws2812;
	uint8_t width; 
	uint8_t height;

public:
	BarDisplay(WS2812* ws2812, uint8_t width, uint8_t height);
	~BarDisplay();

	void setPixel(uint8_t bar, uint16_t heigth, RGBColor color);
	void setHorizontalLine(uint16_t height, RGBColor color);
	void setVerticalLine(uint8_t bar, uint16_t heigth, RGBColor color);

	void setPixel(uint8_t bar, uint16_t pos, HSLColor color);
};

