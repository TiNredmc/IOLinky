#include "iol_al.h"

device_directparam_t device_dp_t = {
	{
		0,					// MasterCommand
		0,					// MasterCycleTime
		(0 << 6) | 
		(40),					// MinCycleTime
		PREOP_M_0 | 
		OP_M_2_2 | 1,					// MSeqenceCapability
		0x11,					// RevisionID
		16,					// ProcessDataIn 16 Bit
		0,					// ProcessDataOut;
		0x3412,				// VendorID
		0xA017DE,				// DeviceID
		0,					// FunctionID
		0,					// --Reserved--
		0						// SystemCommand
	},
	
	{
		0						// Device Specific profile
	}
	
};

uint16_t PD_test = 0xAA55;

void iol_al_init(){
	iol_dl_init(
		(uint8_t *)&device_dp_t.dp_p1_t,	
		(uint8_t *)&PD_test,
		1
	);
}

void iol_al_poll(){


}
