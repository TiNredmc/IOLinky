#include "iol_dl.h"

void iol_dl_T0WritePage(uint8_t address);
void iol_dl_T0ReadPage(uint8_t address);
void iol_dl_T0WriteISDU(uint8_t address);
void iol_dl_T0ReadISDU(uint8_t address);
void iol_dl_T0Xor();

// parsing M-sequence in STARTUP mode
void iol_dl_handleSTARTUP(){
	
	// Always make sure that we get TYPE_0 message
	if(iol_mt0.CKTBit.MT != 0){
		return;
	}
	
	
	if(iol_mt0.MCBit.RW){
		// Read mode
		switch(iol_mt0.MCBit.CC){
			case 0:// Process data
				break;
			case 1:// Page data
				iol_dl_T0ReadPage(iol_mt0.MCBit.ADDR);
				break;
			case 3:// ISDU data
				iol_dl_T0ReadISDU(iol_mt0.MCBit.ADDR);
				break;
			default:
				break;
		}
	}else{
		// Write mode
		switch(iol_mt0.MCBit.CC){
			case 0:// Process data
				break;
			case 1:// Page data
				iol_dl_T0WritePage(iol_mt0.MCBit.ADDR);
				break;
			case 3:// ISDU data
				iol_dl_T0WriteISDU(iol_mt0.MCBit.ADDR);
				break;
			default:
				break;
		}
	}
	
	iol_dl_T0Xor();
}

// Handle Master "page" write request (TYPE 0 message)
void iol_dl_T0WritePage(uint8_t address){
	
		// Store for internal use at DL
		switch(address){
			case 0x00:
				master_cmd = iol_mt0.OD;
				break;
			case 0x01:
				master_cycle = iol_mt0.OD;
				break;
			default:
				break;
		}	

		*(uint8_t *)(direct_param_ptr + (address & 0x1F)) = iol_mt0.OD;
		
		iol_mt0.CKS = 0x00;
		iol_pl_update_WriteBuffer(&iol_mt0.CKS);
		iol_pl_WriteRequest(1);// Return Type 0 message reply
}

// Handle Master "page" read request (TYPE 0 message)
void iol_dl_T0ReadPage(uint8_t address){	

	iol_mt0.OD = *(uint8_t *)(direct_param_ptr + (address & 0x1F));
	
	iol_mt0.CKS = 0x00;
	iol_pl_WriteRequest(2);// Return Type 0 message reply
}

// Handle Master "ISDU" write request (TYPE 0 message)
void iol_dl_T0WriteISDU(uint8_t address){

	if(address < 0x10){
		ISDU_in_buffer[ISDU_data_pointer] = iol_mt0.OD;
		
		if(iol_iservice.length == 1)
			ISDU_data_count = ISDU_in_buffer[1];// Grab the Extlength
		
		ISDU_data_pointer++;
		
		if(ISDU_data_count == ISDU_data_pointer){ 
			dl_isdu_fsm = 1;// enter decode state
			ISDU_data_pointer = 0;
		}
		
	}
	
	switch(address){
			case 0x10:// Get the ISDU I-service and kick FSM into decode state
				ISDU_in_buffer[0] = iol_mt0.OD;
				iol_iservice.Iservice = ISDU_in_buffer[0];
			
				if(iol_iservice.length != 1)// In case of no ExtLength, we can grab the data length directly
					ISDU_data_count = iol_iservice.length;
				
				ISDU_data_pointer = 1;// Reset data pointer to next one
			break;
		
		default:
			break;
	}
	
	iol_mt0.CKS = 0x00;
	iol_pl_update_WriteBuffer(&iol_mt0.CKS);
	iol_pl_WriteRequest(1);// Return Type 0 message reply
}

// Handle Master "ISDU" read request (TYPE 0 message)
void iol_dl_T0ReadISDU(uint8_t address){
	// Grab the M-seq count
	if(address < 0x10){
		iol_mt0.OD = ISDU_out_buffer[ISDU_data_pointer++];
		if(ISDU_data_pointer == ISDU_data_count){
			ISDU_data_pointer = 0;
			ISDU_data_count = 0;
			dl_isdu_fsm = 0;// Enter Idle state
		}
		
	}
	
	switch(address){
		case 0x10:// FlowCTRL START, send read respond I-service
			if(dl_isdu_fsm < 3)
				iol_mt0.OD = 0x01;// Reply busy response until we parsed ISDU
			else{
				iol_mt0.OD = ISDU_out_buffer[0];// reply I-service 
				ISDU_data_pointer = 1;// Start on next byte
			}
			break;
		
		case 0x11:// FlowCTRL IDLE1
			// Reply with OD = 0x00
			iol_mt0.OD = 0x00;
			dl_isdu_fsm = 0;// Went back to idle state
			break;
		
		case 0x1F:// FlowCTRL ABORT
			ISDU_data_pointer = 0;
			ISDU_data_count = 0;
			dl_isdu_fsm = 0;// Went back to idle state
			break;
		
		case 0x12:// FlowCTRL IDLE2 (reserved for future use, no use here for now...)
		default:// 0x13 to 0x1E is reserved
			break;
	}
	
	iol_mt0.CKS = 0x00;
	iol_pl_update_WriteBuffer(&iol_mt0.OD);
	iol_pl_WriteRequest(2);// Return Type 0 message reply
}

void iol_dl_T0Xor(){
	uint8_t temp_xor = 0;
	uint8_t temp_xor_odd = 0;
	uint8_t temp_xor_even = 0;
	uint8_t temp_xor_pair	= 0;
	
	// Calcualte the XOR check sum of Device's message
	// This include the Event and PD valid bit in CKS
	if(iol_mt0.MCBit.RW){
		temp_xor = iol_mt0.OD ^ 0x52;
		temp_xor ^= iol_mt0.CKS;
	}else{
		temp_xor = iol_mt0.CKS ^ 0x52;
	}
	// Packing Checksum8 to Checksum6
	
	iol_cks_t.CKS_B	= temp_xor;
	
	temp_xor_even = 
		iol_cks_t.CKS_0 ^
		iol_cks_t.CKS_2	^
		iol_cks_t.CKS_4	^
		iol_cks_t.CKS_6;
	
	temp_xor_odd =	
		iol_cks_t.CKS_1	^
		iol_cks_t.CKS_3	^
		iol_cks_t.CKS_5	^
		iol_cks_t.CKS_7;
	
	temp_xor_pair =
		(iol_cks_t.CKS_0 ^ iol_cks_t.CKS_1)					|
		((iol_cks_t.CKS_2 ^ iol_cks_t.CKS_3) 	<< 1)	|
		((iol_cks_t.CKS_4 ^ iol_cks_t.CKS_5)	<< 2)	|
		((iol_cks_t.CKS_6 ^ iol_cks_t.CKS_7)	<< 3);	
	
	iol_mt0.CKSBit.CKS = 
		(temp_xor_odd 	<< 5) |
		(temp_xor_even 	<< 4) |
		(temp_xor_pair 	& 0x0F);
}