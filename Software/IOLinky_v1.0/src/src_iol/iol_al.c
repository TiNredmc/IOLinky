#include "iol_al.h"

#define UNIQUEID_BASE	0x1FFFF7AC

// Private typedef
isdu_data_t *iol_al_isdu_d_t;

isdu_handler_t iol_isdu_RWHandler_t = {
	.isdu_handleRead 	= iol_al_handleISDURead,
	.isdu_handleWrite = iol_al_handleISDUWrite
};

// Initialize the underlying layer (AL->DL->PL)
void iol_al_init(
	device_directparam_t *d_dp_t,
	isdu_data_t *isdu_dat_t,
	uint8_t *PD_data_ptr	
	){
	
	iol_al_isdu_d_t	= isdu_dat_t;
		
	iol_dl_init(
		(uint8_t *)d_dp_t,	
		&iol_isdu_RWHandler_t,
		iol_al_isdu_d_t,
		(uint8_t *)PD_data_ptr,
		8			// 8 octets for TYPE2_V with 8 PD in
	);
}

// Poll to run the IO-Link communication
// called in app_device_impl.c
void iol_al_poll(){
	iol_dl_poll();
}

// Update Process Data.
void iol_al_updatePD(){
	iol_dl_updatePD();
}

void iol_al_AppReportEvent(
	uint16_t eventCode
	){
	uint8_t eventType = 0;
		
	// Standard IO-Link Event code
	// Only implemented if needed
	switch(eventCode){
		case 0x0000:// No malfunction
			eventType = EVT_NOTIFICATION;
			break;
		case 0x1000:// General mulfunction 
		case 0x4000:// Temperature fault
		case 0x5000:// Device hardware fault
		case 0x5010:// Component malfunction
		case 0x5011:// NV memory loss
		case 0x5100:// General PSU error
		case 0x5101:// Fuse blown
		case 0x6000:// Device Software fault
		case 0x6320:// Parameter error
		case 0x6321:// Parameter missing
			eventType = EVT_ERROR;
			break;
		case 0x5110:// Primary supply olvervolt 
		case 0x5111:// Primary supply undervolt
		case 0x5112:// Sec supply fault
		case 0x8C10:// Process data range overrun
		case 0x8C30:// Process data underrun
			eventType = EVT_WARNING;
			break;
	}
	
	iol_dl_pushEvt(
		eventType,
		eventCode
	);
}

