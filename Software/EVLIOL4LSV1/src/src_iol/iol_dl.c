#include "iol_dl.h"

iol_mtype_0		iol_mt0;
#define MT0_LEN		4

iol_mtype_1_2 iol_mt1_2;
#define MT1_2_LEN	4

iol_isdu_Iserv iol_iservice;

uint8_t dl_main_fsm = 0;
uint8_t dl_startup_fsm = 0;
uint8_t dl_mode_fsm = 0;

uint8_t master_cmd = 0;
uint8_t master_cycle = 0;

void iol_dl_modeHandler();

void iol_dl_handleSTARTUP();
void iol_dl_T0WritePage(uint8_t address);
void iol_dl_T0ReadPage(uint8_t address);
void iol_dl_T0WriteISDU(uint8_t address);
void iol_dl_T0ReadISDU(uint8_t address);

void iol_dl_handlePREOPERATE();

void iol_dl_handleOPERATE();
void iol_dl_T12ReadISDU(uint8_t address);
void iol_dl_T12WriteISDU(uint8_t address);

void iol_dl_init(){
	iol_pl_init(
		(uint8_t *)&iol_mt0, 
		(uint8_t *)&iol_mt0.OD
	);
}

void iol_dl_poll(){
	switch(dl_main_fsm){
		case DL_MFSM_PR:// Wait for test message after wake up pulse
		{
			if(!iol_pl_ReadAvailable())
				break;

			// If read available. Read the data.
			iol_dl_modeHandler();
			
			dl_main_fsm = DL_MFSM_WFWR;
		}
		break;
		
		case DL_MFSM_WFWR:
		{ // Wait until reply (write) is done.
			if(iol_pl_checkWriteStatus()){
				GPIOB->ODR &= ~(1 << IOL_mon);
				
				// Make sure we are ready to reply on next cycle.
				switch(dl_mode_fsm){
					case DL_MODE_STARTUP:
					case DL_MODE_PREOP:
					{
						iol_pl_update_WriteBuffer(&iol_mt0.OD);
					}
					break;
					
					case DL_MODE_OP:
					{
						iol_pl_update_WriteBuffer(iol_mt1_2.OD);
					}
					break;
				}
				
				dl_main_fsm = DL_MFSM_PR;
			}
		}
		break;
	}
		
}

void iol_dl_modeHandler(){

	switch(dl_mode_fsm){
		case DL_MODE_STARTUP:
		{
			iol_dl_handleSTARTUP();
		}
		break;
		
		case DL_MODE_PREOP:
		{
			iol_dl_handlePREOPERATE();
		}
		break;
		
		case DL_MODE_OP:
		{
			iol_dl_handleOPERATE();
		}
		break;
	}
	
	switch(master_cmd){
		case 0x5A:// Fallback to SIO but nah ¯\_(o_0)_/¯
		case 0x97:// Switch to start up mode
		{
			iol_pl_setMtype0();
			iol_pl_updateBuffer(
				&iol_mt0.MC,
				&iol_mt0.OD
			);
			dl_mode_fsm = DL_MODE_STARTUP;
		}
		break;
		
		case 0x99:// Switch to Operate mode
		{
			iol_pl_setMtype1_2();
			iol_pl_updateBuffer(
				&iol_mt1_2.MC,
				iol_mt1_2.OD
			);
			dl_mode_fsm = DL_MODE_OP;
		}	
		break;
		
		case 0x9A:// Switch to Preoperate mode
		{
			iol_pl_setMtype0();
			iol_pl_updateBuffer(
				&iol_mt0.MC,
				&iol_mt0.OD
			);
			dl_mode_fsm = DL_MODE_PREOP;
		}
		break;
		
		default:
			break;
	}
	master_cmd = 0;// Clear master cmd for receive next cmd.
	
}

void iol_dl_handleSTARTUP(){
	
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
	
}

void iol_dl_T0WritePage(uint8_t address){
		switch(address){
			case 0x00:
				master_cmd = iol_mt0.OD;
				break;
			case 0x01:
				master_cycle = iol_mt0.OD;
				break;
			default:
				return;
		}	
		iol_pl_update_WriteBuffer(&iol_mt0.CKS);
		iol_mt0.CKS = 0x00;
		iol_pl_WriteRequest(1);// Return Type 0 message reply
}

void iol_dl_T0ReadPage(uint8_t address){
		switch(address){
		case 0x00:
			iol_mt0.OD = 0x00;
		break;
		
		case 0x01:
			iol_mt0.OD = 0x00;
		break;
		
		case 0x02:// Min cycle time
			iol_mt0.OD = (0 << 6) | (40);// 0.1ms * 40 = 4ms min cycle time.
		break;
		
		case 0x03:// M-sequence Capability
			iol_mt0.OD = 
				(0 << 4) |	// M-seq code type 0 for Preoperate	
				(1 << 1);		// M-seq code type 1_2 for Operate
		break;
		
		case 0x04:// IO-Link revision ID
			iol_mt0.OD = 0x11;
		break;
		
		case 0x05:// PD in properties
		case 0x06:// PD out	proterties
			iol_mt0.OD = 0x00;
		break;
		
		case 0x07:// Vendor ID MSB
		case 0x08:// Vendor ID LSB
			iol_mt0.OD = 0x69;
		break;
		
		case 0x09:// DevID MSB
			iol_mt0.OD = 0x69;
		break;
		
		case 0x0A:// DevID
			iol_mt0.OD = 0x04;
		break;
		
		case 0x0B:// DevID LSb
			iol_mt0.OD = 0x20;
		break;
		
		case 0x0C:// Function ID MSB
		case 0x0D:// Function ID LSB
			iol_mt0.OD = 0x00;
			break;
		
		case 0x0E:// Reserved
			iol_mt0.OD = 0x00;
			break;
		
		case 0x0F:// System command
			iol_mt0.OD = 0x00;
			break;
		
		default:
			iol_mt0.OD = 0x00;
			break;
	}
	
	iol_mt0.CKS = 0x00;
	
	iol_pl_WriteRequest(2);// Return Type 0 message reply
}

