#ifndef APP_PSU_H
#define APP_PSU_H

#include "gd32e23x_gpio.h"
#include "gpio_pinout.h"
#include "adc.h"

#include "app_nvm.h"

// PSU error timeout before halt
#define TIMEOUT_SC				300 // 300ms
#define TIMEOUT_VO_OV			500 // 500ms

// Recovery wait
#define RECOVER_HICCUP		5000 // 5 sec


// PSU Voltage threshold

// Calculation from Vin to ADC
// (Vin * 0.11) * (4095 / 3.3)

#define THRESHOLD_VIN_LO_DN	2909 // around 21V
#define THRESHOLD_VIN_LO_UP	3180 // around 23V

#define THRESHOLD_VIN_HI	3535 // around 25.5V	

// Calculation from Vo to ADC
// (Vin/2) * (4095 / 3.3)
#define THRESHOLD_VO_LO		2792 // around 4.5V
#define THRESHOLD_VO_HI		3413 // around 5.5v

// Calculation from Current to ADC
// (I*0.01*50) * (4095 / 3.3)
// (I/2) * (4095 / 3.3)
// PSU Current threshold
#define THRESHOLD_IO_NOM	1861 // around 3A
#define THRESHOLD_IO_I2T	1954 // 5% morethan Io_nom
#define THRESHOLD_IO_PEAK	2172 // around 3.5A 116%
#define THRESHOLD_IO_HI		2515 // around 4A ~130%
#define THRESHOLD_IO_SC		2830 // short circuit at 4.5A 150%
// Efuse calculation
// (Iout^2 - Ipeak^2) * t
// Inom is 3A
// Pick Ipeak at 3.5A
// Allowed t time at 3.5A is 6000ms
// Number max according to int32_t
// 
// Actual calculation performed inside MCU
// ((Imeasured - Inom) * (Imeasured + Inom)) >> 10
// All integer 
#define THRESHOLD_EFUSE		7344000	 


typedef struct __attribute__((packed)){
	union{
		uint16_t PSU_status_w;
		struct{
			uint8_t VIn_ok 		:1;// normal voltage
			uint8_t VIn_UV 		:1;// under voltage
			uint8_t VIn_OV		:1;// over voltage
			uint8_t VOut_ok 	:1;// normal voltage
			
			uint8_t VOut_UV		:1;// under voltage
			uint8_t VOut_OV 	:1;// over voltage
			uint8_t IOut_ok	 	:1;// normal IOut
			uint8_t IOut_OC 	:1;// Overcurrent detected
			
			uint8_t IOut_SC 	:1;// Short circuit detected
			uint8_t Efuse_Act :1;// Efuse active status
			uint8_t Efuse_Trip:1;// Efuse tripped
			uint8_t Buck_en		:1;// Buck converter enable status
			
			uint8_t Buck_Hiccup :1;// Hiccup mode indicator
			uint8_t sl :1;
			uint8_t sm :1;
			uint8_t sn :1;
		}PSU_status_b;
	};

	uint16_t IOsense_val;
	uint16_t VOsense_val;
	uint16_t VIsense_val;	
}psu_data_t;

typedef struct __attribute__((packed)){
	uint16_t IOsense_raw;
	uint16_t VOsense_raw;
	uint16_t VIsense_raw;
}psu_raw_t;

extern psu_raw_t psu_rawmondata_t;
extern psu_data_t psu_mondata_t;

enum PSU_FSM_ENUM{
	PSU_STATE_INIT 	= 0,
	PSU_STATE_IDLE,
	PSU_STATE_NORMAL,
	
	PSU_STATE_VOUT_NOK,
	PSU_STATE_VIN_NOK,
	
	PSU_STATE_OLP,
	
	PSU_STATE_SC,
	PSU_STATE_HALT,
	
	PSU_STATE_PWROFF
};

void app_psu_init();
void app_psu_runner();
uint8_t app_psu_status();

void app_psu_requestPWRON();
void app_psu_requestPWROFF();
uint8_t app_psu_getPWRStatus();

int32_t app_mon_getEfuse();
void app_mon_updateEfuseThreshold();

#endif