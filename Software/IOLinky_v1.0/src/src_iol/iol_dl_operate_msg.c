#include "iol_dl.h"

void iol_dl_updatePDData();
void iol_dl_T2VWritePage(uint8_t address);
void iol_dl_T2VReadPage(uint8_t address);
void iol_dl_T2VWriteISDU(uint8_t address);
void iol_dl_T2VReadISDU(uint8_t address);
void iol_dl_T2VXor();

// parsing M-sequence in OPERATE mode
void iol_dl_handleOPERATE(){
	
	// Always make sure that we get TYPE_2 message
	if(iol_mt2_v.CKTBit.MT != 2){
		return;
	}
	
	if(iol_mt2_v.MCBit.RW){
		// Read mode
		switch(iol_mt2_v.MCBit.CC){
			case 1:// Page data
				iol_dl_T2VReadPage(iol_mt2_v.MCBit.ADDR);
				break;
			case 3:// ISDU data
				iol_dl_T2VReadISDU(iol_mt2_v.MCBit.ADDR);
				break;
			default:
				dl_main_fsm = DL_MFSM_PR;
				break;
		}
			
	}else{
		// Write mode
		switch(iol_mt2_v.MCBit.CC){
			case 1:// Page data
				iol_dl_T2VWritePage(iol_mt2_v.MCBit.ADDR);
				break;
			case 3:// ISDU data
				iol_dl_T2VWriteISDU(iol_mt2_v.MCBit.ADDR);
				break;
			default:
				dl_main_fsm = DL_MFSM_PR;
				break;
		}
	}

	if(PD_setFlag == 1){
		PD_setFlag = 0;
		iol_mt2_v.PD[7] = *pdIn_ptr;
		iol_mt2_v.PD[6] = *(pdIn_ptr+1);
		iol_mt2_v.PD[5] = *(pdIn_ptr+2);
		iol_mt2_v.PD[4] = *(pdIn_ptr+3);
		iol_mt2_v.PD[3] = *(pdIn_ptr+4);
		iol_mt2_v.PD[2] = *(pdIn_ptr+5);
		iol_mt2_v.PD[1] = *(pdIn_ptr+6);
		iol_mt2_v.PD[0] = *(pdIn_ptr+7);
	}
	
	iol_dl_T2VXor();
}

// Handle Master "page" write request (TYPE 2_2 message)
void iol_dl_T2VWritePage(uint8_t address){
		switch(address){
			case 0x00:
				master_cmd = iol_mt2_v.OD;
				break;
			case 0x01:
				master_cycle = iol_mt2_v.OD;
				break;
			default:
				return;
		}	
		
		*(uint8_t *)(direct_param_ptr + (address & 0x0F)) = iol_mt2_v.OD;
		
		iol_mt2_v.CKS = 0x00;
		iol_pl_update_WriteBuffer(iol_mt2_v.PD);
		iol_pl_WriteRequest(8 + 1);// Return Type 2_V message reply
}

// Handle Master "page" read request (TYPE 2_V message)
void iol_dl_T2VReadPage(uint8_t address){
	
	if(address < 0x10)
		iol_mt2_v.OD = *(uint8_t *)(direct_param_ptr + (address & 0x0F));
	
	iol_mt2_v.CKS = 0x00;
	iol_pl_update_WriteBuffer(&iol_mt2_v.OD);
	iol_pl_WriteRequest(1 + 8 + 1);// Return Type 2_V message reply
}

// Handle Master "ISDU" write request (TYPE 2_v message)
void iol_dl_T2VWriteISDU(uint8_t address){
	
	if(address < 0x10){
		ISDU_in_buffer[ISDU_data_pointer] = iol_mt2_v.OD;
		
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
				ISDU_in_buffer[0] = iol_mt2_v.OD;
				iol_iservice.Iservice = ISDU_in_buffer[0];
			
				if(iol_iservice.length != 1)// In case of no ExtLength, we can grab the data length directly
					ISDU_data_count = iol_iservice.length;
				
				ISDU_data_pointer = 1;// Reset data pointer to next one
			break;
		
		default:
			break;
	}

	iol_mt2_v.CKS = 0x00;
	iol_pl_update_WriteBuffer(iol_mt2_v.PD);
	iol_pl_WriteRequest(8 + 1);// Return Type 2_v message reply
}

// Handle Master "ISDU" read request (TYPE 2_v message)
void iol_dl_T2VReadISDU(uint8_t address){
	// Grab the M-seq count
	if(address < 0x10){
		iol_mt2_v.OD = ISDU_out_buffer[ISDU_data_pointer++];
		if(ISDU_data_pointer == ISDU_data_count){
			ISDU_data_pointer = 0;
			dl_isdu_fsm = 0;// Enter Idle state
		}
	}
	
	switch(address){
		case 0x10:// FlowCTRL START, send read respond I-service
			if(dl_isdu_fsm < 3)
				iol_mt2_v.OD = 0x01;// Reply busy response until we parsed ISDU
			else{
				iol_mt2_v.OD = ISDU_out_buffer[0];// reply I-service 
				ISDU_data_pointer = 1;// Start on next byte
			}
			break;
		
		case 0x11:// FlowCTRL IDLE1
		case 0x12:// FlowCTRL IDLE2 (reserved for future use, no use here for now...)	
			// Reply with OD = 0x00
			iol_mt2_v.OD = 0x00;
			dl_isdu_fsm = 0;// Went back to idle state
			break;
		
		case 0x1F:// FlowCTRL ABORT
			ISDU_data_pointer = 0;
			ISDU_data_count = 0;
			dl_isdu_fsm = 0;// Went back to idle state
			break;
		
		default:// 0x13 to 0x1E is reserved
			break;
	}
	
	iol_mt2_v.CKS = 0x00;
	iol_pl_update_WriteBuffer(&iol_mt2_v.OD);
	iol_pl_WriteRequest(1 + 8 + 1);// Return Type 1_2 message reply
}

void iol_dl_T2VXor(){
	uint8_t temp_xor = 0;
	uint8_t temp_xor_odd = 0;
	uint8_t temp_xor_even = 0;
	uint8_t temp_xor_pair	= 0;
	
	// Calcualte the XOR check sum of Device's message
	// This include the Event and PD valid bit in CKS
	if(iol_mt2_v.MCBit.RW){
		temp_xor = iol_mt2_v.OD ^ 0x52;
		temp_xor ^= iol_mt2_v.PD[0];
		
	}else{
		temp_xor = iol_mt2_v.PD[0] ^ 0x52;
	}
	
	temp_xor ^= iol_mt2_v.PD[1];
	temp_xor ^= iol_mt2_v.PD[2];
	temp_xor ^= iol_mt2_v.PD[3];
	temp_xor ^= iol_mt2_v.PD[4];
	temp_xor ^= iol_mt2_v.PD[5];
	temp_xor ^= iol_mt2_v.PD[6];
	temp_xor ^= iol_mt2_v.PD[7];
	temp_xor ^= iol_mt2_v.CKS;
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
	
	iol_mt2_v.CKSBit.CKS = 
		(temp_xor_odd 	<< 5) |
		(temp_xor_even 	<< 4) |
		(temp_xor_pair 	& 0x0F);
}