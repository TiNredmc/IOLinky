#ifndef IOL_PL_H
#define IOL_PL_H

#include <stdint.h>

typedef struct{
	void (*phy_init)(void);
	uint8_t (*phy_getParityError)(void);
	uint8_t (*phy_readFIFO)(
		uint8_t *read_ptr);
	
	uint8_t (*phy_writeFIFO)(
		uint8_t write_count,
		uint8_t *write_ptr
		);
	
	void (*phy_setMsequenceLength)(
		uint8_t master_data_len,
		uint8_t master_od_len,
		uint8_t device_od_len
		);
	
}iol_pl_handler_t;

enum current_mtype_e{
	MTYPE_0 = 0,
	MTYPE_1_2,
	MTYPE_2_2,
	MTYPE_2_V_8PDI
};

void iol_pl_init(
	iol_pl_handler_t *pl_pHandler_t
	);

void iol_pl_update_ReadBuffer(uint8_t *rd);
void iol_pl_update_WriteBuffer(uint8_t *wr);
void iol_pl_updateBuffer(
	uint8_t *rd_buffer_ptr,
	uint8_t *wr_buffer_ptr
	);

void iol_pl_pollRead();
void iol_pl_pollWrite();

// Available for IOL Data Link layer
uint8_t iol_pl_ReadAvailable();
void iol_pl_WriteRequest(uint8_t count);
uint8_t iol_pl_checkWriteStatus();

uint8_t iol_pl_getCurrentMtype();
void iol_pl_setMtype0();
void iol_pl_setMtype1_2();
void iol_pl_setMtype2_2();
void iol_pl_setMtype2_V_8PDI();

#endif
