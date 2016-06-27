#include "Util.h"

void Util::initGPIOClock(GPIO_TypeDef* gpio_type_def) {
	if(gpio_type_def == GPIOA) {
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	} else if (gpio_type_def == GPIOB) {
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	} else if(gpio_type_def == GPIOC) {
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	} else if(gpio_type_def == GPIOD) {
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
	} else if(gpio_type_def == GPIOE) {
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);
	} else if(gpio_type_def == GPIOF) {
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF, ENABLE);
	} 
}
