#include "iol_pl.h"

uint8_t pl_rx_iol_byte = 0;
uint8_t pl_tx_iol_byte = 0;

uint8_t *read_buffer_ptr;
uint8_t *write_buffer_ptr;

uint8_t current_mtype = 0;

uint8_t pl_write_fsm = 0;
uint8_t write_request = 0;

uint8_t alive_led = 0;

void iol_pl_init(
	uint8_t *rd_buffer_ptr,
	uint8_t *wr_buffer_ptr
	){
	
	read_buffer_ptr = rd_buffer_ptr;	
	write_buffer_ptr = wr_buffer_ptr;
		
	l6364_setIOLmode();	
	l6364_setCOM(COM3);// COM3 mode	
	l6364_setLED1(0x01);// Arbitrary number to set LED on. Indicates the successful SPI comm. 

	// Startup with Type-0 to detect wakeup sequence
	iol_pl_setMtype0();
}
	
void iol_pl_aliveLED(){
	if(alive_led > 0x04){
		alive_led--;
		if(alive_led == 0x10)
			alive_led = 0;
	}else{
		alive_led++;
		if(alive_led == 0x04)
			alive_led = 0x14;
	}
	
	l6364_setLED2(alive_led & 0x0F);
}

void iol_pl_poll(){
	l6364_pollAllReg();
	
	// if data present in the buffer
	if(l6364_isDAT()){
		pl_rx_iol_byte = l6364_pollLinkCnt();
		l6364_readFIFO(
			pl_rx_iol_byte,
			read_buffer_ptr
		);
	}
	
	// Data Write FSM 
	switch(pl_write_fsm){
		case 0:// Wait for write request flag
		{
			if(write_request){
				// make sure that write buffer (source) is not NULL
				if(write_buffer_ptr == NULL)
					break;
				
				if(pl_tx_iol_byte == 0)
					break;
				
				// clear flag
				write_request = 0;
				
				// Set Link (data) count
				l6364_setTXCount(pl_tx_iol_byte);
				
				pl_write_fsm = 1;
			}
		}
		break;
		
		case 1:// Write data to L6364
		{
			l6364_writeFIFO(
				pl_tx_iol_byte,
				write_buffer_ptr
			);
			
			pl_write_fsm = 2;
		}
		break;
		
		case 2:// Check status
		{
		
			
			pl_write_fsm = 0;
		}
		break;
		
		default:
			pl_write_fsm = 0;
			break;
	
	}
	

}

void iol_pl_dlReadBuffer(){
 
 
}

void iol_pl_dlWriteRequest(uint8_t count){
	pl_tx_iol_byte = count;
	write_request = 1;
}

uint8_t iol_pl_getCurrentMtype(){
	return current_mtype;
}

void iol_pl_setMtype0(){
	l6364_setMseq(
		2,
		MSEQ_OD1_BC,
		MSEQ_OD2_1B
	);
	current_mtype = MTYPE_0;
}

void iol_pl_setMtype1_2(){
	l6364_setMseq(
		2,
		MSEQ_OD1_2B,
		MSEQ_OD2_2B
	);
	current_mtype = MTYPE_1_2;
}