#include "iol_l6362.h"

// Private pointers
uint8_t *read_fifo_ptr;

// Private variables
uint8_t l6362_read_fifo[32] = {0};

uint8_t read_fifo_size 	= 0;

// Private typedef
l6362_mseq_len_t l6362_m_len_t;

void l6362_init(uint8_t commode){
	usart_initIOLink(
		commode,
		32,
		l6362_read_fifo);
	
	usart_resetReadIdx();
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
	return usart_getPEStatus();
}

uint8_t l6362_readFIFO(
	uint8_t *output_ptr
	){

	if(output_ptr == 0)
			return 0;
	
	if(output_ptr != read_fifo_ptr){
		read_fifo_ptr = output_ptr;
		usart_setReadPtr(read_fifo_ptr);
	}
		
	if(usart_getReadIdx() > 0){
		// Check MC R/W bit 
		if(read_fifo_ptr[0] & 0x80){
			// Read request
			read_fifo_size = l6362_m_len_t.master_data_len;
		}else{
			// Write request
			read_fifo_size = 
				l6362_m_len_t.master_data_len +
				l6362_m_len_t.master_od_len;
		}
	}
	
	if(usart_getReadIdx() == read_fifo_size){		
		// Reset USART FIFO pointer
		usart_resetReadIdx();
		return read_fifo_size;
	}
	
	return 0;	
}
	
uint8_t l6362_writeFIFO(
	uint8_t count,
	uint8_t *input_ptr
	){
	
	uint8_t ret = 0;	
		
	if(count > 0){
		usart_writeRequest(
			count,
			input_ptr);
	}	
	
	ret = usart_pollWrite();
	
	return ret;
}