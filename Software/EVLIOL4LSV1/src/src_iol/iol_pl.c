#include "iol_pl.h"

// Private variable
uint8_t pl_rx_iol_byte = 0;
uint8_t pl_tx_iol_byte = 0;

uint8_t current_mtype = 0;

uint8_t pl_write_fsm = 0;
uint8_t write_request = 0;
uint8_t read_available = 0;

uint8_t alive_led = 0;

// Private pointers

uint8_t *read_buffer_ptr;
uint8_t *write_buffer_ptr;



// Initialize PHY, setting up data pointers
void iol_pl_init(
	uint8_t *rd_buffer_ptr,
	uint8_t *wr_buffer_ptr
	){
	
	read_buffer_ptr = rd_buffer_ptr;	
	write_buffer_ptr = wr_buffer_ptr;
		
	l6364_setIOLmode();// Put chip into IO-Link mode
	l6364_setCOM(COM2);// COM2 mode	
	l6364_setLED1(0x00);// Green LED off
	l6364_setLED2(0x00);// Red LED off
		
	// Startup with Type-0 to detect wakeup sequence
	iol_pl_setMtype0();
}
	
// Update read buffer pointer
void iol_pl_update_ReadBuffer(uint8_t *rd){
	read_buffer_ptr = rd;
}

// Update write buffer pointer
void iol_pl_update_WriteBuffer(uint8_t *wr){
	write_buffer_ptr = wr;
}
	
// Update read and write buffer pointer
void iol_pl_updateBuffer(
	uint8_t *rd_buffer_ptr,
	uint8_t *wr_buffer_ptr
	){
	read_buffer_ptr = rd_buffer_ptr;	
	write_buffer_ptr = wr_buffer_ptr;
}

// Run the standby LED
void iol_pl_standbyLED(){// Red LED
	if(alive_led > 0x08){
		alive_led--;
		if(alive_led == 0x10)
			alive_led = 0;
	}else{
		alive_led++;
		if(alive_led == 0x08)
			alive_led = 0x18;
	}
	
	l6364_setLED2(alive_led & 0x0F);
}

// Run the connected LED
void iol_pl_connectedLED(){// Green LED
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

// Poll reading from the L6364
void iol_pl_pollRead(){
		read_available = l6364_readFIFOFast(read_buffer_ptr);
	
	if(l6364_isCHK()){
		l6364_linkEnd();
		read_available = 0;
	}
}

// Poll for write request
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

// Check if data ready to read from PHY
uint8_t iol_pl_ReadAvailable(){
	if(read_available == 0)
		return 0;
	
	read_available = 0;
	return 1;
}

// Send write request to PHY
void iol_pl_WriteRequest(uint8_t count){
	pl_tx_iol_byte = count;
	write_request = 1;
}

// Check writing status
uint8_t iol_pl_checkWriteStatus(){
	return !write_request;
}	

// send link "send" to the L6364
void iol_pl_LinkSND(){
	l6364_linkSend();
}

// send link "end" to the L6364
void iol_pl_LinkEND(){
	l6364_linkEnd();
}

// ========= Setting M-Sequence Type ===============
// Return the current M-seq type according to number in enum 
uint8_t iol_pl_getCurrentMtype(){
	return current_mtype;
}

// Set the M-seq type to TYPE_0
void iol_pl_setMtype0(){
	l6364_setMseq(
		2,
		MSEQ_OD1_BC,
		MSEQ_OD2_1B
	);
	current_mtype = MTYPE_0;
}

// Set the M-seq type to TYPE1_2
void iol_pl_setMtype1_2(){
	l6364_setMseq(
		2,
		MSEQ_OD1_2B,
		MSEQ_OD2_2B
	);
	current_mtype = MTYPE_1_2;
}

// Set the M-seq type to TYPE2_2
void iol_pl_setMtype2_2(){
	l6364_setMseq(
		2,
		MSEQ_OD1_BC,// Ignored by the chip in Type 2
		MSEQ_OD2_1B
	);
	current_mtype = MTYPE_2_2;
}

// ========= END Set M-Seq Type ====================