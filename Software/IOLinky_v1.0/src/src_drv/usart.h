#ifndef USART_H
#define USART_H

#include <stdint.h>
#include "gd32e23x.h"
#include "gd32e23x_rcu.h"
#include "gd32e23x_gpio.h"
#include "gd32e23x_usart.h"

#include "gpio_pinout.h"

enum IOL_COM_MODE{
	COM1 = 0,
	COM2,
	COM3,
};

void usart_initIOLink(
	uint8_t com_mode,				// IO-Link COM speed
	uint8_t read_size_max,	// RX buffer size
	uint8_t *read_ptr				// RX buffer	
);
	
void usart_setReadPtr(uint8_t *read_ptr);

void usart_writeRequest(
	uint8_t count,
	uint8_t *wr_ptr);
uint8_t usart_pollWrite();

uint8_t usart_getReadIdx();
void usart_resetReadIdx();

uint8_t usart_getPEStatus();

#endif