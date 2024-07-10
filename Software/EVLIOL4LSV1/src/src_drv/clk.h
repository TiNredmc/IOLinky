#ifndef CLK_H
#define CLK_H
#include "stm32g071xx.h"

// Enum for system clock source selection.
enum CLOCK_SRC{
	HS_INT_16M = 0,
	HS_EXT,
	PLL_R,
	LS_INT,
	LS_EXT
};

enum CLOCK_HSI_DIV{
	HSIDIV_1 = 0,
	HSIDIV_2,
	HSIDIV_4,
	HSIDIV_8,
	HSIDIV_16,
	HSIDIV_32,
	HSIDIV_64,
	HSIDIV_128
};

typedef struct pll_config{
	uint8_t pll_src;
	uint8_t pll_div_m;
	uint8_t pll_mul_n;
	uint8_t pll_div_p;
	uint8_t pll_div_q;
	uint8_t pll_div_r;
} clk_pll_config_t;

void clk_setHsiDiv(uint8_t div);
uint8_t clk_hseDisableBypass();
uint8_t clk_hseEnableBypass();
uint8_t clk_disableClkSrc(uint8_t clk_source);
uint8_t clk_setupSysClkSrc(uint8_t clk_source);
uint8_t clk_getSysClkSrc();
void clk_setSysClkMcoOutput();
void clk_CSSHandler();
void clk_initSysClk();


#endif