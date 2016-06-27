#pragma once

#include <stdint-gcc.h>
#include <cmath>

struct RGBColor {
	uint8_t red;
	uint8_t green;
	uint8_t blue;

	RGBColor(uint8_t r, uint8_t g, uint8_t b) : red(r), green(g), blue(b){}
	RGBColor() : red(0), green(0), blue(0) {}
};

struct HSLColor {
	uint16_t hue;
	uint8_t saturation;
	uint8_t lightness;

	HSLColor(uint16_t h, uint8_t s, uint8_t l) : hue(h), saturation(s), lightness(l){}
	HSLColor() : hue(0), saturation(0), lightness(0) {}
};

class ColorUtil {
public:
	static RGBColor HSL2RGB(HSLColor hsv);

	static unsigned int h2rgb(unsigned int v1, unsigned int v2, unsigned int hue);

	
};


//bh