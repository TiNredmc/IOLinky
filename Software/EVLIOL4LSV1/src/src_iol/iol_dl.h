#ifndef IOL_DL_H
#define IOL_DL_H

#include <stdint.h>
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
}iol_mtype_0;



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
	
}iol_mtype_1_2;

typedef struct{
	union{
		uint8_t Iservice;
		struct{
			uint8_t length:4;
			uint8_t Iserv	:4;
		};		
	};

}iol_isdu_Iserv;


void iol_dl_init();

void iol_dl_poll();
uint8_t iol_dl_getModeStatus();


#endif