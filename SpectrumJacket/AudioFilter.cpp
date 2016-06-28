#include "AudioFilter.h"

AudioFilter* AudioFilter::lastInstance = NULL;

AudioFilter::AudioFilter() {
	lastInstance = this;

	initPeriph();
}


AudioFilter::~AudioFilter(){
}

void AudioFilter::initPeriph() {

	RCC_ADCCLKConfig(RCC_PCLK2_Div6);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO | RCC_APB2Periph_ADC1, ENABLE);


	/* GPIO Init */
	GPIO_InitTypeDef gpio_init;
	gpio_init.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
	gpio_init.GPIO_Mode = GPIO_Mode_AIN;
	gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &gpio_init);

	gpio_init.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
	gpio_init.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &gpio_init);

	/* ADC Init */
	ADC_InitTypeDef adcInit;
	adcInit.ADC_ContinuousConvMode = DISABLE;
	adcInit.ADC_DataAlign = ADC_DataAlign_Right;
	adcInit.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	adcInit.ADC_Mode = ADC_Mode_Independent;
	adcInit.ADC_NbrOfChannel = 2;
	adcInit.ADC_ScanConvMode = DISABLE;
	ADC_Init(ADC1, &adcInit);

	ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 1, ADC_SampleTime_1Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_9, 2, ADC_SampleTime_1Cycles5);

	ADC_Cmd(ADC1, ENABLE);

	/* ADC CALIBRATION */
	ADC_ResetCalibration(ADC1);
	while (ADC_GetResetCalibrationStatus(ADC1));
	ADC_StartCalibration(ADC1);
	while (ADC_GetCalibrationStatus(ADC1));

	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

uint16_t AudioFilter::readAnalog(uint8_t channel) {
	ADC_RegularChannelConfig(ADC1, channel, 1, ADC_SampleTime_1Cycles5);
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
	while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
	return ADC_GetConversionValue(ADC1);
}


void AudioFilter::trigger() {
	readValues();
}

void AudioFilter::readValues() {
	setRST(1);
	setRST(0);

	for (uint8_t i = 0; i < 7; i++) {
		setSTROBE(0);
		Timing::delayMicroseconds(65);

		leftBuffer[i] = readAnalog(ADC_Channel_9);
		rightBuffer[i] = readAnalog(ADC_Channel_8);
		setSTROBE(1);
	}
}

uint16_t AudioFilter::get(uint16_t* buffer, float normFreq) {
	int16_t freq = (int16_t)(normFreq * 6);
	float frac = (normFreq * 6) - freq;

	uint16_t a = buffer[freq];
	if (freq >= 6) 
		return a;

	uint16_t b = buffer[freq + 1];
	return a + (b - a) * frac; // linear interpolation
}

uint16_t AudioFilter::getLeft(float normFreq) {
	return get(leftBuffer, normFreq);
}

uint16_t AudioFilter::getRight(float normFreq) {
	return get(rightBuffer, normFreq);
}

void AudioFilter::setRST(uint8_t val) {
	GPIO_WriteBit(GPIOB, GPIO_Pin_8, static_cast<BitAction>(val));
}

void AudioFilter::setSTROBE(uint8_t val) {
	GPIO_WriteBit(GPIOB, GPIO_Pin_9, static_cast<BitAction>(val));
}
