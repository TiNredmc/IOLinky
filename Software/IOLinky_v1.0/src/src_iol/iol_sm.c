#include "iol_sm.h"

uint8_t sm_main_fsm = 0;


void iol_sm_poll(){

	switch(sm_main_fsm){
		case SM_MAIN_INIT:
		{
		
		}
		break;
	
		default:
			sm_main_fsm = SM_MAIN_INIT;
			break;
	}
}