#include "tim0_led.h"

void timerled_init(){
	RCU_APB2EN |= (1 << 11);// Enable TIMER0 clock
	
	TIMER_PSC(TIMER0) = 2812;// 72Mhz/(256*1000) -> 2812
	TIMER_CAR(TIMER0) = 0x00FF;// 8bit pwm
	
	TIMER_CHCTL0(TIMER0) = 
		(7 << 12)					|
		(1 << 11)					;	// CH1 as PWM
	
	TIMER_CHCTL1(TIMER0) =
		(7 << 4)					|
		(1 << 3)					;	// CH2 as PWM
	
	TIMER_CHCTL2(TIMER0) = 
		(1 << 8)		|				// Enable CH2
		(1 << 4)		;				// Enable CH1
		
	TIMER_CCHP(TIMER0) |= (1 << 14);// Enable Primary output
	
	TIMER_CTL0(TIMER0) |= 1;// Enable Timer
}

void timerled_setRedLED(uint8_t intensity){
	TIMER_CH1CV(TIMER0) = (uint16_t)intensity;
}

void timerled_setGreenLED(uint8_t intensity){
	TIMER_CH2CV(TIMER0) = (uint16_t)intensity;
}