uint8_t ISDU_data_pointer = 0;
uint8_t ISDU_buffer[12] = {0};
void iol_dl_T0WriteISDU(uint8_t address){
	
	if(address < 0x10){
		ISDU_buffer[address] = iol_mt0.OD;
		goto reply_cks;
	}
	
	switch(address){
		case 0x10:// Get the ISDU I-service
				iol_iservice.Iservice = iol_mt0.OD;
			break;
		
		default:
			break;
	}

reply_cks:
	iol_pl_update_WriteBuffer(&iol_mt0.CKS);
	iol_mt0.CKS = 0x00;
	iol_pl_WriteRequest(1);// Return Type 0 message reply
}

uint8_t prev_FlowCTRL = 0;
uint8_t ISDU_FC_count = 0;
void iol_dl_T0ReadISDU(uint8_t address){
	if(prev_FlowCTRL == address){
		// No FlowCTRL Address change
		// NACK to let master resend
	}	

	// Grab the M-seq count
	if(address < 0x10){
		ISDU_FC_count = address;
	}
	
	switch(address){
		case 0x10:// FlowCTRL START, send read respond I-service
			iol_mt0.OD = 0xD0 | (1);// I-service is Read response(+) and the data length is 1 (arbitrary num for testing)
			break;
		
		case 0x11:// FlowCTRL IDLE1
			// Reply with OD = 0x00
			iol_mt0.OD = 0x00;
			break;
		
		case 0x1F:// FlowCTRL ABORT
			break;
		
		case 0x12:// FlowCTRL IDLE2 (reserved for future use, no use here for now...)
		default:// 0x13 to 0x1E is reserved
			break;
	}
	
	iol_pl_update_WriteBuffer(&iol_mt0.CKS);
	iol_mt0.CKS = 0x00;
	iol_pl_WriteRequest(2);// Return Type 0 message reply

exit_t0isdu_ja:
	prev_FlowCTRL = address;
}

void iol_dl_handlePREOPERATE(){
	
	if(iol_mt0.MCBit.RW){
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
		switch(iol_mt0.MCBit.CC){
			case 0:// Process data
				break;
			case 1:// Page data
				iol_dl_T0WritePage(iol_mt0.MCBit.ADDR);
				break;
			case 3:// ISDU data
				//iol_dl_T0WriteISDU(iol_mt0.MCBit.ADDR);
				break;
			default:
				break;
		}
	}
	
}

void iol_dl_handleOPERATE(){

	if(iol_mt1_2.MCBit.RW){
		// Read mode
		switch(iol_mt1_2.MCBit.CC){
			case 0:// Process data
				break;
			case 1:// Page data
				//iol_dl_T12ReadPage(iol_mt1_2.MCBit.ADDR);
				break;
			case 3:// ISDU data
				iol_dl_T12ReadISDU(iol_mt1_2.MCBit.ADDR);
				break;
			default:
				break;
		}
	}else{
		// Write mode
		switch(iol_mt1_2.MCBit.CC){
			case 0:// Process data
				break;
			case 1:// Page data
				//iol_dl_T0WritePage(iol_mt1_2.MCBit.ADDR);
				break;
			case 3:// ISDU data
				iol_dl_T0WriteISDU(iol_mt1_2.MCBit.ADDR);
				break;
			default:
				break;
		}
	}
	
}

void iol_dl_T12ReadISDU(uint8_t address){
		if(prev_FlowCTRL == address){
		// No FlowCTRL Address change
		// NACK to let master resend
	}	

	// Grab the M-seq count
	if(address < 0x10){
		ISDU_FC_count = address;
	}
	
	switch(address){
		case 0x10:// FlowCTRL START, send read respond I-service
			GPIOB->ODR |= (1 << IOL_mon);
			iol_mt1_2.OD[0] = 0xD0 | (1);// I-service is Read response(+) and the data length is 1 (Just for testing)
			iol_mt1_2.OD[1] = 0x00;
			break;
		
		case 0x11:// FlowCTRL IDLE1
			// Reply with OD = 0x00
			iol_mt1_2.OD[0] = 0x00;
			iol_mt1_2.OD[1] = 0x00;
			break;
		
		case 0x1F:// FlowCTRL ABORT
			break;
		
		case 0x12:// FlowCTRL IDLE2 (reserved for future use, no use here for now...)
		default:// 0x13 to 0x1E is reserved
			break;
	}
	
	iol_pl_update_WriteBuffer(&iol_mt1_2.CKS);
	iol_mt1_2.CKS = 0x00;
	iol_pl_WriteRequest(3);// Return Type 1_2 message reply

exit_t12isdu_ja:
	prev_FlowCTRL = address;
}

void iol_dl_T12WriteISDU(uint8_t address){

}

uint8_t iol_dl_getModeStatus(){
	return dl_mode_fsm;
}