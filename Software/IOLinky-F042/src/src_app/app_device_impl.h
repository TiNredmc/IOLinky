#ifndef APP_DEVICE_IMPL_H
#define APP_DEVICE_IMPL_H

#include <stdint.h>

#include "clk.h"
#include "Systick_delay.h"

#include "iol_l6362.h"

#include "iol_pl.h"
#include "iol_dl.h"
#include "iol_al.h"

#define F_CPU		48000000UL


enum APP_FSM_STATEs{
	APP_INIT_STATE = 0,
	APP_RUN_STATE = 1
};


void app_initIO();
void app_runner();

#endif