#include "app_nvm.h"

// Private variables
uint8_t cal_unlock = 0;

// Private typedefs

// Default Calibration value and Efuse trip point
__IO const flash_data_t real_flash_t __attribute__((section(".store_config"))) = {
	// Isense
	.ADC5.ADC_SCALING = 4080,
	.ADC5.ADC_OFFSET = 0,
	
	// VOsense
	.ADC6.ADC_SCALING = 4080,
	.ADC6.ADC_OFFSET = 1,
	
	// VIsense
	.ADC7.ADC_SCALING = 4125,
	.ADC7.ADC_OFFSET = 1,
	
	// I2T Holding Current at specific period
	.I2T_AmpHold = 2172,			// Default 3.5A
	.I2T_PeriodHold = 6000		// Default 6000ms
};

flash_data_t temp_data_t;


void app_nvm_init(){
	temp_data_t.ADC5.ADC_SCALING = 
		real_flash_t.ADC5.ADC_SCALING;
	temp_data_t.ADC5.ADC_OFFSET = 
		real_flash_t.ADC5.ADC_OFFSET;
	
	temp_data_t.ADC6.ADC_SCALING = 
		real_flash_t.ADC6.ADC_SCALING;
	temp_data_t.ADC6.ADC_OFFSET = 
		real_flash_t.ADC6.ADC_OFFSET;
	
	temp_data_t.ADC7.ADC_SCALING = 
		real_flash_t.ADC7.ADC_SCALING;
	temp_data_t.ADC7.ADC_OFFSET = 
		real_flash_t.ADC7.ADC_OFFSET;
	
	temp_data_t.I2T_AmpHold = 
		real_flash_t.I2T_AmpHold;
	temp_data_t.I2T_PeriodHold = 
		real_flash_t.I2T_PeriodHold;
}

void app_nvm_unlockFlash(){
	
	// Unlock the flash if it was locked
	if(FMC_CTL & FMC_CTL_LK){
		// Write unlock key
		FMC_KEY = UNLOCK_KEY0;
		FMC_KEY = UNLOCK_KEY1;

		while(FMC_CTL & FMC_CTL_LK);
	}

}

void app_nvm_lockFlash(){
	if(FMC_CTL & FMC_CTL_LK)
		return;
	
	FMC_CTL |= FMC_CTL_LK;
}

uint8_t app_nvm_erasePage(){
	if(FMC_CTL & FMC_CTL_LK)
		return 1;

	while((FMC_STAT & FMC_STAT_BUSY));
	
	FMC_CTL |= FMC_CTL_PER;// Set page erase command
	
	FMC_ADDR = FLASH_CONFIG_BASE;// Erase page 63
	
	FMC_CTL |= FMC_CTL_START;// Start erasing

	while((FMC_STAT & FMC_STAT_BUSY));
	
	while(!(FMC_STAT & FMC_STAT_ENDF));

	FMC_STAT |= FMC_STAT_ENDF;
	
	FMC_CTL &= ~FMC_CTL_PER;	
	
	return 0;
}

uint8_t app_nvm_writeData(){
	if(FMC_CTL & FMC_CTL_LK)
		return 1;

	while((FMC_STAT & FMC_STAT_BUSY));
	
	FMC_CTL |= FMC_CTL_PG;
	
	{
		FLASH_DATA->ADC5.ADC_SCALING = 
			temp_data_t.ADC5.ADC_SCALING;
		__ISB();
		if(FMC_STAT & FMC_STAT_PGERR)
			goto bad_exit;
	}
	
	{
		FLASH_DATA->ADC5.ADC_OFFSET = 
			temp_data_t.ADC5.ADC_OFFSET;
		__ISB();
		if(FMC_STAT & FMC_STAT_PGERR)
			goto bad_exit;
	}
	
	{
		FLASH_DATA->ADC6.ADC_SCALING = 
			temp_data_t.ADC6.ADC_SCALING;
		__ISB();
		if(FMC_STAT & FMC_STAT_PGERR)
			goto bad_exit;
	}
	
	{
		FLASH_DATA->ADC6.ADC_OFFSET = 
			temp_data_t.ADC6.ADC_OFFSET;
		__ISB();
		if(FMC_STAT & FMC_STAT_PGERR)
			goto bad_exit;
	}
	
	{
		FLASH_DATA->ADC7.ADC_SCALING = 
			temp_data_t.ADC7.ADC_SCALING;
		__ISB();
		if(FMC_STAT & FMC_STAT_PGERR)
			goto bad_exit;
	}
	
	{
		FLASH_DATA->ADC7.ADC_OFFSET = 
			temp_data_t.ADC7.ADC_OFFSET;
		__ISB();
		if(FMC_STAT & FMC_STAT_PGERR)
			goto bad_exit;
	}
	
	{
		FLASH_DATA->I2T_AmpHold = 
			temp_data_t.I2T_AmpHold;
		__ISB();
		if(FMC_STAT & FMC_STAT_PGERR)
			goto bad_exit;
	}
	
	{
		FLASH_DATA->I2T_PeriodHold = 
			temp_data_t.I2T_PeriodHold;
		__ISB();
		if(FMC_STAT & FMC_STAT_PGERR)
			goto bad_exit;
	}
	
	while((FMC_STAT & FMC_STAT_BUSY));
	
	while(!(FMC_STAT & FMC_STAT_ENDF));

	FMC_STAT |= FMC_STAT_ENDF;
	
	FMC_CTL &= ~FMC_CTL_PG;
	
	return 0;	
	
bad_exit:	
	FMC_STAT |= FMC_STAT_PGERR;
	app_nvm_lockFlash();
	return 1;
}

