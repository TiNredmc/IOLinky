#include "iol_dl.h"

// Private typedef

iol_mtype_0_t		iol_mt0;				// M-sequence TYPE_0 for start up
#define MT0_LEN		4

//iol_mtype_1_2_t iol_mt1_2;			
//#define MT1_2_LEN	5

//iol_mtype_2_2_t iol_mt2_2;			
//#define MT2_2_LEN 6

iol_mtype_2_V_8PDI_t iol_mt2_v;
#define MT2_V_LEN 12

iol_isdu_t iol_iservice;

iol_event_t	iol_evt_t;

// Private variables

// DL FSMs
uint8_t dl_main_fsm = 0;
uint8_t dl_mode_fsm = 0;
uint8_t dl_isdu_fsm = 0;

// DL_mode related
uint8_t master_cmd = 0;
uint8_t master_cycle = 0;

// IO-Link comm timeout counter
uint32_t timeout_counter = 0;

// ISDU
//uint8_t ISDU_attempt_count = 0;
uint8_t ISDU_data_pointer = 0;
uint8_t ISDU_data_offset_index	= 0;
uint8_t ISDU_data_count = 0;
uint8_t ISDU_in_buffer[70] = {0};
uint8_t ISDU_out_buffer[70] = {0};
uint8_t ISDU_RWflag = 0;
uint8_t ISDU_16bIndex = 0;
//uint8_t ISDU_LenType = 0;

// Process Data In
uint8_t PD_In_count = 0;
uint8_t PD_setFlag = 0;

// Private pointers
uint8_t 				*direct_param_ptr;
isdu_data_t 		*isdu_device_data_t;
isdu_handler_t 	*isdu_pHandler_t;
uint8_t 				*pdIn_ptr;

// Private functions

void iol_dl_modeSwitcher();

void iol_dl_setMseq(
	uint8_t *mseq_ptr,
	uint8_t m_cur_mtype,
	uint8_t m_od_pos,
	uint8_t m_od_len,
	uint8_t m_pdi_pos,
	uint8_t m_pdi_len
	);
void iol_dl_mseqHandler();

void iol_dl_ISDUFSM();

void iol_dl_eventFSM();

// Initialize the DL (Actaully it just initialize PL)
void iol_dl_init(
	uint8_t *dev_param_1_ptr,
	isdu_handler_t *isdu_ptrHandler_t,
	isdu_data_t *isdu_data_ptr_t,
	uint8_t *pd_in_ptr,
	uint8_t pd_in_cnt
	){
		
	iol_pl_setMtype0();	
	iol_pl_update_ReadBuffer(
		&iol_mt0.MC
	);
	
		
	iol_dl_setMseq(
		(uint8_t *)&iol_mt0.MC,
		MSEQ_MTYPE_0,
		2,
		1,
		0,
		0
	);	
		
	direct_param_ptr 		=	dev_param_1_ptr;
	isdu_pHandler_t			=	isdu_ptrHandler_t;
	isdu_device_data_t 	= isdu_data_ptr_t;
	pdIn_ptr 						= pd_in_ptr;
	PD_In_count 				= pd_in_cnt;
}
	
// Poll to receive and send IO-Link data
// Called in iol_al_poll() in the polling manner
void iol_dl_poll(){
	// If no communication for some period of time
	// Fall back to STARTUP to be ready for next initiation
	timeout_counter++;
	if(timeout_counter == 0x5FA57){
		timeout_counter = 0;
		
		iol_pl_setMtype0();
		iol_pl_update_ReadBuffer(
			&iol_mt0.MC
		);
		
		iol_dl_setMseq(
			(uint8_t *)&iol_mt0.MC,
			MSEQ_MTYPE_0,
			2,
			1,
			0,
			0
		);
		
		dl_mode_fsm = DL_MODE_STARTUP;
		master_cmd = 0x97;
		
		dl_isdu_fsm = 0;
		ISDU_data_pointer = 0;
	}
		
	switch(dl_main_fsm){
		case DL_MFSM_PR:// Wait for UART message.
		{
			iol_pl_pollRead();
			if(!iol_pl_ReadAvailable())
				break;
			
			dl_main_fsm = DL_MFSM_WFWR;
			
			// If read available. Read the data.
			iol_dl_mseqHandler();// Takes 9us - to slow
			
		}
		break;

		case DL_MFSM_WFWR:// Wait for reply done
		{ 
			iol_pl_pollWrite();
			// Wait until reply (write) is done.
			if(iol_pl_checkWriteStatus()){
				
				// Switch mode if necessary
				iol_dl_modeSwitcher();
				
				dl_main_fsm = DL_MFSM_PR;
			}
		}
		break;
	}
	
	iol_dl_ISDUFSM();// Run the ISDU decoder FSM.
	iol_dl_eventFSM();// Run the Events Dispatcher.
}

// DL mode switcher handle FSM of IO-Link operation mode
// based on received master command. 
// Normally IO-Link device start with STARTUP state
// then later swith to PREOPERATE after master command
// in the end, IO-Link device will operate in OPERATE mode.
void iol_dl_modeSwitcher(){
	switch(master_cmd){
		case 0x5A:// Fallback to SIO but nah ¯\_(o_0)_/¯
		case 0x97:// Switch to start up mode
		{
			iol_pl_setMtype0();
			iol_pl_update_ReadBuffer(
				&iol_mt0.MC
			);
			
			iol_dl_setMseq(
				(uint8_t *)&iol_mt0.MC,
				MSEQ_MTYPE_0,
				2,
				1,
				0,
				0
			);
			
			
			dl_mode_fsm = DL_MODE_STARTUP;
		}
		break;
		
		case 0x99:// Switch to Operate mode
		{
			iol_pl_setMtype2_V_8PDI();
			iol_pl_update_ReadBuffer(
				&iol_mt2_v.MC
			);
			
			iol_dl_setMseq(
				(uint8_t *)&iol_mt2_v.MC,
				MSEQ_MTYPE_2,
				2,
				1,
				3,
				8
			);
			
			dl_mode_fsm = DL_MODE_OP;
		}	
		break;
		
		case 0x9A:// Switch to Preoperate mode
		{
			iol_pl_setMtype0();
			iol_pl_update_ReadBuffer(
				&iol_mt0.MC
			);
			
			iol_dl_setMseq(
				(uint8_t *)&iol_mt0.MC,
				MSEQ_MTYPE_0,
				2,
				1,
				0,
				0
			);
			
			dl_mode_fsm = DL_MODE_PREOP;
		}
		break;
		
		default:
			break;
	}
	master_cmd = 0;// Clear master cmd for receive next cmd.
}

// Get current DL_mode
uint8_t iol_dl_getModeStatus(){
	return dl_mode_fsm;
}


void iol_dl_updatePD(){
	PD_setFlag = 1;
}