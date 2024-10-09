#include "app_device_impl.h"

// Private variables
uint32_t led_millis = 0;
uint32_t PD_refresh_millis = 0;
uint32_t PSU_millis = 0;

uint8_t alive_led = 0;

// Setup all required GPIOs.
void app_initGPIO(){
	
	// Enable GPIO clock
	RCU_AHBEN |= 
		(1 << 17)	| // GPIOA Clock
		(1 << 18)	; // GPIOB Clock
	
	// PORT-A
	GPIO_CTL(GPIOA) |= 
		(1 << (IOL_mon * 2))			|	// IO-Link activity monitoring pin
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
		(1 << (OL_pin *	2));		// R-pull up on the input pin	
	
	GPIO_OCTL(GPIOA) &= ~(1 << EN_pin);// Disable Transmitter
	
	GPIO_AFSEL0(GPIOA) |= 
		(1 << (USART2_TX * 4))	|
		(1 << (USART2_RX * 4));

	GPIO_AFSEL1(GPIOA) |=
		(2 << ((RedLED_Pin - 8) * 4))		| // TIMER0 CH1
		(2 << ((GreenLED_Pin - 8) * 4))	;	// TIMER0 CH2

	// PORT-B
	GPIO_CTL(GPIOB) |= 
		(1 << (ENBuck_Pin * 2))	;	// Buck converter enable pin

}

// Init IO system
void app_initIO(){
	clk_initSysClk();								// Setup system clock
	systick_init(F_CPU, 10000);			// Setup 10kHz systick
	
	app_initGPIO();
	
	timerled_init();
	
	app_nvm_init();
	
	app_psu_init();
	
	iol_sm_init(
		COM3,
		(uint8_t *)&psu_mondata_t.PSU_status_w
	);
}

// Run the standby LED
void app_iol_standbyLED(){// Red LED	
	if(alive_led > 0x08){
		alive_led--;
		if(alive_led == 0x10)
			alive_led = 0;
	}else{
		alive_led++;
		if(alive_led == 0x08)
			alive_led = 0x18;
	}
	
	timerled_setRedLED(alive_led & 0x0F);
	timerled_setGreenLED(0);
}

// Run the connected LED
void app_iol_connectedLED(){// Green LED
	if(alive_led > 0x08){
		alive_led--;
		if(alive_led == 0x10)
			alive_led = 0;
	}else{
		alive_led++;
		if(alive_led == 0x08)
			alive_led = 0x18;
	}
	
	timerled_setRedLED(0);
	timerled_setGreenLED(alive_led & 0x0F);
}

// Run both LED to get Yellow warning error.
void app_iol_faultLED(){
	if(alive_led > 0x08){
		alive_led--;
		if(alive_led == 0x10)
			alive_led = 0;
	}else{
		alive_led++;
		if(alive_led == 0x08)
			alive_led = 0x18;
	}
	
	timerled_setRedLED(alive_led & 0x0F);
	timerled_setGreenLED(alive_led & 0x0F);
}

// alive LED status task.
void app_iol_aliveTask(){
	if((millis() - led_millis) >= PERIOD_ALIVE_TASK){
		led_millis = millis();

		switch(app_psu_status()){
			case PSU_STATE_INIT:
			case PSU_STATE_IDLE:
			case PSU_STATE_NORMAL:
			case PSU_STATE_PWROFF:
			{
				if(iol_dl_getModeStatus() == DL_MODE_OP){
					app_iol_connectedLED();
				}else{
					app_iol_standbyLED();
				}
			}
			break;
			
			default:
				iol_al_AppReportEvent(0x5100);
				app_iol_faultLED();
			break;
		}
	}
}

void app_iol_updatePDTask(){
	if((millis() - PD_refresh_millis) >= PERIOD_PD_TASK){
		PD_refresh_millis = millis();
		iol_al_updatePD();	
	}
}

void app_iol_psuRunner(){
	if((millis() - PSU_millis) >= PERIOD_PSU_TASK){
		PSU_millis = millis();
		app_psu_runner();	
	}
}

// Tasks that runs this IO-Link device
inline void app_runner(){
	iol_al_poll();
	app_iol_psuRunner();
	app_iol_updatePDTask();
	app_iol_aliveTask();
}