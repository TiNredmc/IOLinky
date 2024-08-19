#include "adc.h"

void adc_init(){
	RCC->APB2ENR |= RCC_APB2ENR_ADCEN;
	
	if ((ADC1->CR & ADC_CR_ADEN) != 0) 
		ADC1->CR |= ADC_CR_ADDIS;

	// Start calibration routine
	ADC1->CR |= ADC_CR_ADCAL;
	
	// wait until calibration is completed
	while((ADC1->CR & ADC_CR_ADCAL) != 0);

	ADC->CCR |= ADC_CCR_VREFEN;
	
	ADC1->CR |= ADC_CR_ADEN;
	
	while(!(ADC1->ISR & ADC_ISR_ADRDY));
}

uint16_t adc_read(uint8_t channel){
	// Sanity check, make sure we don't select the out of rage channel.
	if(channel > 19)
		return 0;
	
	ADC1->CR &= ~(1 << ADC_CR_ADSTART_Pos);// Stop ADC conversion
	
	ADC1->CHSELR = (1 << channel);// Select the Reading channel
	
	ADC1->CR |= (1 << ADC_CR_ADSTART_Pos);// Start the ADC conversion
	while(!(ADC1->ISR & (1 << ADC_ISR_EOS_Pos)));// Wait until conversion is completed
	
	return ADC1->DR;// Return result
}

uint8_t adc_fsm = 0;

int16_t adc_readScheduled(uint8_t channel){

	switch(adc_fsm){
		case 0:// Init
		{
			if(channel > 19)
				return -1;
	
			ADC1->CHSELR = (1 << channel);// Select the Reading channel
		
			ADC1->CR |= (1 << ADC_CR_ADSTART_Pos);// Start the ADC conversion
			
			adc_fsm = 1;
		}
		
		break;
		
		case 1:
		{
			if(ADC1->ISR & (1 << ADC_ISR_EOS_Pos)){// Wait until conversion is completed
				adc_fsm = 0;
				return ADC1->DR;
			}
		}
		break;
	
	}
	
	return -1;
}