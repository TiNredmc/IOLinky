#include "Clk.h"

// Initialize system clock
void clk_initSysClk(){
	// Enable 48Mhz HSI
	RCC->CR2 |= 1 << RCC_CR2_HSI48ON_Pos;
	while(!(RCC->CR2 & (1 << RCC_CR2_HSI48RDY_Pos)));// Wait HSI48 until stable.
	
	// Set HSI48 as system clok 

	RCC->CFGR |= 3;
	
	while(!(RCC->CFGR & (3 << 2)));
}
