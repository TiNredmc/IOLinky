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
	void *isdu_data_ptr;
	
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
	
	memset(ISDU_out_buffer, 0, 67);
	ISDU_out_buffer[0] = 0xD0;// Default Read Response (+) I-service;
	
	switch(isdu_idx){
		case 0x0010:// Vendor Name (64 Bytes) Mandatory!
		{
			ISDU_data_count = strlen(isdu_device_data_t->vendor_name);
			
			isdu_data_ptr = isdu_device_data_t->vendor_name;			
		}
		break;
		
		case 0x0011:// Vendor Text (64 Bytes)
		{
			ISDU_data_count = strlen(isdu_device_data_t->vendor_text);
			
			isdu_data_ptr = isdu_device_data_t->vendor_text;			
		}
		break;
		
		case 0x0012:// Product Name (64 Bytes) Mandatory!
		{
			ISDU_data_count = strlen(isdu_device_data_t->product_name);
			
			isdu_data_ptr = isdu_device_data_t->product_name;
		}
		break;
		
		case 0x0013:// Product ID (64 Bytes)
		{
			ISDU_data_count = strlen(isdu_device_data_t->product_id);
			
			isdu_data_ptr = isdu_device_data_t->product_id;
		}
		break;
		
		case 0x0014:// Product Text (64 Bytes)
		{
			ISDU_data_count = strlen(isdu_device_data_t->product_text);
			
			isdu_data_ptr = isdu_device_data_t->product_text;			
		}
		break;
		
		case 0x0015:// Serial Number (16 Bytes) Mandatory!
		{
			ISDU_data_count = strlen(isdu_device_data_t->serial_number);
			
			isdu_data_ptr = isdu_device_data_t->serial_number;
		}
		break;
		
		case 0x0016:// Hardware Revision 
		{
			ISDU_data_count = strlen(isdu_device_data_t->hardware_revision);
			
			isdu_data_ptr = isdu_device_data_t->hardware_revision;
		}
		break;
		
		case 0x0017:// Firmware Revision 
		{
			ISDU_data_count = strlen(isdu_device_data_t->firmware_revision);
			
			isdu_data_ptr = isdu_device_data_t->firmware_revision;
		}
		break;
		
		case 0x0028:// Process Data input
		{
			ISDU_data_count = 1 + 8 + 1;// I-service + PD + checksum
			ISDU_out_buffer[0] |= ISDU_data_count;// Length
			ISDU_out_buffer[8] = *pdIn_ptr;
			ISDU_out_buffer[7] = *(pdIn_ptr+1);
			ISDU_out_buffer[6] = *(pdIn_ptr+2);
			ISDU_out_buffer[5] = *(pdIn_ptr+3);
			ISDU_out_buffer[4] = *(pdIn_ptr+4);
			ISDU_out_buffer[3] = *(pdIn_ptr+5);
			ISDU_out_buffer[2] = *(pdIn_ptr+6);
			ISDU_out_buffer[1] = *(pdIn_ptr+7);
			
			goto isdu_exit;
		}
		break;
		
		case 0x00FD:// I2t Efuse integreator value
		{
			ISDU_data_count = 1 + 4 + 1;// I-service + Efuse data + checksum
			int32_t efuse_temp = iol_al_getEfuseCount();
			
			ISDU_out_buffer[0] |= ISDU_data_count;
			ISDU_out_buffer[1] = 
				*(((uint8_t *)&efuse_temp) + 3);
			ISDU_out_buffer[2] = 
				*(((uint8_t *)&efuse_temp) + 2);
			ISDU_out_buffer[3] = 
				*(((uint8_t *)&efuse_temp) + 1);
			ISDU_out_buffer[4] = 
				*(((uint8_t *)&efuse_temp));
			
			goto isdu_exit;
		}
		
		case 0x00FE:// PSU FSM state
		{
			ISDU_data_count = 1 + 1 + 1;// I-service + data + checksum
			ISDU_out_buffer[0] |= ISDU_data_count;
			ISDU_out_buffer[1] = iol_al_getPSUState();
			
			goto isdu_exit;
		}
			
		default:
			return;		
	}
	
	if(ISDU_data_count < 16){
		memcpy(
			ISDU_out_buffer+1, 
			isdu_data_ptr, 
			ISDU_data_count);

		ISDU_data_count += 2;// I-service + checksum;

		ISDU_out_buffer[0] |= ISDU_data_count;// Length


	}else{// Require Extlength if ISDU data is longer than 15 bytes
		memcpy(
			ISDU_out_buffer+2, 
			isdu_data_ptr, 
			ISDU_data_count);

		ISDU_data_count += 3;// I-service + ExtLength + checksum

		ISDU_out_buffer[0] |= 1;// Length
		ISDU_out_buffer[1] = ISDU_data_count;

	}
	
isdu_exit:
	iol_dl_xorISDU(ISDU_data_count);
	
}

void iol_dl_craftISDUWrite(){
	uint16_t isdu_idx = 0;
	uint8_t isdu_write_amount = 0;
	
	
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
	
	if(iol_iservice.length == 1){
		isdu_write_amount = iol_iservice.ExtLength;
	}else{
		isdu_write_amount = iol_iservice.length;
	}
	
	ISDU_data_count = 2;
	ISDU_out_buffer[0] = 0x50 | 2;// Default write Response (+) I-service;
	
	switch(isdu_idx){
		case 0x0002:// System command
		{
			system_cmd = ISDU_in_buffer[ISDU_data_offset_index];
			ISDU_data_offset_index = 0;
		}
		break;
		
		default:
			// Preferred Index (used for vendor specific funciton)
			if(
				(isdu_idx > 0x0039) &&
				(isdu_idx < 0x00FF)
			){
			
			}
		
		
			break;
	}
	
	
	
	// Calculate checksum
	ISDU_out_buffer[1] = 0x00;
	ISDU_out_buffer[1] = ISDU_out_buffer[0] ^ ISDU_out_buffer[1];
}