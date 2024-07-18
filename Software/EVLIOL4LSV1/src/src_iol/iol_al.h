#ifndef IOL_AL_H
#define IOL_AL_H

#include <stdint.h>

typedef struct{
	// Comminucation Control
	uint8_t MasterCommand;
	
	union{
	uint8_t MasterCycleTime;
	struct{
		uint8_t MCTimeBase	:2;
		uint8_t MCMult			:6;
	};
	};
	
	union{
	uint8_t MinCycleTime;
	struct{
		uint8_t MTTimeBase	:2;
		uint8_t MTMult			:6;
	};
	};
	
	union{
	uint8_t MSequenceCapability;
	struct{
		uint8_t MSreserved		:2;
		uint8_t MSeqPreOpCode	:2;
		uint8_t MSeqOpCode		:3;
		uint8_t MSisdu				:1;
	};
	};
	
	union{
	uint8_t RevisionID;
	struct{
		uint8_t	MajorRev			:4;
		uint8_t MinorRev			:4;
	};
	};
	
	union{
	uint8_t ProcessDataIn;
	struct{
		uint8_t PDIByte		:1;
		uint8_t PDISIO		:1;
		uint8_t PDIres1		:1;
		uint8_t PDIlength	:5;
	};
	};
	
	union{
	uint8_t ProcessDataOut;
	struct{
		uint8_t PDOByte		:1;
		uint8_t PDOres1		:1;
		uint8_t PDOres2		:1;
		uint8_t PDOlength	:5;
	};
	};
	
	// Identification
	uint16_t VendorID;
	union{
	uint32_t DeviceID:24;
	struct{
		uint8_t DeviceID1;
		uint8_t DeviceID2;
		uint8_t DeviceID3;
	};
	};
	uint16_t FunctionID;
	uint8_t reserved1;
	// Application Control
	uint8_t SystemCommand;
}direct_param_p1_t;

typedef struct{
	uint8_t data[0x0F];
}direct_param_p2_t;

typedef struct{
	direct_param_p1_t dp_p1_t;
	direct_param_p2_t dp_p2_t;
}device_directparam_t;


#endif