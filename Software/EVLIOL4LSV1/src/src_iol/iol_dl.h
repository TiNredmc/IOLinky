#ifndef IOL_DL_H
#define IOL_DL_H

#include <stdint.h>
#include <string.h>
#include <stm32g071xx.h>
#include "iol_pl.h"
#include "gpio_pinout.h"

// DL Main FSM enums
enum DL_MAIN_FSM{
	DL_MFSM_PR	= 0,// Wait for message read
	DL_MFSM_WFWR		// Wait for reply done
};

enum DL_MODE_FSM{
	DL_MODE_STARTUP = 0,
	DL_MODE_PREOP,
	DL_MODE_OP
};

typedef struct __attribute__((packed)){
	union{
		uint8_t MC;
		struct{
			uint8_t ADDR	:5;// Address
			uint8_t CC		:2;// Commu Channel
			uint8_t RW		:1;// Read/Write
		}MCBit;
	};
	
	union{
		uint8_t CKT;
		struct{
			uint8_t CKS		:6;// Checksum
			uint8_t MT		:2;// M-sequence type
		}CKTBit;
	};
	
	uint8_t OD;// On-request Data
	
	union{
		uint8_t CKS;
		struct{
			uint8_t CKS		:6;// Checksum
			uint8_t PD		:1;// PD status
			uint8_t EV		:1;// Event flag
		}CKSBit;
	};
}iol_mtype_0_t;

typedef struct __attribute__((packed)){
	union{
		uint8_t MC;
		struct{
			uint8_t ADDR	:5;// Address
			uint8_t CC		:2;// Commu Channel
			uint8_t RW		:1;// Read/Write
		}MCBit;
	};
	
	union{
		uint8_t CKT;
		struct{
			uint8_t CKS		:6;// Checksum
			uint8_t MT		:2;// M-sequence type
		}CKTBit;
	};
	
	uint8_t OD[2];// On-request Data
	
	union{
		uint8_t CKS;
		struct{
			uint8_t CKS		:6;// Checksum
			uint8_t PD		:1;// PD status
			uint8_t EV		:1;// Event flag
		}CKSBit;
	};
	
}iol_mtype_1_2_t;

typedef struct __attribute__((packed)){
	union{
		uint8_t MC;
		struct{
			uint8_t ADDR	:5;// Address
			uint8_t CC		:2;// Commu Channel
			uint8_t RW		:1;// Read/Write
		}MCBit;
	};
	
	union{
		uint8_t CKT;
		struct{
			uint8_t CKS		:6;// Checksum
			uint8_t MT		:2;// M-sequence type
		}CKTBit;
	};
	
	uint8_t OD;// On-request Data
	
	// Process Data
	union{
		uint16_t PD;
		uint8_t  PD_small[2];
	};
	
	union{
		uint8_t CKS;
		struct{
			uint8_t CKS		:6;// Checksum
			uint8_t PD		:1;// PD status
			uint8_t EV		:1;// Event flag
		}CKSBit;
	};
	
}iol_mtype_2_2_t;

typedef struct{
	union{
		uint8_t Iservice;
		struct{
			uint8_t length:4;
			uint8_t Iserv	:4;
		};		
	};
	
	uint8_t ExtLength;
	uint8_t Index[2];
	uint8_t Subindex;

}iol_isdu_t;


void iol_dl_init(
	uint8_t *dev_param_1_ptr,
	uint8_t *pd_in_ptr,
	uint8_t pd_in_cnt
	);

void iol_dl_poll();
uint8_t iol_dl_getModeStatus();
void iol_dl_updatePD();

#endif