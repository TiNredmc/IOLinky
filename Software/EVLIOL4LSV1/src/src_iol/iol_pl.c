#include "iol_pl.h"

uint8_t pl_rx_iol_byte = 0;
uint8_t pl_tx_iol_byte = 0;

uint8_t *read_buffer_ptr;
uint8_t *write_buffer_ptr;

uint8_t current_mtype = 0;

uint8_t pl_write_fsm = 0;
uint8_t write_request = 0;

uint8_t read_available = 0;

uint8_t alive_led = 0;

void iol_pl_init(
	uint8_t *rd_buffer_ptr,
	uint8_t *wr_buffer_ptr
	){
	
	read_buffer_ptr = rd_buffer_ptr;	
	write_buffer_ptr = wr_buffer_ptr;
		
	l6364_setIOLmode();// Put chip into IO-Link mode
	l6364_setCOM(COM2);// COM2 mode	
	l6364_setLED1(0x00);	
	l6364_setLED2(0x00);
		
	// Startup with Type-0 to detect wakeup sequence
	iol_pl_setMtype0();
}
	
void iol_pl_update_ReadBuffer(uint8_t *rd){
	read_buffer_ptr = rd;
}

void iol_pl_update_WriteBuffer(uint8_t *wr){
	write_buffer_ptr = wr;
}
	
void iol_pl_updateBuffer(
	uint8_t *rd_buffer_ptr,
	uint8_t *wr_buffer_ptr
	){
	read_buffer_ptr = rd_buffer_ptr;	
	write_buffer_ptr = wr_buffer_ptr;
}

void iol_pl_aliveLED(){
	if(alive_led > 0x08){
		alive_led--;
		if(alive_led == 0x10)
			alive_led = 0;
	}else{
		alive_led++;
		if(alive_led == 0x08)
			alive_led = 0x18;
	}
	
	l6364_setLED1(alive_led & 0x0F);
}

void iol_pl_pollRead(){
		read_available = l6364_readFIFOFast(read_buffer_ptr);
	
	if(l6364_isCHK()){
		l6364_linkEnd();
		read_available = 0;
	}
}

void iol_pl_pollWrite(){
	// Data write
	if(write_request){
		write_request = 0;
		l6364_writeFIFOFast(
			pl_tx_iol_byte,
			write_buffer_ptr
		);
	}
}

uint8_t iol_pl_ReadAvailable(){
	if(read_available == 0)
		return 0;
	
	read_available = 0;
	return 1;
}

void iol_pl_WriteRequest(uint8_t count){
	pl_tx_iol_byte = count;
	write_request = 1;
}

uint8_t iol_pl_checkWriteStatus(){
	return !write_request;
}	

void iol_pl_LinkSND(){
	l6364_linkSend();
}

void iol_pl_LinkEND(){
	l6364_linkEnd();
}

// ========= Setting M-Sequence Type ===============
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

// ========= END Set M-Seq Type ====================