uint16_t iol_al_handleISDURead(
	uint16_t index,
	uint8_t *isdu_out_buffer
	){
	void *isdu_data_ptr;
	uint16_t ISDU_data_count = 0;	
	
	switch(index){
		case 0x0010:// Vendor Name (64 Bytes) Mandatory!
		{
			ISDU_data_count = strlen(iol_al_isdu_d_t->vendor_name);
			
			isdu_data_ptr = iol_al_isdu_d_t->vendor_name;			
		}
		break;
		
		case 0x0011:// Vendor Text (64 Bytes)
		{
			ISDU_data_count = strlen(iol_al_isdu_d_t->vendor_text);
			
			isdu_data_ptr = iol_al_isdu_d_t->vendor_text;			
		}
		break;
		
		case 0x0012:// Product Name (64 Bytes) Mandatory!
		{
			ISDU_data_count = strlen(iol_al_isdu_d_t->product_name);
			
			isdu_data_ptr = iol_al_isdu_d_t->product_name;
		}
		break;
		
		case 0x0013:// Product ID (64 Bytes)
		{
			ISDU_data_count = strlen(iol_al_isdu_d_t->product_id);
			
			isdu_data_ptr = iol_al_isdu_d_t->product_id;
		}
		break;
		
		case 0x0014:// Product Text (64 Bytes)
		{
			ISDU_data_count = strlen(iol_al_isdu_d_t->product_text);
			
			isdu_data_ptr = iol_al_isdu_d_t->product_text;			
		}
		break;
		
		case 0x0015:// Serial Number (16 Bytes) Mandatory!
		{
			ISDU_data_count = strlen(iol_al_isdu_d_t->serial_number);
			
			isdu_data_ptr = iol_al_isdu_d_t->serial_number;
		}
		break;
		
		case 0x0016:// Hardware Revision 
		{
			ISDU_data_count = strlen(iol_al_isdu_d_t->hardware_revision);
			
			isdu_data_ptr = iol_al_isdu_d_t->hardware_revision;
		}
		break;
		
		case 0x0017:// Firmware Revision 
		{
			ISDU_data_count = strlen(iol_al_isdu_d_t->firmware_revision);
			
			isdu_data_ptr = iol_al_isdu_d_t->firmware_revision;
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
			
			goto isdu_handle_exit;
		}
		break;
		
		case 0x00FD:// I2t Efuse integreator value
		{
			ISDU_data_count = 1 + 4 + 1;// I-service + Efuse data + checksum
			int32_t efuse_temp = app_mon_getEfuse();
			
			ISDU_out_buffer[0] |= ISDU_data_count;
			ISDU_out_buffer[1] = 
				*(((uint8_t *)&efuse_temp) + 3);
			ISDU_out_buffer[2] = 
				*(((uint8_t *)&efuse_temp) + 2);
			ISDU_out_buffer[3] = 
				*(((uint8_t *)&efuse_temp) + 1);
			ISDU_out_buffer[4] = 
				*(((uint8_t *)&efuse_temp));
			
			goto isdu_handle_exit;
		}
		
		case 0x00FE:// PSU FSM state
		{
			ISDU_data_count = 1 + 1 + 1;// I-service + data + checksum
			ISDU_out_buffer[0] |= ISDU_data_count;
			ISDU_out_buffer[1] = app_psu_status();
			
			goto isdu_handle_exit;
		}
			
		case 0x0040:// PSU Power switch status
		{
			ISDU_data_count = 1 + 1 + 1;// I-service + data + checksum
			ISDU_out_buffer[0] |= ISDU_data_count;
			ISDU_out_buffer[1] = app_psu_getPWRStatus();
			
			goto isdu_handle_exit;			
		}
		break;
		
		case 0x0041:// Efuse holding current (milliamps)
		{
			ISDU_data_count = 1 + 4 + 1;// I-service + data + checksum
			ISDU_out_buffer[0] |= ISDU_data_count;
			ISDU_out_buffer[1] = (uint8_t)(FLASH_DATA->I2T_AmpHold >> 24);
			ISDU_out_buffer[2] = (uint8_t)(FLASH_DATA->I2T_AmpHold >> 16);
			ISDU_out_buffer[3] = (uint8_t)(FLASH_DATA->I2T_AmpHold >> 8);
			ISDU_out_buffer[4] = (uint8_t)FLASH_DATA->I2T_AmpHold;
			goto isdu_handle_exit;	
		}
		break;
		
		case 0x0042:// Efuse holding period (milliseconds)
		{
			ISDU_data_count = 1 + 4 + 1;// I-service + data + checksum
			ISDU_out_buffer[0] |= ISDU_data_count;
			ISDU_out_buffer[1] = (uint8_t)(FLASH_DATA->I2T_PeriodHold >> 24);
			ISDU_out_buffer[2] = (uint8_t)(FLASH_DATA->I2T_PeriodHold >> 16);
			ISDU_out_buffer[3] = (uint8_t)(FLASH_DATA->I2T_PeriodHold >> 8);
			ISDU_out_buffer[4] = (uint8_t)FLASH_DATA->I2T_PeriodHold;
			goto isdu_handle_exit;	
		}
		break;
		
		// Super duper secret index for calibration
		
//		case 0x0100:// Calibration unlock password
//		{

//		}
//		break;
		
		case 0x0101:// Vin scaling calibration value
		{
			ISDU_data_count = 1 + 4 + 1;// I-service + data + checksum
			ISDU_out_buffer[0] |= ISDU_data_count;
			ISDU_out_buffer[1] = (uint8_t)(FLASH_DATA->ADC5.ADC_SCALING >> 24);
			ISDU_out_buffer[2] = (uint8_t)(FLASH_DATA->ADC5.ADC_SCALING >> 16);
			ISDU_out_buffer[3] = (uint8_t)(FLASH_DATA->ADC5.ADC_SCALING >> 8);
			ISDU_out_buffer[4] = (uint8_t)FLASH_DATA->ADC5.ADC_SCALING;
			goto isdu_handle_exit;	
		}
		break;
		
		case 0x0102:// Vin offset calibration value
		{
			ISDU_data_count = 1 + 4 + 1;// I-service + data + checksum
			ISDU_out_buffer[0] |= ISDU_data_count;
			ISDU_out_buffer[1] = (uint8_t)(FLASH_DATA->ADC5.ADC_OFFSET >> 24);
			ISDU_out_buffer[2] = (uint8_t)(FLASH_DATA->ADC5.ADC_OFFSET >> 16);
			ISDU_out_buffer[3] = (uint8_t)(FLASH_DATA->ADC5.ADC_OFFSET >> 8);
			ISDU_out_buffer[4] = (uint8_t)FLASH_DATA->ADC5.ADC_OFFSET;
			goto isdu_handle_exit;	
		}
		break;
		
		case 0x0103:// Vout scaling calibration value
		{
			ISDU_data_count = 1 + 4 + 1;// I-service + data + checksum
			ISDU_out_buffer[0] |= ISDU_data_count;
			ISDU_out_buffer[1] = (uint8_t)(FLASH_DATA->ADC6.ADC_SCALING >> 24);
			ISDU_out_buffer[2] = (uint8_t)(FLASH_DATA->ADC6.ADC_SCALING >> 16);
			ISDU_out_buffer[3] = (uint8_t)(FLASH_DATA->ADC6.ADC_SCALING >> 8);
			ISDU_out_buffer[4] = (uint8_t)FLASH_DATA->ADC6.ADC_SCALING;
			goto isdu_handle_exit;	
		}
		break;
		
		case 0x0104:// Vout offset calibration value
		{
			ISDU_data_count = 1 + 4 + 1;// I-service + data + checksum
			ISDU_out_buffer[0] |= ISDU_data_count;
			ISDU_out_buffer[1] = (uint8_t)(FLASH_DATA->ADC6.ADC_OFFSET >> 24);
			ISDU_out_buffer[2] = (uint8_t)(FLASH_DATA->ADC6.ADC_OFFSET >> 16);
			ISDU_out_buffer[3] = (uint8_t)(FLASH_DATA->ADC6.ADC_OFFSET >> 8);
			ISDU_out_buffer[4] = (uint8_t)FLASH_DATA->ADC6.ADC_OFFSET;
			goto isdu_handle_exit;	
		}
		break;
		
		case 0x0105:// Iout scaling calibration value
		{
			ISDU_data_count = 1 + 4 + 1;// I-service + data + checksum
			ISDU_out_buffer[0] |= ISDU_data_count;
			ISDU_out_buffer[1] = (uint8_t)(FLASH_DATA->ADC7.ADC_SCALING >> 24);
			ISDU_out_buffer[2] = (uint8_t)(FLASH_DATA->ADC7.ADC_SCALING >> 16);
			ISDU_out_buffer[3] = (uint8_t)(FLASH_DATA->ADC7.ADC_SCALING >> 8);
			ISDU_out_buffer[4] = (uint8_t)FLASH_DATA->ADC7.ADC_SCALING;
			goto isdu_handle_exit;	
		}
		break;
		
		case 0x0106:// Iout offset calibration value
		{
			ISDU_data_count = 1 + 4 + 1;// I-service + data + checksum
			ISDU_out_buffer[0] |= ISDU_data_count;
			ISDU_out_buffer[1] = (uint8_t)(FLASH_DATA->ADC7.ADC_OFFSET >> 24);
			ISDU_out_buffer[2] = (uint8_t)(FLASH_DATA->ADC7.ADC_OFFSET >> 16);
			ISDU_out_buffer[3] = (uint8_t)(FLASH_DATA->ADC7.ADC_OFFSET >> 8);
			ISDU_out_buffer[4] = (uint8_t)FLASH_DATA->ADC7.ADC_OFFSET;
			goto isdu_handle_exit;	
		}
		break;
		
		default:// Read other not supported index will return access denied error
		{
			ISDU_data_count = 1 + 2 + 1;// I-service + error code + checksum
			ISDU_out_buffer[0] = 0xC0 | 4; 
			ISDU_out_buffer[1] = 0x23;
			ISDU_out_buffer[2] = 0x80;
			goto isdu_handle_exit;	
		}
		break;
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
	
isdu_handle_exit:		
	return ISDU_data_count;
}

uint16_t iol_al_handleISDUWrite(
	uint16_t index,
	uint8_t *isdu_in_buffer,
	uint16_t length,
	uint16_t offset
	){
	uint32_t temp_data = 0;
		
	switch(index){
		case 0x0002:// System command
		{
			//system_cmd = isdu_in_buffer[ISDU_data_offset_index];
			
		}
		break;
		
		case 0x0040:// PSU Power switch
		{
			if(isdu_in_buffer[offset] == 0x00){
				// Disable PSU
				app_psu_requestPWROFF();
			}else{
				// Enable PSU
				app_psu_requestPWRON();
			}
			
		}
		break;
		
		case 0x0041:// Efuse holding current (milliamps)
		{
			if(!app_nvm_getAccessUnlock())
				return 0x8023;
			
			temp_data =
				isdu_in_buffer[offset + 3] 				|
				(isdu_in_buffer[offset + 2] << 8)	|
				(isdu_in_buffer[offset + 1] << 16)|
				(isdu_in_buffer[offset] << 24)
			;
			
			app_nvm_setI2TAmpHold((int32_t)temp_data);
		}
		break;
		
		case 0x0042:// Efuse holding period (milliseconds)
		{
			if(!app_nvm_getAccessUnlock())
				return 0x8023;
			
			temp_data =
				isdu_in_buffer[offset + 3] 				|
				(isdu_in_buffer[offset + 2] << 8)	|
				(isdu_in_buffer[offset + 1] << 16)|
				(isdu_in_buffer[offset] << 24)
			;
			
			app_nvm_setI2TPeriodHold((int32_t)temp_data);
		}
		break;
		
		// Super duper secret index for calibration
		
		case 0x0100:// Calibration unlock password
		{
			temp_data =
				isdu_in_buffer[offset + 3] 				|
				(isdu_in_buffer[offset + 2] << 8)	|
				(isdu_in_buffer[offset + 1] << 16)|
				(isdu_in_buffer[offset] << 24)
			;
			
			// Check for incorrect password
			switch(app_nvm_setAccessUnlock(temp_data)){
				case 254:// Invalid password
					// Return Invalid parameter set (PAR_SETINVALID)
					return 0x8040;
				
				case 1:// Erase flash error
				case 2:// Write flash error
				case 3:// Verify flash error
					iol_al_AppReportEvent(0x5000);
					return 0x8082;// Application not ready
				
				default:// Update Efuse
					app_mon_updateEfuseThreshold();
					break;
			}
		}
		break;
		
		case 0x0101:// Vin scaling calibration value
		{
			if(!app_nvm_getAccessUnlock())
				return 0x8023;
			
			temp_data =
				isdu_in_buffer[offset + 3] 				|
				(isdu_in_buffer[offset + 2] << 8)	|
				(isdu_in_buffer[offset + 1] << 16)|
				(isdu_in_buffer[offset] << 24)
			;
			
			app_nvm_setADC5Scaling((int32_t)temp_data);
			 
		}
		break;
		
		case 0x0102:// Vin offset calibration value
		{
			if(!app_nvm_getAccessUnlock())
				return 0x8023;
			
			temp_data =
				isdu_in_buffer[offset + 3] 				|
				(isdu_in_buffer[offset + 2] << 8)	|
				(isdu_in_buffer[offset + 1] << 16)|
				(isdu_in_buffer[offset] << 24)
			;
			
			app_nvm_setADC5Offset((int32_t)temp_data);
		}
		break;
		
		case 0x0103:// Vout scaling calibration value
		{
			if(!app_nvm_getAccessUnlock())
				return 0x8023;
			
			temp_data =
				isdu_in_buffer[offset + 3] 				|
				(isdu_in_buffer[offset + 2] << 8)	|
				(isdu_in_buffer[offset + 1] << 16)|
				(isdu_in_buffer[offset] << 24)
			;
			
			app_nvm_setADC6Scaling((int32_t)temp_data);
		}
		break;
		
		case 0x0104:// Vout offset calibration value
		{
			if(!app_nvm_getAccessUnlock())
				return 0x8023;
			
			temp_data =
				isdu_in_buffer[offset + 3] 				|
				(isdu_in_buffer[offset + 2] << 8)	|
				(isdu_in_buffer[offset + 1] << 16)|
				(isdu_in_buffer[offset] << 24)
			;
			
			app_nvm_setADC6Offset((int32_t)temp_data);
		}
		break;
		
		case 0x0105:// Iout scaling calibration value
		{
			if(!app_nvm_getAccessUnlock())
				return 0x8023;
			
			temp_data =
				isdu_in_buffer[offset + 3] 				|
				(isdu_in_buffer[offset + 2] << 8)	|
				(isdu_in_buffer[offset + 1] << 16)|
				(isdu_in_buffer[offset] << 24)
			;
			
			app_nvm_setADC7Scaling((int32_t)temp_data);
		}
		break;
		
		case 0x0106:// Iout offset calibration value
		{
			if(!app_nvm_getAccessUnlock())
				return 0x8023;
			
			temp_data =
				isdu_in_buffer[offset + 3] 				|
				(isdu_in_buffer[offset + 2] << 8)	|
				(isdu_in_buffer[offset + 1] << 16)|
				(isdu_in_buffer[offset] << 24)
			;
			
			app_nvm_setADC7Offset((int32_t)temp_data);
		}
		break;		
		
		default:
			// Return Access denied error code
			return 0x8023;
	}
	
	return 0;
}