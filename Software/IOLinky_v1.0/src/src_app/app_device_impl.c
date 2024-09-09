#include "app_device_impl.h"

// Private variables
uint32_t led_millis = 0;
uint32_t PD_refresh_millis = 0;
uint32_t PSU_millis = 0;

// Setup all required GPIOs.
void app_initGPIO(){
	
	// Enable GPIO clock
	RCU_AHBEN |= 
		(1 << 17)	| // GPIOA Clock
		(1 << 18)	; // GPIOB Clock
	
	// Init SPI pins
	GPIO_CTL(GPIOA) &= 
		~(
		(3 << (USART2_TX * 2)) 		|
		(3 << (USART2_RX * 2)) 		|	
		(3 << (OL_pin * 2)) 			|
		(3 << (EN_pin	* 2))				|
		(3 << (Isense_pin * 2))		|	
		(3 << (V5sense_pin * 2))	|	
		(3 << (V24sense_pin * 2))	|
		(3 << (RedLED_Pin * 2))		|
		(3 << (GreenLED_Pin * 2))	
		);

	GPIO_CTL(GPIOA) |= 
		(0 << (OL_pin * 2)) 			| // Input 
		(2 << (USART2_TX * 2)) 		| // AF1
		(2 << (USART2_RX * 2)) 		| // AF1
		(1 << (EN_pin	* 2))				| // Output OD Pull-up
		(3 << (Isense_pin * 2))		|	// Analog input
		(3 << (V5sense_pin * 2))	|	// Analog input
		(3 << (V24sense_pin * 2))	| // Analog input
		(2 << (RedLED_Pin * 2))		|	// Red led pin
		(2 << (GreenLED_Pin * 2))	;	// Green led pin
	
	GPIO_OSPD(GPIOA) |= 
		(3 << (USART2_TX * 2)) 	|
		(3 << (USART2_RX * 2))	|
		(3 << (EN_pin	* 2));			// High speed
	
	GPIO_PUD(GPIOA) |=
		//(1 << (EN_pin * 2))		|	// R-pull up on the OD pin
		(1 << (OL_pin *	2));		// R-pull up on the input pin	
	
	GPIO_OCTL(GPIOA) &= ~(1 << EN_pin);// Disable Transmitter
	
	GPIO_AFSEL0(GPIOA) |= 
		(1 << (USART2_TX * 4))	|
		(1 << (USART2_RX * 4));

	GPIO_AFSEL1(GPIOA) |=
		(2 << ((RedLED_Pin - 8) * 4))		| // TIMER0 CH1
		(2 << ((GreenLED_Pin - 8) * 4))	;	// TIMER0 CH2

	// IO-Link monitoring LED
	GPIO_CTL(GPIOB) &=
		~(
		//(3 << (IOL_mon * 2))		|
		(3 << (ENBuck_Pin * 2))	
		);
		
	GPIO_CTL(GPIOB) |= 
		//(1 << (IOL_mon * 2))		|	// IO-Link activity monitoring pin
		(1 << (ENBuck_Pin * 2))	;	// Buck converter enable pin

}

// Init IO system
void app_initIO(){
	clk_initSysClk();								// Setup system clock
	systick_init(F_CPU, 10000);			// Setup 10kHz systick
	
	app_initGPIO();
	
	timerled_init();
	app_psu_init();
	iol_al_init(
	// Passing pointer to Process Data
	(uint8_t *)&psu_mondata_t.PSU_status_w
	);
}

// alive LED status task.
void app_iol_aliveTask(){
	
	if((millis() - led_millis) > PERIOD_ALIVE_TASK){
		led_millis = millis();
		
		if(iol_dl_getModeStatus() == DL_MODE_OP){
			iol_pl_connectedLED();
		}else{
			iol_pl_standbyLED();
		}
		
	}
}

void app_iol_updatePDTask(){
	if((millis() - PD_refresh_millis) > PERIOD_PD_TASK){
		PD_refresh_millis = millis();
		iol_al_updatePD();	
	}
}

void app_iol_psuRunner(){
	if((millis() - PSU_millis) > PERIOD_PSU_TASK){
		PSU_millis = millis();
		app_psu_runner();
	}
}

// Tasks that runs this IO-Link device
void app_runner(){
	app_iol_psuRunner();
	iol_al_poll();
	app_iol_updatePDTask();
	app_iol_aliveTask();
}