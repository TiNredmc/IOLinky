#ifndef ADC_H
#define ADC_H

#include "stm32f042x6.h"

void adc_init();

uint16_t adc_read(uint8_t channel);
int16_t adc_readScheduled(uint8_t channel);

#endif