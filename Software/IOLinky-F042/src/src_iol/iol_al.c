#include "iol_al.h"

// Device's direct parameter data
device_directparam_t device_dp_t = {
	// Page 1
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
		0xFFFF,				// VendorID
		0x563412,				// DeviceID
		0,					// FunctionID
		0,					// --Reserved--
		0						// SystemCommand
	},
	
	// Page 2 // Device Specific profile
	{
		1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16					
	}
	
};

isdu_data_t isdu_d_t = {
	"KMITL",
	"The Master of Innivation",
	"IOLinkyF042",
	"10Linky",
	"TinLethax",
	"1234ABCD",
	"EVLIOL",
	"EVL"
};

uint16_t PD_test = 0x0000;
uint16_t PD_cpy  = 0;
// Initialize the underlying layer (AL->DL->PL)
void iol_al_init(){
	iol_dl_init(
		(uint8_t *)&device_dp_t,	
		&isdu_d_t,
		(uint8_t *)&PD_cpy,
		1
	);
}

// Poll to run the IO-Link communication
// called in app_device_impl.c
void iol_al_poll(){
	iol_dl_poll();
}

// Demo used to update Process Data.
void iol_al_updatePD(){
	PD_test++;
	PD_cpy = PD_test;
	iol_dl_updatePD();
}
