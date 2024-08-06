#include "Systick_delay.h"

volatile uint32_t millis_counter;

void SysTick_Handler(void){
	millis_counter++;

}

void systick_init(const uint32_t f_cpu, const uint32_t Hz){
	SysTick->LOAD = (f_cpu / Hz) - 1;
	SysTick->VAL	=	0;
	SysTick->CTRL |= 
			(1 << SysTick_CTRL_CLKSOURCE_Pos) |
			(1 << SysTick_CTRL_TICKINT_Pos);
	SysTick->CTRL |= (1 << SysTick_CTRL_ENABLE_Pos);
	NVIC_SetPriority(SysTick_IRQn, 0);
	NVIC_ClearPendingIRQ(SysTick_IRQn);
	NVIC_EnableIRQ(SysTick_IRQn);
}
	
uint32_t millis(){
	return millis_counter;
}

void delay_ms(uint32_t ms){
	static uint32_t  elapsed;
	if(ms == 0)
		return;
	
	elapsed = millis();
	while((millis()) - elapsed < ms);
}