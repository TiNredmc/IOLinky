#ifndef APP_NVM_H
#define APP_NVM_H

#include "gd32e23x.h"
#include "gd32e23x_fmc.h"

#define FLASH_CONFIG_BASE		0x0800FC00UL

// Unlock password to allow calibration value from ISDU
#define NVM_PASSWORD	12345678UL
#define NVM_UPDATE		87654321UL

// Store ADC calibration value
typedef struct{
	uint32_t ADC_SCALING;
	uint32_t ADC_OFFSET;
}adccal_const_t;


typedef struct{
	// ADC calibration value
	adccal_const_t ADC5;
	adccal_const_t ADC6;
	adccal_const_t ADC7;
	// I2T Efuse setting
	uint32_t I2T_AmpHold;
	uint32_t I2T_PeriodHold;
}flash_data_t;

#define FLASH_DATA	((__IO flash_data_t *)FLASH_CONFIG_BASE)

void app_nvm_init();

uint8_t app_nvm_updateData();

uint8_t app_nvm_setAccessUnlock(uint32_t unlock_code);
uint8_t app_nvm_getAccessUnlock();

void app_nvm_setI2TAmpHold(uint32_t Ihold);
void app_nvm_setI2TPeriodHold(uint32_t Thold);
void app_nvm_setADC5Scaling(int32_t scale);
void app_nvm_setADC5Offset(int32_t offset);
void app_nvm_setADC6Scaling(int32_t scale);
void app_nvm_setADC6Offset(int32_t offset);
void app_nvm_setADC7Scaling(int32_t scale);
void app_nvm_setADC7Offset(int32_t offset);

#endif