#include "app_device_impl.h"

// Private variables
uint32_t led_millis = 0;
uint32_t PD_refresh_millis = 0;
uint8_t alive_fsm = 0;
uint8_t app_fsm = 0;

// Private typedef
adc_buf_t adc_measurement_t;

// Setup all required GPIOs.
void app_initGPIO(){
	
	// Enable GPIO clock
	RCU_AHBEN |= 
		(1 << 17)	| // GPIOA Clock
		(1 << 18)	; // GPIOB Clock
	
	// Init SPI pins
	GPIO_CTL(GPIOA) &= 
		~(
		(3 << (USART2_TX * 2)) 	|
		(3 << (USART2_RX * 2)) 	| 
		(3 << (OL_pin * 2)) 		|
		(3 << (EN_pin	* 2))			|
		(3 << (Isense_pin * 2))	|	
		(3 << (V5sense_pin * 2))|	
		(3 << (V24sense_pin * 2))
		);

	GPIO_CTL(GPIOA) |= 
		(0 << (OL_pin * 2)) 		| // Input 
		(2 << (USART2_TX * 2)) 	| // AF1
		(2 << (USART2_RX * 2)) 	| // AF1
		(1 << (EN_pin	* 2))			| // Output OD Pull-up
		(3 << (Isense_pin * 2))	|	// Analog input
		(3 << (V5sense_pin * 2))|	// Analog input
		(3 << (V24sense_pin * 2)); // Analog input
	
//	GPIO_OMODE(GPIOA) |= 
//		(1 << EN_pin);					// Open-drain output
	
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

	// IO-Link monitoring LED
	GPIO_CTL(GPIOB) &=
		~(
		(3 << (IOL_mon * 2))
		);
		
	GPIO_CTL(GPIOB) |= (1 << (IOL_mon * 2));
	GPIO_OSPD(GPIOA) |= (3 << (IOL_mon *2)); 

}

// Init IO system
void app_initIO(){
	clk_initSysClk();								// Setup system clock
	systick_init(F_CPU, 1000);			// Setup 1000 systick
	app_initGPIO();
	adc_initScanDMA((uint16_t*)&adc_measurement_t);
	adc_softTrigger();// Initial ADC conversion
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
					GPIO_OCTL(GPIOB) ^= (1 << IOL_mon);
					if(iol_dl_getModeStatus() == DL_MODE_OP){
						
						alive_fsm = 1;
					}
				}
				break;
				
			case 1:// Status : IO-Link active
				{
					iol_pl_connectedLED();
					GPIO_OCTL(GPIOB) |= (1 << IOL_mon);
					if(iol_dl_getModeStatus() != DL_MODE_OP){
						
						alive_fsm = 0;
					}
				}
				break;
		}
		
	}
}

int16_t temp_adc_read = 0;

void app_iol_updatePDTask(){
	if((millis() - PD_refresh_millis) > 20){
		PD_refresh_millis = millis();
		
		if(adc_getDataAvaible()){
			iol_al_updatePD(adc_measurement_t.Isense_val);	
				
			adc_softTrigger();// Trigger next ADC conversion
		}
	}
}

// Tasks and FSMs that runs this IO-Link device
void app_runner(){
	switch(app_fsm){
		case APP_INIT_STATE:// App initialize state
		{
			iol_al_init();
			app_fsm = APP_RUN_STATE;
		}
		break;
		
		case APP_RUN_STATE:// App running state
		{
			iol_al_poll();
			app_iol_updatePDTask();
			app_iol_aliveTask();
		}
		
		default:
			break;
	
	}
}