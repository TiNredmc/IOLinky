#ifndef IOL_L6364_H
#define IOL_L6364_H

#include <stdint.h>

#include "spi.h"


#define REG_MSEQ		0x00
#define REG_CFG			0x01
#define REG_CCTL		0x02
#define REG_DCTL		0x03
#define REG_LINK		0x04
#define REG_THERM		0x05
#define REG_TEMP		0x06
#define REG_LED			0x07
#define REG_DCDC		0x08
#define REG_DSTAT		0x09
#define REG_STATUS	0x0A
#define REG_LINK2		0x0F
#define REG_FR0			0x10
#define REG_FR14		0x1E

/* REGSITER AND DATA STRUCTs*/

typedef struct __attribute__((packed)){
	// Shadown register of status register (First byte transaction from the SPI)
	union{
		struct{
			uint8_t INT			:1;
			uint8_t	UV			:1;
			uint8_t DINT		:1;
			uint8_t CHK			:1;
			uint8_t DAT			:1;
			uint8_t SSC			:1;
			uint8_t SOT			:1;
			uint8_t RST			:1;
		}statusBit;
		uint8_t STATUS;
	}status0;
	
	
	// MSEQ register
	union{
		struct{
			uint8_t OD2			:2;
			uint8_t M2CNT		:4;
			uint8_t OD1			:2;
		}mseqBit;
		uint8_t MSEQ;
	}mseq;
	
	// CFG register
	union{
		struct{
			uint8_t res			:2;
			uint8_t PD5V		:1;
			uint8_t RF			:1;
			uint8_t BD			:1;
			uint8_t UVT			:3;
		}cfgBit;
		uint8_t CFG;
	}cfg;
	
	// CCTL register
	union{
		struct{
			uint8_t LS			:1;
			uint8_t HS			:1;
			uint8_t SIO			:1;
			uint8_t SGL			:1;
			uint8_t SCT			:3;
			uint8_t TRNS		:1;
		}cctlBit;
		uint8_t CCTL;
	}cctl;

	// DCTL register
	union{
		struct{
			uint8_t LS			:1;
			uint8_t HS			:1;
			uint8_t DIO			:1;
			uint8_t IEN			:1;
			uint8_t SCT			:3;
			uint8_t EXT			:1;
		}dctlBit;
		uint8_t	DCTL;
	}dctl;
	
	// LINK register
	union{
		struct{
			uint8_t SND			:1;
			uint8_t END			:1;
			uint8_t CNT			:4;
			uint8_t res			:2;
		}linkBit;
		uint8_t LINK;
	}link;
	
	// THERM register
	union{
		struct{
			uint8_t TH			:5;
			uint8_t RES 		:2;
			uint8_t AUT			:1;
		}thermBit;
		uint8_t THERM;
	}therm;
	
	// TEMP register
	union{
		uint8_t	TEMP;
	}temp;
	
	// LED register
	union{
		struct{
			uint8_t LED2		:4;
			uint8_t LED1		:4;
		}ledBit;
		uint8_t LED;
	}led;
	
	// DCDC register
	union{
		struct{
			uint8_t VSET		:3;
			uint8_t FSET		:3;
			uint8_t BYP			:1;
			uint8_t DIS			:1;
		}dcdcBit;
		uint8_t DCDC;
	}dcdc;
	
	// DSTAT register
	union{
		struct{
			uint8_t res2		:1;
			uint8_t SSC			:1;
			uint8_t	LVL			:1;
			uint8_t res			:5;
		}dstatBit;
		uint8_t DSTAT;
	}dstat;
	
	// STATUS register
	union{
		struct{
			uint8_t INT			:1;
			uint8_t	UV			:1;
			uint8_t DINT		:1;
			uint8_t CHK			:1;
			uint8_t DAT			:1;
			uint8_t SSC			:1;
			uint8_t SOT			:1;
			uint8_t RST			:1;
		}statusBit;
		uint8_t STATUS;
	}status;
	
	uint8_t reserved[4];// Secrets hidden here?
	
	// LINK2 Register
	union{
		struct{
			uint8_t SND			:1;
			uint8_t END			:1;
			uint8_t CNT			:4;
			uint8_t res			:2;
		}link2Bit;
		uint8_t LINK2;
	}link2;
	
}l6364_reg_t;

