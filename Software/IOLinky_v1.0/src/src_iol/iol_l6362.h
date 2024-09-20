#ifndef IOL_L6362_H
#define IOL_L6362_H

#include <stdint.h>
#include <string.h>

#include "usart.h"

typedef struct{
	uint8_t master_data_len;
	uint8_t master_od_len;
	uint8_t device_od_len;
}l6362_mseq_len_t;

typedef struct{
	void (*uart_init)(
		uint8_t commode,
		uint8_t read_size_max,
		uint8_t *read_ptr);
	
	uint8_t (*uart_getPEStatus)(void);
	
	void (*uart_setReadPointer)(uint8_t *read_ptr);
	uint8_t (*uart_getReadIndex)(void);
	uint8_t (*uart_resetReadIndex)(void);
	
	void (*uart_writeRequest)(
		uint8_t count,
		uint8_t *write_ptr);
	uint8_t (*uart_pollWrite)(void);
}l6362_uart_handler_t;

void l6362_init(uint8_t commode);

void l6362_setMseq(
	uint8_t m_data_len,
	uint8_t m_od_len,
	uint8_t d_od_len
	);

uint8_t l6362_parityChk();

// Read/Write FIFO

uint8_t l6362_readFIFO(
	uint8_t *output_ptr
	);

uint8_t l6362_writeFIFO(
	uint8_t count,
	uint8_t *input_ptr
	);

#endif