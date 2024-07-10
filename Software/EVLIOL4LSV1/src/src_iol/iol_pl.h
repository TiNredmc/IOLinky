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
void iol_pl_aliveLED();
void iol_pl_poll();
void iol_pl_dlReadBuffer();
void iol_pl_dlWriteBuffer();
uint8_t iol_pl_getCurrentMtype();
void iol_pl_setMtype0();
void iol_pl_setMtype1_2();



#endif
