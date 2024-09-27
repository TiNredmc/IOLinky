#include "iol_dl.h"

// Private variables
uint8_t eventNumber = 0;

// Private prototypes
void iol_dl_setEventFlag();


void iol_dl_eventFSM(){
	
	// Set the Activated Events bit
	// in Status code (type 2 thing)
	switch(eventNumber){
		case 0:
			iol_evt_t.StatusCode = 0x80;
		break;
		
		case 1:
			iol_evt_t.StatusCode = 0x81;
			iol_dl_setEventFlag();
		break;
		
		case 2:
			iol_evt_t.StatusCode = 0x83;
			iol_dl_setEventFlag();
		break;
		
		case 3:
			iol_evt_t.StatusCode = 0x87;
			iol_dl_setEventFlag();
		break;
		
		case 4:
			iol_evt_t.StatusCode = 0x8F;
			iol_dl_setEventFlag();
		break;
		
		case 5:
			iol_evt_t.StatusCode = 0x9F;
			iol_dl_setEventFlag();
		break;
		
		case 6:
			iol_evt_t.StatusCode = 0xBF;
			iol_dl_setEventFlag();
		break;
	}
	

}

void iol_dl_pushEvt(
	uint8_t eventType,
	uint16_t eventCode
	){

	if(eventNumber > 6)
		return;
		
	iol_evt_t.
		EventSlot[eventNumber].
		EventQualifierBit.instant	= 4;
		
	iol_evt_t.
		EventSlot[eventNumber].
		EventQualifierBit.type = eventType;
		
	iol_evt_t.EventSlot[eventNumber].EventCode =
		((uint8_t)eventCode << 8) |
		((uint8_t)(eventCode >> 8));
		
	eventNumber++;
	if(eventNumber > 6)
		eventNumber = 6;
}
	
void iol_dl_popEvt(){
	if(eventNumber == 0)
		return;
	eventNumber--;
}