// L6364 FIFO read data Register
typedef struct __attribute__((packed)){
	uint8_t stat0;
	
	// Buffer register
	uint8_t FR[15];

}l6364_fr_t;


/* END REGSITERs AND DATA STRUCTs*/

/* ENUMs*/
enum MSEQ_OD1{
	MSEQ_OD1_BC = 0,
	MSEQ_OD1_2B = 1,
	MSEQ_OD1_8B = 2
};

enum MSEQ_OD2{
	MSEQ_OD2_1B = 0,
	MSEQ_OD2_2B = 1,
	MSEQ_OD2_8B = 2
};

enum CFG_UVLO{
	UVLO_15V0 = 0,
	UVLO_14V0 = 1,
	UVLO_13V0 = 2,
	UVLO_12V0 = 3,
	UVLO_10V5 = 4,
	UVLO_8V5  = 5,
	UVLO_7V5  = 6,
	UVLO_6V0  = 7
};

enum COM_MODE{
	COM2 = 0,
	COM3 = 1
};

// CCTL and DCTL ISET
enum SET{
	ISET_190_380 = 0,
	ISET_210_420,
	ISET_230_460,
	ISET_250_500,
	ISET_110_220,
	ISET_130_260,
	ISET_150_300,
	ISET_170_340
};

enum DCDC_FREQ{
	DCDC_1000K = 0,
	DCDC_1250K,
	DCDC_1670K,
	DCDC_2000K,
	DCDC_400K,
	DCDC_625K,
	DCDC_710K,
	DCDC_830K
};

enum DCDC_VSET{
	DCDC_5V0 = 0,
	DCDC_5V6,
	DCDC_6V1,
	DCDC_6V8,
	DCDC_7V8,
	DCDC_8V6,
	DCDC_9V6,
	DCDC_10V5
};

/* END ENUMs*/


// Peforms IO stuffs
uint8_t l6364_readReg(uint8_t reg);
void l6364_writeReg(
	uint8_t reg, 
	uint8_t data
	);
void l6364_setCOM(uint8_t com_mode);

// Status related
void l6364_getStatus();
uint8_t l6364_isRST();
uint8_t l6364_isINT();
uint8_t l6364_isUV();
uint8_t l6364_isDINT();
uint8_t l6364_isCHK();
uint8_t l6364_isDAT();
uint8_t l6364_isSSC();
uint8_t l6364_isSOT();

// M-sequence related stuffs
void l6364_setMseq(
	uint8_t m_len,		// Master message length (Including MC and CKT)
	uint8_t	od1_len,	// 
	uint8_t od2_len		//
	);
uint8_t iol_pl_getCurrentMtype();
void iol_pl_setMtype0();
void iol_pl_setMtype1_2();

void l6364_setUVLO(uint8_t uvlo_val);


// Set LED brightness
void l6364_setLED1(uint8_t Iled);
void l6364_setLED2(uint8_t Iled);

// Set Operation modes
void l6364_setIOLmode();
void l6364_setSIOmode();

// Polling-required code
// Periodic execution with timer or millis thingy super-loop
void l6364_pollAllReg();
uint8_t l6364_pollLinkCnt();

// FIFO r/w stuffs
uint8_t l6364_getRxCount();// Same as pollLinkCnt but retrieve fresh data from SPI.
void l6364_readFIFO(
	uint8_t count, 
	uint8_t *output_ptr
	);

void l6364_setTXCount(uint8_t count);
void l6364_writeFIFO(
	uint8_t count,
	uint8_t *input_ptr
	);

uint8_t l6364_readFIFOFast(uint8_t *output_ptr);
void l6364_writeFIFOFast(
	uint8_t count,
	uint8_t *input_ptr 
	);

void l6364_linkEnd();
void l6364_linkSend();




#endif