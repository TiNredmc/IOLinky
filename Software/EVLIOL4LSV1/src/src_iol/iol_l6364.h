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

typedef struct{
	// Shadown register of status register (First byte transaction from the SPI)
	union{
		uint8_t STATUS;
		struct{
			uint8_t RST			:1;
			uint8_t INT			:1;
			uint8_t	UV			:1;
			uint8_t DINT		:1;
			uint8_t CHK			:1;
			uint8_t DAT			:1;
			uint8_t SSC			:1;
			uint8_t SOT			:1;
		}statusBit;
	}status0;
	
	
	// MSEQ register
	union{
		uint8_t MSEQ;
		struct{
			uint8_t OD1			:2;
			uint8_t M2CNT		:4;
			uint8_t OD2			:2;
		}mseqBit;
	}mseq;
	
	// CFG register
	union{
		uint8_t CFG;
		struct{
			uint8_t UVT			:3;
			uint8_t BD			:1;
			uint8_t RF			:1;
			uint8_t PD5V		:1;
			uint8_t res			:2;
		}cfgBit;
	}cfg;
	
	// CCTL register
	union{
		uint8_t CCTL;
		struct{
			uint8_t TRNS		:1;
			uint8_t SCT			:3;
			uint8_t SGL			:1;
			uint8_t SIO			:1;
			uint8_t HS			:1;
			uint8_t LS			:1;
		}cctlBit;
	}cctl;

	// DCTL register
	union{
		uint8_t	DCTL;
		struct{
			uint8_t EXT			:1;
			uint8_t SCT			:3;
			uint8_t IEN			:1;
			uint8_t DIO			:1;
			uint8_t HS			:1;
			uint8_t LS			:1;
		}dctlBit;
	}dctl;
	
	// LINK register
	union{
		uint8_t LINK;
		struct{
			uint8_t res			:2;
			uint8_t CNT			:4;
			uint8_t END			:1;
			uint8_t SND			:1;
		}linkBit;
	}link;
	
	// THERM register
	union{
		uint8_t THERM;
		struct{
			uint8_t AUT			:1;
			uint8_t RES 		:2;
			uint8_t TH			:5;
		}thermBit;
	}therm;
	
	// TEMP register
	union{
		uint8_t	TEMP;
	}temp;
	
	// LED register
	union{
		uint8_t LED;
		struct{
			uint8_t LED1		:4;
			uint8_t LED2		:4;
		}ledBit;
	}led;
	
	// DCDC register
	union{
		uint8_t DCDC;
		struct{
			uint8_t DIS			:1;
			uint8_t BYP			:1;
			uint8_t FSET		:3;
			uint8_t VSET		:3;
		}dcdcBit;
	}dcdc;
	
	// DSTAT register
	union{
		uint8_t DSTAT;
		struct{
			uint8_t res			:5;
			uint8_t	LVL			:1;
			uint8_t SSC			:1;
			uint8_t res2		:1;
		}dstatBit;
	}dstat;
	
	// STATUS register
	union{
		uint8_t STATUS;
		struct{
			uint8_t RST			:1;
			uint8_t INT			:1;
			uint8_t	UV			:1;
			uint8_t DINT		:1;
			uint8_t CHK			:1;
			uint8_t DAT			:1;
			uint8_t SSC			:1;
			uint8_t SOT			:1;
		}statusBit;
	}status;
	
	uint8_t reserved[4];// Secrets hidden here?
	
	// LINK2 Register
	union{
		uint8_t LINK2;
		struct{
			uint8_t res 		:2;
			uint8_t CNT			:4;
			uint8_t END			:1;
			uint8_t SND			:1;
		}link2Bit;
	}link2;
	
}l6364_reg_t;

// L6364 FIFO data Register
typedef struct{
	uint8_t stat0;
	
	// Buffer register
	uint8_t FR[15];

}l6364_fr_t;

/* END REGSITER AND DATA STRUCTs*/

/* ENUMs*/
enum MSEQ_OD1{
	MSEQ_OD1_BC = 0,
	MSEQ_OD1_2B,
	MSEQ_OD1_8B
};

enum MSEQ_OD2{
	MSEQ_OD2_1B = 0,
	MSEQ_OD2_2B,
	MSEQ_OD2_8B
};

enum CFG_UVLO{
	UVLO_15V0 = 0,
	UVLO_14V0,
	UVLO_13V0,
	UVLO_12V0,
	UVLO_10V5,
	UVLO_8V5,
	UVLO_7V5,
	UVLO_6V0
};

enum COM_MODE{
	COM2 = 0,
	COM3
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

uint8_t l6364_readReg(uint8_t reg);
void l6364_writeReg(uint8_t reg, uint8_t data);

void l6364_getStatus();
uint8_t l6364_isRST();
uint8_t l6364_isINT();
uint8_t l6364_isUV();
uint8_t l6364_isDINT();
uint8_t l6364_isCHK();
uint8_t l6364_isDAT();
uint8_t l6364_isSSC();
uint8_t l6364_isSOT();

void l6364_setMseq(
	uint8_t m_len,		// Master message length (Including MC and CKT)
	uint8_t	od1_len,	// 
	uint8_t od2_len		//
	);
void l6364_setUVLO(uint8_t uvlo_val);
void l6364_setCOM(uint8_t com_mode);
void l6364_setLED1(uint8_t Iled);
void l6364_setLED2(uint8_t Iled);
void l6364_setIOLmode();
void l6364_setSIOmode();

// Polling-required code
// Periodic execution with timer or millis thingy super-loop
void l6364_pollAllReg();
uint8_t l6364_pollLinkCnt();
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







#endif