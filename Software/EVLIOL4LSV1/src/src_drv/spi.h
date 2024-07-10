#ifndef SPI_H
#define SPI_H

#include <stdint.h>
#include <stdio.h>

#include "stm32g071xx.h"
#include "gpio_pinout.h"

void spi_init();

void spi_read(uint8_t *data, size_t len);
void spi_Write(uint8_t *data, size_t len);

void spi_WriteAndRead(
	uint8_t *TXbuf, 
	uint8_t *RXbuf,
	size_t len);

void spi_WriteOnceAndRead(
	uint8_t TX,
	uint8_t *RXbuf,
	size_t len);

void spi_WriteOnceAndWrite(
	uint8_t TX,
	uint8_t *TXbuf,
	size_t len);

#endif