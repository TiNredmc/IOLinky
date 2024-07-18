#ifndef IOL_PL_H
#define IOL_PL_H

#include <stdint.h>
#include "iol_l6364.h"

enum current_mtype_e{
	MTYPE_0 = 0,
	MTYPE_1_2,

};

void iol_pl_init(
	uint8_t *rd_buffer_ptr,
	uint8_t *wr_buffer_ptr
	);

void iol_pl_update_ReadBuffer(uint8_t *rd);
void iol_pl_update_WriteBuffer(uint8_t *wr);
void iol_pl_updateBuffer(
	uint8_t *rd_buffer_ptr,
	uint8_t *wr_buffer_ptr
	);

void iol_pl_aliveLED();
void iol_pl_pollRead();
void iol_pl_pollWrite();

// Available for IOL Data Link layer
uint8_t iol_pl_ReadAvailable();
void iol_pl_WriteRequest(uint8_t count);
uint8_t iol_pl_checkWriteStatus();
void iol_pl_LinkSND();
void iol_pl_LinkEND();

uint8_t iol_pl_getCurrentMtype();
void iol_pl_setMtype0();
void iol_pl_setMtype1_2();



#endif