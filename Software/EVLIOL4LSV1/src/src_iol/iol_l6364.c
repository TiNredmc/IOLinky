#include "iol_l6364.h"

l6364_reg_t l6364_t;
l6364_fr_t	l6364_data_t;

uint8_t l6364_readReg(uint8_t reg){
	uint8_t tx[2], rx[2];
	tx[0] = reg;
	
	spi_WriteAndRead(tx, rx, 2);
	
	return rx[1];

}

void l6364_writeReg(uint8_t reg, uint8_t data){
	uint8_t tx[2];
	tx[0] = reg | (1 << 7);
	tx[1] = data;
	
	spi_WriteAndRead(tx, NULL, 2);
}

// Periordic polling register read.
void l6364_pollAllReg(){
	spi_WriteOnceAndRead(
		REG_MSEQ,
		(uint8_t *)&l6364_t,
		16);
	l6364_t.status.STATUS = l6364_t.status0.STATUS;
}

void l6364_getStatus(){
	spi_WriteOnceAndRead(
		REG_STATUS, 
		&l6364_t.status.STATUS, 
		1);
}

uint8_t l6364_isRST(){
	return l6364_t.status.statusBit.RST;
}

uint8_t l6364_isINT(){
	return l6364_t.status.statusBit.INT;
}

uint8_t l6364_isUV(){
	return l6364_t.status.statusBit.UV;
}

uint8_t l6364_isDINT(){
	return l6364_t.status.statusBit.DINT;
}

uint8_t l6364_isCHK(){
	return l6364_t.status.statusBit.CHK;
}

uint8_t l6364_isDAT(){
	return l6364_t.status.statusBit.DAT;
}

uint8_t l6364_isSSC(){
	return l6364_t.status.statusBit.SSC;
}

uint8_t l6364_isSOT(){
	return l6364_t.status.statusBit.SOT;
}


void l6364_setMseq(
	uint8_t m_len,		// Master message length (Including MC and CKT)
	uint8_t	od1_len,	// 
	uint8_t od2_len		//
	){
	
	l6364_t.mseq.mseqBit.M2CNT 	= m_len;
	l6364_t.mseq.mseqBit.OD1		=	od1_len;
	l6364_t.mseq.mseqBit.OD2		= od2_len;

	l6364_writeReg(
		REG_MSEQ,
		l6364_t.mseq.MSEQ
	);	
		
}

void l6364_setUVLO(uint8_t uvlo_val){
	
	l6364_t.cfg.cfgBit.UVT = uvlo_val;
	l6364_writeReg(
		REG_MSEQ,
		l6364_t.cfg.CFG
	);
	
}

void l6364_setCOM(uint8_t com_mode){
	
	l6364_t.cfg.cfgBit.BD = com_mode;
	l6364_writeReg(
		REG_MSEQ,
		l6364_t.cfg.CFG
	);
}

void l6364_setLED1(uint8_t Iled){
	l6364_t.led.ledBit.LED1 = Iled;
	l6364_writeReg(
		REG_LED,
		l6364_t.led.LED
	);
}

void l6364_setLED2(uint8_t Iled){
	l6364_t.led.ledBit.LED2 = Iled;
	l6364_writeReg(
		REG_LED,
		l6364_t.led.LED
	);
}

void l6364_setIOLmode(){
	l6364_t.cctl.cctlBit.SIO = 0;
	l6364_writeReg(
		REG_CCTL,
		l6364_t.cctl.CCTL
	);
}

void l6364_setSIOmode(){
	l6364_t.cctl.cctlBit.SIO = 1;
	l6364_writeReg(
		REG_CCTL,
		l6364_t.cctl.CCTL
	);
}

// Call after l6364_pollAllReg()
uint8_t l6364_pollLinkCnt(){
	return l6364_t.link.linkBit.CNT;
}

uint8_t l6364_getRxCount(){
	spi_WriteOnceAndRead(
		REG_LINK,
		(uint8_t *)&l6364_t.link.LINK,
		1
	);
	l6364_t.link2.LINK2 = l6364_t.link.LINK;
	
	return l6364_t.link.linkBit.CNT;
}

void l6364_readFIFO(
	uint8_t count, 
	uint8_t *output_ptr){
	
	uint8_t *fr_ptr = 
		(uint8_t *)(&l6364_data_t.FR);
		
	spi_WriteOnceAndRead(
		REG_FR0,
		(uint8_t *)&l6364_data_t,
		count
	);
		
	l6364_t.status.STATUS = l6364_data_t.stat0;
		
	while(count--){
		*output_ptr++ = *fr_ptr++;
	}
}

void l6364_setTXCount(uint8_t count){
	// Update current data of LINK register
	l6364_getRxCount();
	l6364_t.link.linkBit.CNT = count;
	l6364_t.link.linkBit.SND = 1;
	l6364_t.link2.LINK2 = l6364_t.link.LINK;
	
	l6364_writeReg(
		REG_LINK,
		l6364_t.link.LINK
	);
}

void l6364_writeFIFO(
	uint8_t count,
	uint8_t *input_ptr){
	
	uint8_t *fr_prt = 
		(uint8_t *)(&l6364_data_t.FR);

	spi_WriteOnceAndWrite(
		REG_FR0,
		input_ptr,
		count
	);

	// Copy data to shadow register
	while(count--){
		*fr_prt++ = *input_ptr++;
	}	
}












