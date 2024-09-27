#include "iol_dl.h"

// Private pointers
uint8_t *mseq_message_ptr;
uint8_t *mseq_ckt_ptr;
uint8_t *mseq_pd_ptr;
uint8_t *mseq_cks_ptr;

// Private variables
uint8_t mseq_current_mtype 	= 0;
uint8_t mseq_message_length = 0;
uint8_t mseq_od_offset			=	0;
uint8_t mseq_od_length			= 0;
uint8_t mseq_pdin_offset		= 0;
uint8_t mseq_pdin_length		= 0;

uint8_t mseq_read_length		= 0;
uint8_t mseq_write_length		= 0;

uint8_t mseq_reply_len 			= 0;
uint8_t mseq_reply_offset 	= 0;

uint8_t m_od								= 0;

uint8_t i_len 							= 0;

uint8_t xor_odd 						= 0;
uint8_t xor_even 						= 0;
uint8_t xor_pair						= 0;

// Private typedef
typedef struct{
	union{
		uint8_t MC;
		struct{
			uint8_t ADDR	:5;// Address
			uint8_t CC		:2;// Commu Channel
			uint8_t RW		:1;// Read/Write
		}MCBit;
	};
}mseq_mc_t;

mseq_mc_t mseq_mc;

typedef struct{
	union{
		uint8_t CKT;
		struct{
			uint8_t CKT		:6;// Checksum
			uint8_t MT		:2;// M-sequence type
		}CKTBit;
	};
}mseq_ckt_t;

mseq_ckt_t mseq_ckt;

typedef struct{
	union{
		uint8_t CKS;
		struct{
			uint8_t CKS		:6;// Checksum
			uint8_t PD		:1;// PD status
			uint8_t EV		:1;// Event flag
		}CKSBit;
	};
}mseq_cks_t;

mseq_cks_t mseq_cks;

typedef union __attribute__((packed)){
	uint8_t CKS_B;
	struct{
		uint8_t CKS_0	:1;
		uint8_t CKS_1	:1;
		uint8_t CKS_2	:1;
		uint8_t CKS_3	:1;
		uint8_t CKS_4	:1;
		uint8_t CKS_5	:1;
		uint8_t CKS_6	:1;
		uint8_t CKS_7	:1;
	};
}iol_mtype_cks_t;

iol_mtype_cks_t iol_cks_t;

// Private Prototypes
void iol_dl_mseqReadPage(uint8_t m_addr);
void iol_dl_mseqReadEvent(uint8_t m_addr);
void iol_dl_mseqReadISDU(uint8_t m_addr);

void iol_dl_mseqWritePage(uint8_t m_addr);
void iol_dl_mseqWriteEvent(uint8_t m_addr);
void iol_dl_mseqWriteISDU(uint8_t m_addr);

void iol_dl_cksAssembler();


void iol_dl_setMseq(
	uint8_t *mseq_ptr,
	uint8_t m_cur_mtype,
	uint8_t m_od_pos,
	uint8_t m_od_len,
	uint8_t m_pdi_pos,
	uint8_t m_pdi_len
	){
	
	if(mseq_ptr == 0)
		return;
	
	if(m_od_pos < 2)
		return;
	
	if(m_od_len < 1)
		return;
	
	mseq_message_ptr 		= mseq_ptr;
	mseq_current_mtype	= m_cur_mtype;
	mseq_od_offset			= m_od_pos;
	mseq_od_length			= m_od_len;
	mseq_pdin_offset		= m_pdi_pos;
	mseq_pdin_length		= m_pdi_len;
	
	mseq_ckt_ptr				= mseq_message_ptr + 1;

	mseq_pd_ptr 				= 
		mseq_message_ptr		+
		mseq_pdin_offset		;
	
	mseq_cks_ptr 				=
		mseq_message_ptr 		+ 
		mseq_od_offset			+
		mseq_od_length			+
		mseq_pdin_length		;
	
	mseq_write_length 	= 
		mseq_pdin_length 	+	// PD length
		1;									// CKS
		
	mseq_read_length		=
		mseq_od_length 		+	// OD length
		mseq_pdin_length 	+	// PD length
		1;									// CKS
	
}

