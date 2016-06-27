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

int main() {
	Timing::init();

	Config config;
	config.doubleBuffered = false;
	config.dmaBufferCopy = false;
	config.enableTimingDebugging = false;
	config.ledsPerStrip = 22 * 25;
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
		for(uint8_t freg = 0; freg < 11; freg++) {
			uint8_t leftAmplitude = (audio->leftBuffer[freg / 2] * 22) / 4096;
			uint8_t rightAmplitude = (audio->rightBuffer[freg / 2] * 22) / 4096;

			for (uint16_t height = 0; height < leftAmplitude; height++) {
				color.lightness = 25; // (height / 22.0f) * 32 + 32;
				display->setPixel(freg, height, color);
			}
			for (uint16_t height = 0; height < rightAmplitude; height++) {
				color.lightness = 25; // (height / 22.0f) * 32 + 32;
				display->setPixel(24 - freg, height, color);
			}
		}
		ws2812->flush();

		Timing::delay(5);
	}
}

