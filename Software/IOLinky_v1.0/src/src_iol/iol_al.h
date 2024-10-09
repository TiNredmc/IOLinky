#ifndef IOL_AL_H
#define IOL_AL_H

#include <stdint.h>
#include "iol_dl.h"

#include "app_psu.h"

enum PREOP_MSEQ_CODE{
	PREOP_M_0 = 0,
	PREOP_M_1_2 = 1,
	PREOP_M_1_V8 = 2,
	PREOP_M_1_V32 = 3
};

enum OP_MSEQ_CODE{
	OP_M_0 = 0,
	OP_M_1_2 = 1,
	OP_M_2_2 = 0,
	OP_M_2_V8 = 4
};

enum EVT_TYPE{
	EVT_NOTIFICATION	= 1,
	EVT_WARNING				= 2,
	EVT_ERROR					= 3
};

enum EVT_MODE{
	EVT_SINGLE_SHOT = 1,
	EVT_DISAPPEARS	= 2,
	EVT_APPEARS			= 3
};

typedef struct __attribute__((packed)){
	// Comminucation Control
	uint8_t MasterCommand;
	
	union{
	uint8_t MasterCycleTime;
	struct{
		uint8_t MCMult			:6;
		uint8_t MCTimeBase	:2;
	}McycleBit;
	};
	
	union{
	uint8_t MinCycleTime;
	struct{
		uint8_t MTMult			:6;
		uint8_t MTTimeBase	:2;
	}MinCycleBit;
	};
	
	union{
	uint8_t MSequenceCapability;
	struct{
		uint8_t MSisdu				:1;
		uint8_t MSeqOpCode		:3;
		uint8_t MSeqPreOpCode	:2;
		uint8_t MSreserved		:2;
	};
	};
	
	union{
	uint8_t RevisionID;
	struct{
		uint8_t	MinorRev			:4;
		uint8_t MajorRev			:4;
	};
	};
	
	union{
	uint8_t ProcessDataIn;
	struct{
		uint8_t PDIlength	:5;
		uint8_t PDIres1		:1;
		uint8_t PDISIO		:1;
		uint8_t PDIByte		:1;
	};
	};
	
	union{
	uint8_t ProcessDataOut;
	struct{
		uint8_t PDOlength	:5;
		uint8_t PDOres2		:1;
		uint8_t PDOres1		:1;
		uint8_t PDOByte		:1;
	};
	};
	
	// Identification
	uint16_t VendorID;
 	union{
	uint32_t DeviceID:24;
	struct{
		uint8_t DeviceID1:8;
		uint8_t DeviceID2:8;
		uint8_t DeviceID3:8;
	};
	};
	uint16_t FunctionID;
	uint8_t reserved1;
	// Application Control
	uint8_t SystemCommand;
}direct_param_p1_t;

typedef struct __attribute__((packed)){
	uint8_t data[16];
}direct_param_p2_t;

typedef struct __attribute__((packed)){
	direct_param_p1_t dp_p1_t;
	direct_param_p2_t dp_p2_t;
}device_directparam_t;

void iol_al_init(
	device_directparam_t *d_dp_t,
	isdu_data_t *isdu_dat_t,
	uint8_t *PD_data_ptr	
	);
void iol_al_poll();
void iol_al_updatePD();
void iol_al_AppReportEvent(
	uint16_t eventCode
	);

// For ISDU
uint16_t iol_al_handleISDURead(
	uint16_t index,
	uint8_t *isdu_out_buffer
	);

uint16_t iol_al_handleISDUWrite(
	uint16_t index,
	uint8_t *isdu_in_buffer,
	uint16_t length,
	uint16_t offset
	);

#endif