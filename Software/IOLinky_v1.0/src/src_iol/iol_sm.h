#ifndef IOL_SM_H
#define IOL_SM_H

#include <stdint.h>

#include "iol_pl.h"
#include "iol_dl.h"
#include "iol_al.h"

enum IOL_COM_MODE{
	COM1 = 0,
	COM2,
	COM3,
};


void iol_sm_phyInit();

void iol_sm_init(
	uint8_t commode,
	uint8_t *pd_data_ptr);

#endif