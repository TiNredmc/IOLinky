#include "Clk.h"

uint8_t pll_conf_status	= 0;
uint8_t clk_prePllSetup(clk_pll_config_t *pll_conf){
	// Sanity check first 
	
	// Only HSI and HSE are allowed to be set as PLL clock sources
	if((pll_conf->pll_src != HS_INT_16M) && (pll_conf->pll_src != HS_EXT))
		return 1;
	
	// Check the Divisor M
	if((pll_conf->pll_div_m == 0) || (pll_conf->pll_div_m > 8))
		return 2;
	
	// Check the Multiplier N
	if((pll_conf->pll_mul_n < 8) || (pll_conf->pll_mul_n > 86))
		return 3;
	
	// Check the Divisor P 
	if((pll_conf->pll_div_p == 1) || (pll_conf->pll_div_p > 32))
		return 4;

	// Check the Divisor Q
	if((pll_conf->pll_div_q == 1) || (pll_conf->pll_div_q > 8))
		return 5;
	
	// Check the Divisor R
	if((pll_conf->pll_div_r == 1) || (pll_conf->pll_div_r > 8))
		return 6;
	
	// If pll was already enable, disable it first.
	if(RCC->CR & (1 << RCC_CR_PLLON_Pos)){
		// Turn PLL off before configuration
		RCC->CR &= ~(1 << RCC_CR_PLLON_Pos);
		// wait until pll is off
		while((RCC->CR & (1 << RCC_CR_PLLRDY_Pos)));
	}
	
	// Select PLL clock source
	RCC->PLLCFGR &= ~(3 << RCC_PLLCFGR_PLLSRC_Pos);
	RCC->PLLCFGR |= ((pll_conf->pll_src+2) << RCC_PLLCFGR_PLLSRC_Pos); 
	
	// Config the PLL loop factor (M and N).
	RCC->PLLCFGR &= ~((7 << RCC_PLLCFGR_PLLM_Pos) | (0x7F << RCC_PLLCFGR_PLLN_Pos));
	RCC->PLLCFGR |= 
		((pll_conf->pll_div_m-1) << RCC_PLLCFGR_PLLM_Pos) |		// M
		(pll_conf->pll_mul_n << RCC_PLLCFGR_PLLN_Pos);				// N
	
	// Config each P Q and R output channels
	if(pll_conf->pll_div_p > 1){
		RCC->PLLCFGR &= ~((1 << RCC_PLLCFGR_PLLPEN_Pos) | (0x1F << RCC_PLLCFGR_PLLP_Pos));
		RCC->PLLCFGR |= (1 << RCC_PLLCFGR_PLLPEN_Pos) | ((pll_conf->pll_div_p-1) << RCC_PLLCFGR_PLLP_Pos);
	}
	
	if(pll_conf->pll_div_q > 1){
		RCC->PLLCFGR &= ~((1 << RCC_PLLCFGR_PLLQEN_Pos) | (7 << RCC_PLLCFGR_PLLQ_Pos));
		RCC->PLLCFGR |= (1 << RCC_PLLCFGR_PLLQEN_Pos) | ((pll_conf->pll_div_q-1) << RCC_PLLCFGR_PLLQ_Pos);
	}
	
	if(pll_conf->pll_div_r > 1){
		RCC->PLLCFGR &= ~((1 << RCC_PLLCFGR_PLLREN_Pos) | (7 << RCC_PLLCFGR_PLLR_Pos));
		RCC->PLLCFGR |= (1 << RCC_PLLCFGR_PLLREN_Pos) | ((pll_conf->pll_div_r-1) << RCC_PLLCFGR_PLLR_Pos);
	}
	
	
	// turn PLL on
	RCC->CR |= (1 << RCC_CR_PLLON_Pos);
	// Wait until PLL is locked
	while(!(RCC->CR & (1 << RCC_CR_PLLRDY_Pos)));
	
	return 0;
}

void clk_setHsiDiv(uint8_t div){
	uint32_t temp_reg;
	if(div > HSIDIV_128)
		return;
	
	temp_reg = RCC->CR;
	temp_reg &= ~(HSIDIV_128 << RCC_CR_HSIDIV_Pos);
	temp_reg |= (div << RCC_CR_HSIDIV_Pos);
	RCC->CR = temp_reg;

}

// Disable HSE Bypass
uint8_t clk_hseDisableBypass(){
	if(RCC->CR & (1 << RCC_CR_HSEON_Pos))// Return if the HSE is not yet turned off.
		return 1;
	
	RCC->CR &= ~(1 << RCC_CR_HSEBYP_Pos);
	return 0;
}

// Enable HSE bypass
uint8_t clk_hseEnableBypass(){
	if(RCC->CR & (1 << RCC_CR_HSEON_Pos))// Return if the HSE was already on.
		return 1;
	if(RCC->CR & (1 << RCC_CR_HSEBYP_Pos))// Return if the HSE bypass was already enabled.
		return 2;
	
	RCC->CR |= ~(1 << RCC_CR_HSEBYP_Pos);	
	return 0;
}

uint8_t clk_disableClkSrc(uint8_t clk_source){

	switch(clk_source){
		case HS_INT_16M:// For the sake of safety, I won't let you disable the LAST RESORT clock source.
		{
			return 1;
		}
		break;
	
		case HS_EXT:
		{
			RCC->CR &= ~(1 << RCC_CR_HSEON_Pos);
				
		}
		break;
		
		case PLL_R:// TODO :
		{
		
		}
		break;
		
		case LS_INT:
		{
			RCC->CSR &= ~(1 << RCC_CSR_LSION_Pos);
		}
		break;
		
		case LS_EXT:
		{
			RCC->BDCR &= ~(1 << RCC_BDCR_LSCOEN_Pos);
		}
		break;
		
		default:
			return 1;
	}

	return 0;
}

