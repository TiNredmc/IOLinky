#ifndef USART_H
#define USART_H

#include <stdint.h>
#include "gd32e23x.h"
#include "gd32e23x_rcu.h"
#include "gd32e23x_gpio.h"
#include "gd32e23x_usart.h"

#include "gpio_pinout.h"


void usart_init(
	uint32_t baud_rate,			// IO-Link COM speed
	uint8_t read_size_max,	// RX buffer size
	uint8_t *read_ptr				// RX buffer	
);
	
void usart_setReadPtr(uint8_t *read_ptr);

uint8_t usart_getTransferCompleted();

void usart_enableTX();
void usart_disableTX();

void usart_write(uint8_t wdata);
void usart_writePtr(uint8_t *wdata_ptr);

uint8_t usart_getReadIdx();
void usart_resetReadIdx();

uint8_t usart_getPEStatus();

#endif