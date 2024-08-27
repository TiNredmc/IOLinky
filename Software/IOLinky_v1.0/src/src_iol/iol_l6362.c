#include "iol_l6362.h"

// Private pointers

// Private variables
uint8_t l6362_write_fifo[32] = {0};
uint8_t l6362_read_fifo[32] = {0};

uint8_t read_fifo_fsm		= 0;
uint8_t read_fifo_size 	= 0;

uint8_t write_fifo_fsm	= 0;

// Private typedef
l6362_mseq_len_t l6362_m_len_t;

void l6362_init(uint8_t commode){
	usart_initIOLink(
		commode,
		32,
		l6362_read_fifo,
		l6362_write_fifo);
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

	switch(read_fifo_fsm){
		case 0:// Wait for MC byte
		{
			if(usart_getReadIdx() > 1){
				// Check MC R/W bit 
				if(l6362_read_fifo[0] & 0x80)
					read_fifo_fsm = 1;// Read request
				else
					read_fifo_fsm = 2;// Write request
				
				// reset read_fifo_size
				read_fifo_size = 0;
			}
			
		}
		break;
		
		// Read request from IO-Link master doesn't include Master's OD byte(s)
		case 1:// 
		{
			read_fifo_size = l6362_m_len_t.master_data_len;
			read_fifo_fsm = 3;
		}
		break;
		
		// Write request from IO-Link master includes Master's OD byte(s)
		case 2:
		{
			read_fifo_size = 
				l6362_m_len_t.master_data_len +
				l6362_m_len_t.master_od_len;
			read_fifo_fsm = 3;
		}
		break;
		
		case 3:// Check until we got all master message
		{
			if(usart_getReadIdx() == read_fifo_size){
				// Copy data from internal FIFO to 
				// Upper layer's buffer
				memcpy(
					output_ptr, 
					l6362_read_fifo, 
					read_fifo_size);
				
				// Reset USART FIFO pointer
				usart_resetReadIdx();
				
				read_fifo_fsm = 0;
				
				// Return read count
				return read_fifo_size;
			}
		}
		break;
	
	}
	
	return 0;	
}
	
uint8_t l6362_writeFIFO(
	uint8_t count,
	uint8_t *input_ptr
	){
	
	uint8_t ret = 0;	
		
	switch(write_fifo_fsm){
		case 0:
		{
			if(count > 0){
				usart_writeRequest(count);
				memcpy(
					l6362_write_fifo, 
					input_ptr, 
					count);
				
				ret = count;
				
				write_fifo_fsm = 1;
			}
		}
		break;
	
		case 1:
		{
			ret = usart_pollWrite();
			if(ret == 0){
				write_fifo_fsm = 0;
			}
			
		}
		break;
		
	}
	
	return ret;
}