uint8_t clk_setupSysClkSrc(uint8_t clk_source){
	uint32_t temp_reg;
	// Stage 0 : Sanity check.
	if(clk_source > LS_EXT)// We don't accept other number outside the CLOCK_SRC enum!
		return 0;
	
	__disable_irq();
	
	// Stage 1 : Don't re-eanble the already enabled clock.
//	if((uint8_t)((RCC->CFGR & (uint32_t)0x38) >> 3) == clk_source)// Prevent from re-enabling clock source.
//		goto select_clk;
	
	// Stage 1.5 : Enable Clock Secuirty System to switch from HSE-based clock source to HSI when HSE failed.
	RCC->CR |= (1 << RCC_CR_CSSON_Pos);
	
		// Stage 2 : Enable the request clock (if it wasn't).
		
	switch(clk_source){
		case HS_INT_16M:
		{
			RCC->CR |= (1 << RCC_CR_HSION_Pos);// Enable HSI16M.
			while(!(RCC->CR & (1 << RCC_CR_HSIRDY_Pos)));// Wait HSI16M until stable.
		}
		break;
	
		case HS_EXT:
		{
			RCC->CR |= (1 << RCC_CR_HSEON_Pos);
			while(!(RCC->CR & (1 << RCC_CR_HSERDY_Pos)));// Wait HSE until stable.
		}
		break;
		
		case PLL_R:
		{
			// 64Mhz from HSI_16M
			clk_pll_config_t pll_profile;
			pll_profile.pll_src		= HS_EXT;// 8MHz Xtal
			pll_profile.pll_div_m = 1;
			pll_profile.pll_mul_n = 64;
			pll_profile.pll_div_p = 4;
			pll_profile.pll_div_q = 4;
			pll_profile.pll_div_r = 8;// <- Silicon lottery, I reached > 100MHz!!
			
			// From RM0444 Page 171 
			// 5.2.8 Clock source frequency versus voltage scaling
			
			// Switch voltage scaling from mode 2 to mode 1 (High speed CPU mode).
			uint32_t temp_reg = PWR->CR1;
			temp_reg &= ~(3 << PWR_CR1_VOS_Pos);
			temp_reg |= (1 << PWR_CR1_VOS_Pos);
			PWR->CR1 = temp_reg;
			// Wait until the switch was completed.
			while((PWR->SR2 & (1 << PWR_SR2_VOSF_Pos)));
			
			
			// Update flash latency to the minimum.
			temp_reg = FLASH->ACR;
			temp_reg &= ~(7 << FLASH_ACR_LATENCY_Pos);
			temp_reg |= (2 << FLASH_ACR_LATENCY_Pos);
			FLASH->ACR = temp_reg;
			
			while(!(FLASH->ACR & (2 << FLASH_ACR_LATENCY_Pos)));
			
			clk_prePllSetup(&pll_profile);
		}
		break;
		
		case LS_INT:
		{
			RCC->CSR |= (1 << RCC_CSR_LSION_Pos);
			while(!(RCC->CSR & (1 << RCC_CSR_LSIRDY_Pos)));// Wait until the LSI is stable.
		}
		break;
		
		case LS_EXT:
		{
			RCC->BDCR |= (1 << RCC_BDCR_LSCOEN_Pos);
			while(!(RCC->BDCR & (1 << RCC_BDCR_LSERDY_Pos)));// Wait until the LSE is stable.
		}
		break;
		
		default:
			return 1;
	}
		
	// Stage 3 : Select the requested clock source as system clock source. 
select_clk:
	temp_reg = RCC->CFGR;// Copy register content to temporary.
	temp_reg &= ~((uint32_t)0x07);// Clear the system clock selection bit.
	temp_reg |= clk_source;// Select the requested clock source as system clock source.
	RCC->CFGR = temp_reg;
	
	while(!(RCC->CFGR & (clk_source << 3)));// Wait until the systemn clock is switched.

	__enable_irq();
	return 0;
}

// Get current System Clock source
uint8_t clk_getSysClkSrc(){
	return (uint8_t)((RCC->CFGR & (uint32_t)0x38) >> 3);
}

void clk_setSysClkMcoOutput(){	
	RCC->CFGR &= ~(0xFF << 24);// Reset to default 
	RCC->CFGR |= (1 << 24);
	// Select SYSCLK as clock output source of MCO.
}

// Clock security system interrupt Handler
__attribute__((always_inline)) void clk_CSSHandler(){
	if(RCC->CIFR & (1 << RCC_CIFR_CSSF_Pos)){
		RCC->CICR |= (1 << RCC_CICR_CSSC_Pos);// Clear the panic flag
		// TODO : Raise the flag ? Send some fault signal to main code.
		
	}

}

// Initialize system clock
void clk_initSysClk(){
	clk_setHsiDiv(HSIDIV_1);
	// Enable 8Mhz HSE 
	RCC->CR |= (1 << RCC_CR_HSEON_Pos);
	while(!(RCC->CR & (1 << RCC_CR_HSERDY_Pos)));// Wait HSE until stable.
	
	// PLL at 64Mhz 
	clk_setupSysClkSrc(PLL_R);
}
