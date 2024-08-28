#ifndef ADC_H
#define ADC_H

#include "gd32e23x.h"
#include "gd32e23x_adc.h"
#include "gd32e23x_dma.h"

#include "gpio_pinout.h"

void adc_initScanDMA(
	uint32_t* adc_ptr
	);
void adc_softTrigger();
uint8_t adc_getDataAvaible();

#endif