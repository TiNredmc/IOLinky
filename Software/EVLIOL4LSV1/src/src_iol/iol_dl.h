#ifndef IOL_DL_H
#define IOL_DL_H

#include <stdint.h>

typedef struct{
	union{
		uint8_t MC;
		struct{
			uint8_t RW		:1;// Read/Write
			uint8_t CC		:2;// Commu Channel
			uint8_t ADDR	:5;// Address
		}MCBit;
	};
	
	union{
		uint8_t CKT;
		struct{
			uint8_t MT		:2;// M-sequence type
			uint8_t CKS		:6;// Checksum
		}CKTBit;
	};
	
	uint8_t OD;// On-request Data
	
	union{
		uint8_t CKS;
		struct{
			uint8_t EV		:1;// Event flag
			uint8_t PD		:1;// PD status
			uint8_t CKS		:6;// Checksum
		}CKSBit;
	};
}iol_mtype_0;

typedef struct{
	union{
		uint8_t MC;
		struct{
			uint8_t RW		:1;// Read/Write
			uint8_t CC		:2;// Commu Channel
			uint8_t ADDR	:5;// Address
		}MCBit;
	};
	
	union{
		uint8_t CKT;
		struct{
			uint8_t MT		:2;// M-sequence type
			uint8_t CKS		:6;// Checksum
		}CKTBit;
	};
	
	uint8_t PD[2];// Process Data
	
	union{
		uint8_t CKS;
		struct{
			uint8_t EV		:1;// Event flag
			uint8_t PD		:1;// PD status
			uint8_t CKS		:6;// Checksum
		}CKSBit;
	};
	
}iol_mtype_1_1;

typedef struct{
	union{
		uint8_t MC;
		struct{
			uint8_t RW		:1;// Read/Write
			uint8_t CC		:2;// Commu Channel
			uint8_t ADDR	:5;// Address
		}MCBit;
	};
	
	union{
		uint8_t CKT;
		struct{
			uint8_t MT		:2;// M-sequence type
			uint8_t CKS		:6;// Checksum
		}CKTBit;
	};
	
	uint8_t OD[2];// On-request Data
	
	union{
		uint8_t CKS;
		struct{
			uint8_t EV		:1;// Event flag
			uint8_t PD		:1;// PD status
			uint8_t CKS		:6;// Checksum
		}CKSBit;
	};
	
}iol_mtype_1_2;












#endif