void iol_dl_mseqHandler(){
	// Get Msequence control
	mseq_mc.MC	=	*mseq_message_ptr;
	
	// Get Msequence type
	mseq_ckt.CKT = *mseq_ckt_ptr;

	// Check M-Type match
	if(mseq_ckt.CKTBit.MT != mseq_current_mtype)
		return;
	
	if(mseq_mc.MCBit.RW){
		// Read mode
		switch(mseq_mc.MCBit.CC){
			case 1:// Page data
				iol_dl_mseqReadPage(mseq_mc.MCBit.ADDR);
				break;
			case 2:// Read Event
				iol_dl_mseqReadEvent(mseq_mc.MCBit.ADDR);
				break;
			case 3:// ISDU data
				iol_dl_mseqReadISDU(mseq_mc.MCBit.ADDR);
				break;
			default:
				dl_main_fsm = DL_MFSM_PR;
				break;
		}
		
		// Read mode -> Reply including the OD
		mseq_reply_len = mseq_read_length;
			
		mseq_reply_offset = mseq_od_offset;
		
	}else{
		// Write mode
		switch(mseq_mc.MCBit.CC){
			case 1:// Page data
				iol_dl_mseqWritePage(mseq_mc.MCBit.ADDR);
				break;
			case 2:// write Event
				iol_dl_mseqWriteEvent(mseq_mc.MCBit.ADDR);
				break;
			case 3:// ISDU data
				iol_dl_mseqWriteISDU(mseq_mc.MCBit.ADDR);
				break;
			default:
				dl_main_fsm = DL_MFSM_PR;
				break;
		}
		
		// Write mode -> Reply not including the OD
		mseq_reply_len = mseq_write_length;
		
		// Skip the OD byte
		mseq_reply_offset = 
			mseq_od_offset +
			mseq_od_length;
	}
	
	// Only update PD when we have it
	if((mseq_pdin_length > 0)	&&
		(PD_setFlag == 1)){
		PD_setFlag = 0;
		mseq_pd_ptr[7] = *pdIn_ptr;
		mseq_pd_ptr[6] = *(pdIn_ptr+1);
		mseq_pd_ptr[5] = *(pdIn_ptr+2);
		mseq_pd_ptr[4] = *(pdIn_ptr+3);
		mseq_pd_ptr[3] = *(pdIn_ptr+4);
		mseq_pd_ptr[2] = *(pdIn_ptr+5);
		mseq_pd_ptr[1] = *(pdIn_ptr+6);
		mseq_pd_ptr[0] = *(pdIn_ptr+7);
	}
		
	iol_dl_cksAssembler();
	
}

// Handling Read request
void iol_dl_mseqReadPage(uint8_t m_addr){
	*(mseq_message_ptr + 
		mseq_od_offset) = *(uint8_t *)
			(direct_param_ptr + 
			(m_addr & 0x1F)
			);
}

void iol_dl_mseqReadEvent(uint8_t m_addr){
	*(mseq_message_ptr + 
		mseq_od_offset) = *(uint8_t *)
			((uint8_t *)&iol_evt_t + 
			(m_addr & 0x1F)
			);
}

void iol_dl_mseqReadISDU(uint8_t m_addr){
	uint8_t *m_od_ptr =
		(mseq_message_ptr + 
		mseq_od_offset);
	
	// Grab the M-seq count
	if(m_addr < 0x10){
		*m_od_ptr = ISDU_out_buffer[ISDU_data_pointer++];
		if(ISDU_data_pointer == ISDU_data_count){
			ISDU_data_pointer = 0;
			//ISDU_data_count = 0;
			dl_isdu_fsm = 0;// Enter Idle state
		}
	}

	switch(m_addr){
		case 0x10:// FlowCTRL START, send read respond I-service
			if(dl_isdu_fsm < 3)
				*m_od_ptr = 0x01;// Reply busy response until we parsed ISDU
			else{
				*m_od_ptr = ISDU_out_buffer[0];// reply I-service 
				ISDU_data_pointer = 1;// Start on next byte
			}
			break;
		
		case 0x11:// FlowCTRL IDLE1
			// Reply with OD = 0x00
			*m_od_ptr = 0x00;
			timeout_counter = 0;
			dl_isdu_fsm = 0;// Went back to idle state
			break;
		
		case 0x1F:// FlowCTRL ABORT
			ISDU_data_pointer = 0;
			ISDU_data_count = 0;
			dl_isdu_fsm = 0;// Went back to idle state
			break;
		
		case 0x12:// FlowCTRL IDLE2 (reserved for future use, no use here for now...)
		default:// 0x13 to 0x1E is reserved
			break;
	}
}

