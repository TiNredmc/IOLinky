#include "app_device_impl.h"

// Private variables
uint32_t led_millis = 0;
uint32_t PD_refresh_millis = 0;
uint8_t alive_fsm = 0;
uint8_t app_fsm = 0;

// Setup all required GPIOs.
void app_initGPIO(){
	
	// Enable GPIO clock
	RCC->AHBENR |= 
		RCC_AHBENR_GPIOAEN	|
		RCC_AHBENR_GPIOBEN;
	
	// Init SPI pins
	GPIOA->MODER &= 
		~(
		(3 << (USART2_TX * 2)) 	|
		(3 << (USART2_RX * 2)) 	| 
		(3 << (OL_pin * 2)) 		|
		(3 << (EN_pin	* 2))
		);

	GPIOA->MODER |= 
		(0 << (OL_pin * 2)) 		| // Input 
		(2 << (USART2_TX * 2)) 	| // AF1
		(2 << (USART2_RX * 2)) 	| // AF1
		(1 << (EN_pin	* 2))			| // Output OD Pull-up
		(3 << (Isense_pin * 2))	|	// Analog input
		(3 << (V5sense_pin * 2))|	// Analog input
		(3 << (V24sense_pin * 2)); // Analog input
	
//	GPIOA->OTYPER |= 
//		(1 << EN_pin);					// Open-drain output
	
	GPIOA->OSPEEDR |= 
		(3 << (USART2_TX * 2)) 	|
		(3 << (USART2_RX * 2))	|
		(3 << (EN_pin	* 2));			// High speed
	
	GPIOA->PUPDR |=
		//(1 << (EN_pin * 2))		|	// R-pull up on the OD pin
		(1 << (OL_pin *	2));		// R-pull up on the input pin	
	
	GPIOA->ODR &= ~(1 << EN_pin);// Disable Transmitter
	
	GPIOA->AFR[0] |= 
		(1 << (USART2_TX * 4))	|
		(1 << (USART2_RX * 4));

	// IO-Link monitoring LED
	GPIOB->MODER &=
		~(
		(3 << (IOL_mon * 2))
		);
		
	GPIOB->MODER |= (1 << (IOL_mon * 2));
	GPIOB->OSPEEDR |= (3 << (IOL_mon *2)); 

}

// Init IO system
void app_initIO(){
	clk_initSysClk();								// Setup system clock
	systick_init(F_CPU, 1000);			// Setup 1000 systick
	app_initGPIO();
	adc_init();
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
					GPIOB->ODR ^= (1 << IOL_mon);
					if(iol_dl_getModeStatus() == DL_MODE_OP){
						//l6364_setLED2(0x00);// Red LED off
						alive_fsm = 1;
					}
				}
				break;
				
			case 1:// Status : IO-Link active
				{
					iol_pl_connectedLED();
					GPIOB->ODR |= (1 << IOL_mon);
					if(iol_dl_getModeStatus() != DL_MODE_OP){
						//l6364_setLED1(0x00);// Green LED off
						alive_fsm = 0;
					}
				}
				break;
		}
		
	}
}

int16_t temp_adc_read = 0;

void app_iol_updatePDTask(){
	if(temp_adc_read = adc_readScheduled(6), temp_adc_read > -1){
			iol_al_updatePD(temp_adc_read);// Update PD
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