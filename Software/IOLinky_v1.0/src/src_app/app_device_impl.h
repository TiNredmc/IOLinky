#ifndef APP_DEVICE_IMPL_H
#define APP_DEVICE_IMPL_H

#include <stdint.h>

#include "gd32e23x.h"
#include "gd32e23x_rcu.h"
#include "gd32e23x_gpio.h"

#include "clk.h"
#include "Systick_delay.h"
#include "tim0_led.h"


#include "iol_sm.h"
#include "iol_al.h"

#include "app_nvm.h"
#include "app_psu.h"

#define F_CPU		72000000UL

// Super loop define-s
// define task period in ms unit
#define PERIOD_ALIVE_TASK	(100 * 10)// 100ms
#define PERIOD_PD_TASK		(20 * 10)	// 20ms
#define PERIOD_PSU_TASK		(1 * 10)  // 1ms

void app_initIO();
void app_runner();

#endif