// Handling Write request
void iol_dl_mseqWritePage(uint8_t m_addr){
	// Store for internal use at DL
	switch(m_addr){
		case 0x00:
			master_cmd = *(
				mseq_message_ptr + mseq_od_offset);
			break;
		case 0x01:
			master_cycle = *(
				mseq_message_ptr + mseq_od_offset);
			break;
		default:
			*(uint8_t *)(direct_param_ptr + (m_addr & 0x1F)) = 
				*(mseq_message_ptr + mseq_od_offset);
			break;
	}		
}

void iol_dl_mseqWriteEvent(uint8_t m_addr){
	iol_dl_popEvt();
}

void iol_dl_mseqWriteISDU(uint8_t m_addr){

	if(m_addr < 0x10){
		ISDU_in_buffer[ISDU_data_pointer] = *(
			mseq_message_ptr + mseq_od_offset);;
		
		if(iol_iservice.length == 1)
			ISDU_data_count = ISDU_in_buffer[1];// Grab the Extlength
		
		ISDU_data_pointer++;
		
		if(ISDU_data_count == ISDU_data_pointer){ 
			dl_isdu_fsm = 1;// enter decode state
			ISDU_data_pointer = 0;
		}
		
	}
	
	switch(m_addr){
			case 0x10:// Get the ISDU I-service and kick FSM into decode state
				ISDU_in_buffer[0] = *(
					mseq_message_ptr + mseq_od_offset);
				iol_iservice.Iservice = ISDU_in_buffer[0];
			
				if(iol_iservice.length != 1)// In case of no ExtLength, we can grab the data length directly
					ISDU_data_count = iol_iservice.length;
				
				ISDU_data_pointer = 1;// Reset data pointer to next one
			break;
		
		default:
			break;
	}
	
}

// Check sum, PD status and Event flagging
void iol_dl_setPDInvalid(){
	mseq_cks.CKSBit.PD 	= 1;
}

void iol_dl_setEventFlag(){
	mseq_cks.CKSBit.EV	=	1;
}

void iol_dl_cksAssembler(){
	uint8_t *temp_addr 	= 
		mseq_message_ptr 	+ 
		mseq_reply_offset;
	
	// clear old cks
	iol_cks_t.CKS_B = 0x00;
	
	// Update PD invalid and Event flags
	*mseq_cks_ptr = mseq_cks.CKS;
	
	// Calculate checksum	
	for(i_len=0; i_len < mseq_reply_len; i_len++){
		iol_cks_t.CKS_B ^= *temp_addr;
		temp_addr++;
	}

	iol_cks_t.CKS_B ^= 0x52;
	
	xor_even = 
		iol_cks_t.CKS_0 ^
		iol_cks_t.CKS_2	^
		iol_cks_t.CKS_4	^
		iol_cks_t.CKS_6;
	
	xor_odd =	
		iol_cks_t.CKS_1	^
		iol_cks_t.CKS_3	^
		iol_cks_t.CKS_5	^
		iol_cks_t.CKS_7;
	
	xor_pair =
		( iol_cks_t.CKS_0 ^ iol_cks_t.CKS_1)				|
		((iol_cks_t.CKS_2 ^ iol_cks_t.CKS_3) 	<< 1)	|
		((iol_cks_t.CKS_4 ^ iol_cks_t.CKS_5)	<< 2)	|
		((iol_cks_t.CKS_6 ^ iol_cks_t.CKS_7)	<< 3)	;	
		
	// Write to CKS buffer
	mseq_cks.CKSBit.CKS = 
			(xor_odd 	<< 5) 		|
			(xor_even << 4) 		|
			(xor_pair & 0x0F)		;

	*mseq_cks_ptr = mseq_cks.CKS;

	// Clear all flag and cks
	mseq_cks.CKS = 0x00;

	// Point write buffer to the OD 
	iol_pl_update_WriteBuffer(
		mseq_message_ptr + 
		mseq_reply_offset
		);
	// Send write request
	iol_pl_WriteRequest(mseq_reply_len);	
}
