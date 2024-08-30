#include "iol_al.h"

// Device's direct parameter data
device_directparam_t device_dp_t = {
	// Page 1
	{
		0,									// MasterCommand
		0,									// MasterCycleTime
		
		(1 << 6) | 
		(34),								// MinCycleTime -> 20ms
		
		PREOP_M_0 | 				// MSeqenceCapability
		OP_M_2_V8 | 1,			// OPERATE : TYPE2_V 8 PDin octets
		
		0x11,								// RevisionID IO-Link V1.1
		(1 << 7) | (8 - 1),	// ProcessDataIn -> 8 octets
		0,									// ProcessDataOut -> 0
		0xFFFF,							// VendorID
		0x563412,						// DeviceID
		0,									// FunctionID
		0,									// --Reserved--
		0										// SystemCommand
	},
	
	// Page 2 // Device Specific profile
	{
		1, 2, 3, 4,
		5, 6, 7, 8, 
		9, 10, 11, 12, 
		13, 14, 15, 16					
	}
	
};

isdu_data_t isdu_d_t = {
	"KMITL",
	"The Master of Innivation",
	"IOLinkyF042",
	"10Linky",
	"TinLethax",
	"1234ABCD",
	"F042_6362",
	"F042"
};

// Initialize the underlying layer (AL->DL->PL)
void iol_al_init(
	uint8_t *PD_data_ptr	
	){
	iol_dl_init(
		(uint8_t *)&device_dp_t,	
		&isdu_d_t,
		(uint8_t *)PD_data_ptr,
		8			// 8 octets for TYPE2_V with 8 PD in
	);
}

// Poll to run the IO-Link communication
// called in app_device_impl.c
void iol_al_poll(){
	iol_dl_poll();
}

// Demo used to update Process Data.
void iol_al_updatePD(){
	iol_dl_updatePD();
}
