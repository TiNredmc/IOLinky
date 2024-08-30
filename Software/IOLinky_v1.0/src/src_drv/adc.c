#include "adc.h"

// Private pointers
uint32_t *adc_buffer_ptr;

// Private variables
volatile uint32_t dma_stat = 0;
volatile uint8_t adc_data_available = 0;

uint32_t ADC_startup_delay = 0;

void DMA_Channel0_IRQHandler(void){
	dma_stat = DMA_INTF;
	
	// Check for Global Interrupt 
	// and full xfer on CH0
	if(dma_stat & 3){
		ADC_STAT &= ~(1 << 1);// Clear ADC EOC flag
		
		adc_data_available = 1;
	}
	
	DMA_INTC = 1;// Clear All CH0 flags
}

void adc_initScanDMA(
	uint32_t* adc_ptr
	){
		
	if(adc_ptr == 0)
			return;
	
	ADC_startup_delay = 0x1000000;
	
	adc_buffer_ptr = adc_ptr;
		
	RCU_AHBEN |= 1; // Enable DMA Clock
	RCU_APB2EN |= (1 << 9);// Enable ADC Clock
	
	ADC_CTL0 |= (1 << 8);// ADC Scan mode
	
	ADC_RSQ0 = ((3-1) << 20);// Total scan of 3 channels
	
	// Scan sequence
	ADC_RSQ2 =
		(Isense_pin 	<< 0)		|
		(V5sense_pin 	<< 5)		|
		(V24sense_pin << 10)	;
	
	// Set sample cycle
	ADC_SAMPT1 =
		(7 << (Isense_pin 	* 3))	|
		(7 << (V5sense_pin 	* 3))	|
		(7 << (V24sense_pin	* 3))	;
	
	ADC_CTL1 |=
		(1 << 20)		| // Enable external trigger
		(7 << 17)		| // use software trigger (for now)
		(1 << 8)		; // Enable DMA request
		
	ADC_CTL1 |= 1;// Enable ADC
	while(ADC_startup_delay--);
	
	ADC_CTL1 |= (1 << 3);// Reset calibration
	while(ADC_CTL1 & (1 << 3));
	
	ADC_CTL1 |= (1 << 2);// Start Cal routine
	while(ADC_CTL1 & (1 << 2));

	// DMA Config
	
	// Config Peripheral address
	DMA_CH0PADDR = (uint32_t)(&ADC_RDATA);
	
	// Config Memory address (point to adc data holding struct)
	DMA_CH0MADDR = (uint32_t)((uint32_t *)adc_buffer_ptr);
	
	// Data count is 3
	DMA_CH0CNT = 3;
	
	DMA_CH0CTL |=
		(3 << 12)			| // "Ultra high" priority
		(1 << 10)			| // 16 bit data buffer width
		(1 << 8)			| // 16 bit peripheral data width
		(1 << 7)			| // Memory incremental mode
		(1 << 5)			| // Circular mode
		(1 << 1)			; // Channel full xfer interrupt 
		
	DMA_CH0CTL |= 1;// Enable DMA CH0

	NVIC_SetPriority(DMA_Channel0_IRQn, 1);
	NVIC_EnableIRQ(DMA_Channel0_IRQn);
}

void adc_softTrigger(){
	ADC_CTL1 |= (1 << 22);
}


uint8_t adc_getDataAvaible(){
	if(adc_data_available == 0)
		return 0;
	
	adc_data_available = 0;
	
	return 1;
}