#include "iol_dl.h"

void iol_dl_craftISDURead();
void iol_dl_craftISDUWrite();

void iol_dl_ISDUFSM(){

	switch(dl_isdu_fsm){
		case 0:// Idle case
		{
			
		}
		break;
		
		case 1:// Parse I-service
		{
			switch(iol_iservice.Iserv){
				case 0x00:// No serivce
					dl_isdu_fsm = 0;
					break;
				
				case 0x01:// Write 8 bit Index
					if(iol_iservice.length == 1){
						iol_iservice.ExtLength = ISDU_in_buffer[1];
						iol_iservice.Index[0] = ISDU_in_buffer[2];
						ISDU_data_offset_index = 3;
					}else{
						iol_iservice.Index[0] = ISDU_in_buffer[1];
						ISDU_data_offset_index = 2;
					}	
					
					ISDU_16bIndex = 0;
					ISDU_RWflag = 0;
					break;
				
				case 0x02:// Write 8 bit Index and Subindex
					if(iol_iservice.length == 1){
						iol_iservice.ExtLength = ISDU_in_buffer[1];
						iol_iservice.Index[0] = ISDU_in_buffer[2];
						iol_iservice.Subindex = ISDU_in_buffer[3];
						ISDU_data_offset_index = 4;
					}else{
						iol_iservice.Index[0] = ISDU_in_buffer[1];
						iol_iservice.Subindex = ISDU_in_buffer[2];
						ISDU_data_offset_index = 3;
					}	
					ISDU_16bIndex = 0;
					ISDU_RWflag = 0;
					break;
				
				case 0x03:// Write 16 bit Index and Subindex
					if(iol_iservice.length == 1){
						iol_iservice.ExtLength = ISDU_in_buffer[1];
						iol_iservice.Index[0] = ISDU_in_buffer[3];
						iol_iservice.Index[1] = ISDU_in_buffer[2];
						iol_iservice.Subindex = ISDU_in_buffer[4];
						ISDU_data_offset_index = 5;
					}else{
						iol_iservice.Index[0] = ISDU_in_buffer[2];
						iol_iservice.Index[1] = ISDU_in_buffer[1];
						iol_iservice.Subindex = ISDU_in_buffer[3];
						ISDU_data_offset_index = 4;
					}		
					ISDU_16bIndex = 1;
					ISDU_RWflag = 0;
					break;
				
				//case 0x04:// Write Response (-)
				//	break;
				
				//case 0x05:// Write Response (+)
				//	break;
				
				case 0x09:// Read 8 bit Index
					iol_iservice.Index[0] = ISDU_in_buffer[1];
					ISDU_16bIndex = 0;
					ISDU_RWflag = 1;
					break;
				
				case 0x0A:// Read 8 bit Index and Subindex
					iol_iservice.Index[0] = ISDU_in_buffer[1];
					iol_iservice.Subindex = ISDU_in_buffer[2];
					ISDU_16bIndex = 0;
					ISDU_RWflag = 1;
					break;
				
				case 0x0B:// Read 16 bit Index and Subindex
					iol_iservice.Index[0] = ISDU_in_buffer[1];
					iol_iservice.Index[1] = ISDU_in_buffer[2];
					iol_iservice.Subindex = ISDU_in_buffer[3];
					ISDU_16bIndex = 1;
					ISDU_RWflag = 1;
					break;
				
				//case 0x0C:// Read Response (-)
				//	break;
				
				//case 0x0D:// Read Response (+)
				//	break;
				
				default:
					dl_isdu_fsm = 0;
					return;
			}
			
			dl_isdu_fsm = 2;// craft the response
		}
		break;
		
		case 2:// craft the response data
		{
			memset(ISDU_out_buffer, 0, 67);
			if(ISDU_RWflag == 1){
				// Read request
				iol_dl_craftISDURead();
			}else{
				// Write request
				iol_dl_craftISDUWrite();
			}
			
			dl_isdu_fsm = 3;
		}
		break;
		
		case 3:// At this state ISDU data is being exchanged
		{
		
		}
		break;
		
	}
}

void iol_dl_xorISDU(uint16_t Len){
	uint8_t temp_xor = 0;
	uint16_t len_cnt = 0;
	
	temp_xor = ISDU_out_buffer[0];
	len_cnt++;
	Len--;
	while(Len--){
		temp_xor ^= ISDU_out_buffer[len_cnt];
		len_cnt++;
	}
	
	ISDU_out_buffer[len_cnt - 1] = temp_xor;
}

void iol_dl_craftISDURead(){
	uint16_t isdu_idx = 0;
	
	// Parse Index
	if(ISDU_16bIndex == 1){
		// 16 Bit Index
		isdu_idx = (uint16_t)(
			(iol_iservice.Index[0] << 8) |
			iol_iservice.Index[1]
		);
	}else{
		// 8 bit index
		isdu_idx = iol_iservice.Index[0];
	}
	
	ISDU_out_buffer[0] = 0xD0;// Default Read Response (+) I-service;
	
	ISDU_data_count = 
		isdu_pHandler_t->isdu_handleRead(
			isdu_idx,
			(uint8_t *)&ISDU_out_buffer
		);
	
isdu_exit:
	iol_dl_xorISDU(ISDU_data_count);
	
}

void iol_dl_craftISDUWrite(){
	uint16_t isdu_idx = 0;
	uint8_t isdu_write_amount = 0;
	uint16_t isdu_error_ret = 0;
	
	// Parse Index
	if(ISDU_16bIndex == 1){
		// 16 Bit Index
		isdu_idx = (uint16_t)(
			(iol_iservice.Index[1] << 8) |
			iol_iservice.Index[0]
		);
	}else{
		// 8 bit index
		isdu_idx = iol_iservice.Index[0];
	}
	
	if(iol_iservice.length == 1){
		isdu_write_amount = iol_iservice.ExtLength;
	}else{
		isdu_write_amount = iol_iservice.length;
	}
	
	ISDU_data_count = 2;
	ISDU_out_buffer[0] = 0x50 | 2;// Default write Response (+) I-service;
	
	isdu_error_ret = 
		isdu_pHandler_t->isdu_handleWrite(
			isdu_idx,
			(uint8_t *)&ISDU_in_buffer,
			isdu_write_amount,
			ISDU_data_offset_index
		);
	
	if(
		isdu_error_ret != 0
	){
		// If error occur, send error code
		ISDU_data_count = 4;
		ISDU_out_buffer[0] = 0x40 | 4;// data count including error
		ISDU_out_buffer[1] = (uint8_t)(isdu_error_ret >> 8);
		ISDU_out_buffer[2] = (uint8_t)isdu_error_ret;
	
	}

	ISDU_data_offset_index = 0;
	
	// Calculate checksum
	iol_dl_xorISDU(ISDU_data_count);
}