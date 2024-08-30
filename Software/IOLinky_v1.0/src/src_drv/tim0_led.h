#ifndef TIM_LED_H
#define TIM_LED_H

#include "gd32e23x.h"
#include "gd32e23x_rcu.h"
#include "gd32e23x_timer.h"

void timerled_init();

void timerled_setRedLED(uint8_t intensity);
void timerled_setGreenLED(uint8_t intensity);

#endif