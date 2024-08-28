#include "iol_dl.h"

void iol_dl_T0WritePage(uint8_t address);
void iol_dl_T0ReadPage(uint8_t address);
void iol_dl_T0WriteISDU(uint8_t address);
void iol_dl_T0ReadISDU(uint8_t address);
void iol_dl_T0Xor();

// parsing M-sequence in PREOPERATE mode
// Since this mode use the same M-sequence type as STARTUP mode
// we can easily (lazily) share the R/W functions
void iol_dl_handlePREOPERATE(){
	
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