uint8_t app_nvm_verifyData(){
	for(uint8_t i = 0; i < 7; i ++){
		if(
			*((uint32_t *)&real_flash_t + i) !=
			*((uint32_t *)&temp_data_t + i)
		)
			return 1;
	}
	
	return 0;
}

uint8_t app_nvm_updateData(){
	uint8_t ret = 0;
	
	// Entering critical section
	__disable_irq();
	app_nvm_unlockFlash();
	
	if(app_nvm_erasePage()){
		ret = 1;
	}else if(app_nvm_writeData()){
		ret  = 2;
	}else if(app_nvm_verifyData()){
		ret = 3;
	}
	
	app_nvm_lockFlash();
	__enable_irq();

	return 0;
}

uint8_t app_nvm_setAccessUnlock(uint32_t unlock_code){
	if(NVM_PASSWORD == unlock_code){
		cal_unlock = 1;
		return 0;
	}
	
	if(NVM_UPDATE == unlock_code){
		if(cal_unlock == 1){
			cal_unlock = 0;
			return app_nvm_updateData();
		}		
	}
	
	return 254;
}

uint8_t app_nvm_getAccessUnlock(){
	return cal_unlock;
}

// Write Efuse Amp hold value to flash
void app_nvm_setI2TAmpHold(uint32_t Ihold){
	if(FLASH_DATA->I2T_AmpHold != Ihold)
		temp_data_t.I2T_AmpHold = Ihold;
}

// Write Efuse Time (period) hold value to flash
void app_nvm_setI2TPeriodHold(uint32_t Thold){
	if(FLASH_DATA->I2T_PeriodHold != Thold)
		temp_data_t.I2T_PeriodHold = Thold;
}

// ADC5 - Isense ADC scaling
void app_nvm_setADC5Scaling(int32_t scale){
	if(FLASH_DATA->ADC5.ADC_SCALING != scale)
		temp_data_t.ADC5.ADC_SCALING = (uint32_t)scale;
}	

// ADC5 - Isense ADC offset
void app_nvm_setADC5Offset(int32_t offset){
	if(FLASH_DATA->ADC5.ADC_OFFSET != offset)
		temp_data_t.ADC5.ADC_OFFSET = (uint32_t)offset;
}

// ADC6 - VOsense ADC scaling
void app_nvm_setADC6Scaling(int32_t scale){
	if(FLASH_DATA->ADC6.ADC_SCALING != scale)
		temp_data_t.ADC6.ADC_SCALING = (uint32_t)scale;
}	

// ADC6 - VOsense ADC offset
void app_nvm_setADC6Offset(int32_t offset){
	if(FLASH_DATA->ADC6.ADC_OFFSET != offset)
		temp_data_t.ADC6.ADC_OFFSET = (uint32_t)offset;
}

// ADC7 - VIsense ADC scaling
void app_nvm_setADC7Scaling(int32_t scale){
	if(FLASH_DATA->ADC7.ADC_SCALING != scale)
		temp_data_t.ADC7.ADC_SCALING = (uint32_t)scale;
}	

// ADC7 - VIsense ADC offset
void app_nvm_setADC7Offset(int32_t offset){
	if(FLASH_DATA->ADC7.ADC_OFFSET != offset)
		temp_data_t.ADC7.ADC_OFFSET = (uint32_t)offset;
}
