#ifndef IOL_PL_H
#define IOL_PL_H

#include <stdint.h>
#include "iol_l6362.h"

#include "tim0_led.h"

enum current_mtype_e{
	MTYPE_0 = 0,
	MTYPE_1_2,
	MTYPE_2_2,
	MTYPE_2_V_8PDI
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

void iol_pl_standbyLED();
void iol_pl_connectedLED();
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
