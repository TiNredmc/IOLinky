#include "iol_l6362.h"

#define RX_FIFO_SIZE	32

// Private pointers
uint8_t *read_fifo_ptr;
uint8_t *write_fifo_ptr;

// Private variables

// RX
uint8_t l6362_read_fifo[32] = {0};
uint8_t l6362_read_fifo_size 	= 0;
// TX
uint8_t l6362_uart_tx_fsm = 0;
uint8_t l6362_write_size = 0;
uint8_t l6362_write_idx = 0;

// Private typedef
l6362_mseq_len_t l6362_m_len_t;
l6362_uart_handler_t *uart_pHandle_t;

// Private prototypes
uint8_t l6362_pollWrite();

void l6362_init(
	l6362_uart_handler_t *l6362_pHandle,
	uint32_t uart_baud){
	
	if(l6362_pHandle == 0)
			return;
	
	uart_pHandle_t = l6362_pHandle;

	uart_pHandle_t->uart_init(
		uart_baud,
		RX_FIFO_SIZE,
		l6362_read_fifo
	);
	
	uart_pHandle_t->uart_resetReadIndex();
}

void l6362_setMseq(
	uint8_t m_data_len,
	uint8_t m_od_len,
	uint8_t d_od_len
	){
	
	l6362_m_len_t.master_data_len = m_data_len;
	l6362_m_len_t.master_od_len		= m_od_len;
	l6362_m_len_t.device_od_len		=	d_od_len;
}
	
uint8_t l6362_parityChk(){
	return uart_pHandle_t->uart_getPEStatus();
}

uint8_t l6362_readFIFO(
	uint8_t *output_ptr
	){

	if(output_ptr == 0)
			return 0;
	
	// Only update pointer when it changed
	if(output_ptr != read_fifo_ptr){
		read_fifo_ptr = output_ptr;
		uart_pHandle_t->uart_setReadPointer(
			read_fifo_ptr
		);
		GPIO_OCTL(GPIOA) |= (1 << 0);
	}
		
	if(uart_pHandle_t->uart_getReadIndex() > 0){
		// Check MC R/W bit 
		if(read_fifo_ptr[0] & 0x80){
			// Read request
			l6362_read_fifo_size = l6362_m_len_t.master_data_len;
		}else{
			// Write request
			l6362_read_fifo_size = 
				l6362_m_len_t.master_data_len +
				l6362_m_len_t.master_od_len;
		}
	
		if(uart_pHandle_t->uart_getReadIndex() == 
			l6362_read_fifo_size){		
			// Reset USART FIFO pointer
			uart_pHandle_t->uart_resetReadIndex();
			GPIO_OCTL(GPIOA) &= ~(1 << 0);
			return l6362_read_fifo_size;
		}
	
	}
	
	return 0;	
}
	
uint8_t l6362_writeFIFO(
	uint8_t count,
	uint8_t *input_ptr
	){
	
	uint8_t ret = 0;	
	
	if(l6362_uart_tx_fsm == 0){
		if(count > 0){
			if(input_ptr == 0)
				return 0;
				
			l6362_write_size = count;
			write_fifo_ptr = input_ptr;
		}
	}
	
	ret = l6362_pollWrite();
	
	return ret;
}
	
uint8_t l6362_pollWrite(){

	switch(l6362_uart_tx_fsm){
		case 0:// Idle state
		{
			// We got write request
			if(l6362_write_size > 0){
				l6362_write_idx = 0;// reset index pointer
				uart_pHandle_t->uart_enableTX();
				l6362_uart_tx_fsm = 1;
			}
		}
		break;
		
		case 1:// Write to TX
		{
			uart_pHandle_t->uart_write(
				*(write_fifo_ptr + l6362_write_idx)
			);
				
			l6362_write_idx++;
			
			l6362_uart_tx_fsm = 2;
		}
		break;
		
		case 2:// Check transfer complete
		{
			if(uart_pHandle_t->uart_getTCStatus()){
				l6362_write_size--;
				if(l6362_write_size == 0){
					uart_pHandle_t->uart_disableTX();
					l6362_uart_tx_fsm = 0;
				}else{
					l6362_uart_tx_fsm = 1;
				}
			}
		}
		break;
	}
	
	return l6362_write_size;// Return remaning
}