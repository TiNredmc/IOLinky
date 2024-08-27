#ifndef APP_DEVICE_IMPL_H
#define APP_DEVICE_IMPL_H

#include <stdint.h>

#include "gd32e23x.h"
#include "gd32e23x_rcu.h"
#include "gd32e23x_gpio.h"

#include "clk.h"
#include "Systick_delay.h"
#include "adc.h"

#include "iol_l6362.h"

#include "iol_pl.h"
#include "iol_dl.h"
#include "iol_al.h"

#define F_CPU		72000000UL


enum APP_FSM_STATEs{
	APP_INIT_STATE = 0,
	APP_RUN_STATE = 1
};


typedef struct{
	uint16_t Isense_val;
	uint16_t V5sense_val;
	uint16_t V24sense_val;
}adc_buf_t;

void app_initIO();
void app_runner();

#endif