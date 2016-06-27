#include "BarDisplay.h"



BarDisplay::BarDisplay(WS2812* ws2812, uint8_t width, uint8_t height) {
	this->ws2812 = ws2812;
	this->height = height;
	this->width = width;
}


BarDisplay::~BarDisplay() {
}

void BarDisplay::setPixel(uint8_t bar, uint16_t pos, RGBColor color) {
	uint16_t pixelPos = bar * this->height + pos;
	ws2812->setLed(0, pixelPos, color);
}

void BarDisplay::setPixel(uint8_t bar, uint16_t pos, HSLColor color) {
	setPixel(bar, pos, ColorUtil::HSL2RGB(color));
}

void BarDisplay::setHorizontalLine(uint16_t height, RGBColor color) {
	for(uint8_t i = 0; i < this->width; i++) {
		setPixel(i, height, color);
	}
}

void BarDisplay::setVerticalLine(uint8_t bar, uint16_t height, RGBColor color) {
	for(uint16_t i = 0; i < height; i++) {
		setPixel(bar, i, color);
	}
}
