#include <stdint-gcc.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_rcc.h>

#include "Timing.h"
#include "Config.h"
#include "WS2812.h"
#include "Util.h"
#include "AudioFilter.h"
#include "BarDisplay.h"

//uint8_t buffer[22 * 24];

#define STRIPES 25
#define LEDS_PER_STRIPE 22
#define NORM_AMP(x) ((x * LEDS_PER_STRIPE) / 4096)

int main() {
	Timing::init();

	Config config;
	config.doubleBuffered = false;
	config.dmaBufferCopy = false;
	config.enableTimingDebugging = false;
	config.ledsPerStrip = STRIPES * LEDS_PER_STRIPE;
	config.outputPort = GPIOA;
	
	WS2812* ws2812 = new WS2812(&config);
	BarDisplay* display = new BarDisplay(ws2812, 25, 22);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	GPIO_InitTypeDef gpio;
	gpio.GPIO_Mode = GPIO_Mode_Out_OD;
	gpio.GPIO_Pin = GPIO_Pin_13;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &gpio);

	AudioFilter* audio = new AudioFilter();

	HSLColor color;
	color.saturation = 255;

	for (;;) {
		audio->trigger();

		color.hue = (Timing::millis() / 200) % 360;


		ws2812->clearAll();

#define COLOR_LIGHT(x) ((1 - (x / (float)LEDS_PER_STRIPE)) * 32 + 32)

		for(uint8_t freg = 0; freg <= STRIPES / 2; freg++) {
			float normFreq = (float)freq / (STRIPES / 2);

			uint8_t left = NORM_AMP(audio->getLeft(normFreq));
			uint8_t right = NORM_AMP(audio->getRight(normFreq));

			for (uint16_t i = 0; i < left; i++) {
				color.lightness = COLOR_LIGHT(i);
				display->setPixel(freg, i, color);
			}
			for (uint16_t i = 0; i < right; i++) {
				color.lightness = COLOR_LIGHT(i);
				display->setPixel(STRIPES - 1 - freg, i, color);
			}
		}
		if (STRIPES % 2 != 0) {
			uint8_t amplitude = (NORM_AMP(audio->getLeft(1)) + NORM_AMP(audio->getRight(0))) / 2;

			for (uint16_t i = 0; i < amplitude; i++) {
				color.lightness = COLOR_LIGHT(i);
				display->setPixel(STRIPES / 2 + 1, i, color);
			}
		}

		ws2812->flush();

		Timing::delay(5);
	}
}

