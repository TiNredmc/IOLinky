#include "iol_pl.h"

// Private variable
uint8_t pl_rx_iol_byte = 0;
uint8_t pl_tx_iol_byte = 0;

uint8_t current_mtype = 0;

uint8_t pl_write_fsm = 0;
uint8_t write_request = 0;
uint8_t read_available = 0;

uint8_t alive_led = 0;
uint8_t alive_switch_updn = 0;
// Private pointers

uint8_t *read_buffer_ptr;
uint8_t *write_buffer_ptr;

// Private typedef
iol_pl_handler_t *phy_pHandler_t;

// Initialize PHY, setting up data pointers
void iol_pl_init(
	iol_pl_handler_t *pl_pHandler_t
	){
	
	if(pl_pHandler_t == 0)
		return;
	
	phy_pHandler_t = pl_pHandler_t;

	phy_pHandler_t->phy_init();
		
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
	
	timerled_setRedLED(alive_led & 0x0F);
	timerled_setGreenLED(0);
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
	
	timerled_setRedLED(0);
	timerled_setGreenLED(alive_led & 0x0F);
}

// Run both LED to get Yellow warning error.
void iol_pl_faultLED(){
	if(alive_led > 0x08){
		alive_led--;
		if(alive_led == 0x10)
			alive_led = 0;
	}else{
		alive_led++;
		if(alive_led == 0x08)
			alive_led = 0x18;
	}
	
	timerled_setRedLED(alive_led & 0x0F);
	timerled_setGreenLED(alive_led & 0x0F);
}

// Poll reading from the L6364
void iol_pl_pollRead(){
	read_available = 
		phy_pHandler_t->phy_readFIFO(read_buffer_ptr);
	
	if(phy_pHandler_t->phy_getParityError()){
		read_available = 0;
	}
}

// Poll for write request
void iol_pl_pollWrite(){
	// Data write
	if(write_request){
		if(phy_pHandler_t->phy_writeFIFO(
				pl_tx_iol_byte,
				write_buffer_ptr) == 0)
    {
			// Reset write request after transfered all bytes
		  write_request = 0;
    }
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
// return 1 when done
uint8_t iol_pl_checkWriteStatus(){
	return !write_request;
}	

// ========= Setting M-Sequence Type ===============
// Return the current M-seq type according to number in enum 
uint8_t iol_pl_getCurrentMtype(){
	return current_mtype;
}

// Set the M-seq type to TYPE_0
void iol_pl_setMtype0(){
	phy_pHandler_t->phy_setMsequenceLength(
		2,
		1,
		1
	);
	current_mtype = MTYPE_0;
}

// Set the M-seq type to TYPE1_2
void iol_pl_setMtype1_2(){
	phy_pHandler_t->phy_setMsequenceLength(
		2,
		2,
		2
	);
	current_mtype = MTYPE_1_2;
}

// Set the M-seq type to TYPE2_2
void iol_pl_setMtype2_2(){
	phy_pHandler_t->phy_setMsequenceLength(
		2,
		1,
		1
	);
	current_mtype = MTYPE_2_2;
}

// Set the M-seq type to TYPE_2_V
// with 8 bytes PD in
void iol_pl_setMtype2_V_8PDI(){
	phy_pHandler_t->phy_setMsequenceLength(
		2,
		1,
		1
	);
	current_mtype = MTYPE_2_V_8PDI;
}

// ========= END Set M-Seq Type ====================