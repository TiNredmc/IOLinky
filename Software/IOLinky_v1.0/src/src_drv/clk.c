#include "Clk.h"

void clk_enableHXTAL(){
	RCU_CTL0 |= (1 << 16);
	while(!(RCU_CTL0 & (1 << 17)));
}

// Initialize system clock
void clk_initSysClk(){
	// Using internal IRC8M through PLL to get 72MHz
	// 8MHz * 1/2 * PLLMF
	// 4MHz * 18 -> 72MHz
	// PLLMF multiply factor is 18 -> 0x11 in register value 
	
	RCU_CFG0 |= 
		(0x11 << 18);// Set PLLMF
	RCU_CTL0 |= (1 << 24);// Enable PLL
	// Wait for PLL stabilized
	while(!(RCU_CTL0 & (1 << 25)));
	
	// Switch System clock source to PLL
	RCU_CFG0 |= 2;
	while(!(RCU_CFG0 & (2 << 2)));
	
	// Enable ADC Clock
	RCU_CTL1 |= 1;
	while(!(RCU_CTL1 & (1 << 1)));
}
