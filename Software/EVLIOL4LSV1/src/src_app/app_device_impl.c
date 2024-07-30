#include "app_device_impl.h"

// Private variables
uint32_t led_millis = 0;
uint32_t PD_refresh_millis = 0;
uint8_t alive_fsm = 0;
uint8_t app_fsm = 0;

// Setup all required GPIOs.
void app_initGPIO(){
	RCC->IOPENR = (uint32_t)(0x3F);	// Enable all GPIO clock
	
	// Init SPI pins
	GPIOA->MODER &= 
		~(
		(3 << CTLD_pin * 2) |
		(3 << SPI1_SCK * 2) | 
		(3 << SPI1_MOSI * 2) |
		(3 << SPI1_SS	* 2)	|
		(3 << SPI1_INT * 2) |
		(3 << SPI1_MISO * 2)
		);

	GPIOA->MODER |= 
		(1 << CTLD_pin * 2) |		// Output
		(2 << SPI1_SCK * 2) | 	// Alternative
		(2 << SPI1_MOSI * 2) |	// Alternative
		(1 << SPI1_SS	* 2)	|		// Output
		(2 << SPI1_INT * 2) |		// Alternative
		(2 << SPI1_MISO * 2);		// Alternative
	
	GPIOA->OSPEEDR |= 
		(3 << CTLD_pin * 2) |		// High speed
		(3 << SPI1_SCK * 2) | 	// High speed
		(3 << SPI1_MOSI * 2) |	// High speed
		(3 << SPI1_SS	* 2)	|		// High speed
		(3 << SPI1_INT * 2) |		// High speed
		(3 << SPI1_MISO * 2);		// High speed
	
	GPIOA->AFR[0] = 0;
	GPIOA->AFR[1] = 0;

	GPIOA->ODR |= (1 << SPI1_SS);// De-select the L6364.
	
	// No need to config GPIOA->AFR
	// as it is already the AF0 (from the reset value)
	
	// Init IPS4260 sink-driver pins
	GPIOB->MODER &=
		~(
		(3 << IOL_mon * 2)			|
		(3 << OL_pin * 2) |			// Input
		(3 << FLT_pin * 2)			// Input
		);
		
	GPIOB->MODER |= (1 << IOL_mon * 2);
	GPIOB->OSPEEDR |= (3 << IOL_mon *2); 
		
	GPIOD->MODER &=
		~(
		(3 << OUT1_pin * 2)	|
		(3 << OUT2_pin * 2)	|
		(3 << OUT3_pin * 2) |
		(3 << OUT4_pin * 2)
		);
	
	GPIOD->MODER |= 
		(1 << OUT1_pin * 2)	|		// Output
		(1 << OUT2_pin * 2)	|		// Output
		(1 << OUT3_pin * 2) |		// Output
		(1 << OUT4_pin * 2);		// Output
}

// Init IO system
void app_initIO(){
	clk_initSysClk();								// Setup system clock
	systick_init(F_CPU, 1000);			// Setup 1000 systick
	app_initGPIO();
	spi_init();
	
}

// Task to serve alive LED status.
// Warning, the green LED is so bright
// So freaking bright it is k*lling me (ToT).
void app_iol_aliveTask(){
	
	if((millis() - led_millis) > 100){
		led_millis = millis();
		
		switch(alive_fsm){
			case 0:// Status : IO-Link inactive
				{
					iol_pl_standbyLED();
					if(iol_dl_getModeStatus() == DL_MODE_OP){
						l6364_setLED2(0x00);// Red LED off
						alive_fsm = 1;
					}
				}
				break;
				
			case 1:// Status : IO-Link active
				{
					iol_pl_connectedLED();
					if(iol_dl_getModeStatus() != DL_MODE_OP){
						l6364_setLED1(0x00);// Green LED off
						alive_fsm = 0;
					}
				}
				break;
		}
		
	}
}

void app_iol_updatePDTask(){
	if((millis() - PD_refresh_millis) > 100){
		PD_refresh_millis = millis();
		if(iol_dl_getModeStatus() == DL_MODE_OP)
			iol_al_updatePD();// Update PD
	}

}

// Tasks and FSMs that runs this IO-Link device
void app_runner(){
	switch(app_fsm){
		case APP_INIT_STATE:
		{
			iol_al_init();
			app_fsm = APP_RUN_STATE;
		}
		break;
		
		case APP_RUN_STATE:// Check wake up current
		{
			iol_al_poll();
			app_iol_updatePDTask();
			app_iol_aliveTask();
		}
		
		default:
			break;
	
	}
}