#include "app_psu.h"

uint8_t Imon_delay = 0;
uint8_t Vmon_delay = 0;

// Private variables
uint16_t Efuse_Sigma = 0;
int16_t Efuse_Delta = 0;
int32_t Efuse_DifferentOfSquare = 0;
int32_t Efuse_integrator = 0;
int32_t Efuse_threshold = THRESHOLD_EFUSE;

void app_mon_applyADCCal(){
	// Filter data first
	psu_mondata_t.IOsense_val += psu_rawmondata_t.IOsense_raw;
	psu_mondata_t.IOsense_val -= 
		psu_mondata_t.IOsense_val >> 1;
	
	psu_mondata_t.VOsense_val += psu_rawmondata_t.VOsense_raw;
	psu_mondata_t.VOsense_val -= 
		psu_mondata_t.VOsense_val >> 1;
	
	psu_mondata_t.VIsense_val += psu_rawmondata_t.VIsense_raw;
	psu_mondata_t.VIsense_val -= 
		psu_mondata_t.VIsense_val >> 1;
	
	// Then apply the calibration 
	psu_mondata_t.IOsense_val = (int16_t)(
		(int32_t)(
			((int16_t)FLASH_DATA->ADC5.ADC_SCALING *
				psu_mondata_t.IOsense_val)
		) >> 12) + FLASH_DATA->ADC5.ADC_OFFSET;
	
	psu_mondata_t.VOsense_val = (int16_t)( 
		(int32_t)(
			((int16_t)FLASH_DATA->ADC6.ADC_SCALING *
				psu_mondata_t.VOsense_val)
		)	>> 12) + FLASH_DATA->ADC6.ADC_OFFSET;
	
	psu_mondata_t.VIsense_val = (int16_t)(
		(int32_t)(
			((int16_t)FLASH_DATA->ADC7.ADC_SCALING *
				psu_mondata_t.VIsense_val)
		) >> 12) + FLASH_DATA->ADC7.ADC_OFFSET;

}

void app_mon_checkVin(){
	// Input Undervoltage check
	if(psu_mondata_t.PSU_status_b.VIn_UV == 0){
		if(psu_mondata_t.VIsense_val < THRESHOLD_VIN_LO_DN){
			psu_mondata_t.PSU_status_b.VIn_UV = 1;
			psu_mondata_t.PSU_status_b.VIn_OV = 0;
		}
	}else{
		if(psu_mondata_t.VIsense_val > THRESHOLD_VIN_LO_UP){
			psu_mondata_t.PSU_status_b.VIn_UV = 0;
		}
	}
	
	// Input Overvoltage check
	if(psu_mondata_t.VIsense_val > THRESHOLD_VIN_HI){
		psu_mondata_t.PSU_status_b.VIn_OV = 1;
		psu_mondata_t.PSU_status_b.VIn_UV = 0;
	}else{
		psu_mondata_t.PSU_status_b.VIn_OV = 0;
	}
	
	// Determine whether input is ok or not
	if(
		!(psu_mondata_t.PSU_status_b.VIn_UV) &&
		!(psu_mondata_t.PSU_status_b.VIn_OV)
	)
		psu_mondata_t.PSU_status_b.VIn_ok = 1;
	else
		psu_mondata_t.PSU_status_b.VIn_ok = 0;
	
}

void app_mon_checkVout(){
	if(psu_mondata_t.VOsense_val > THRESHOLD_VO_HI){
		psu_mondata_t.PSU_status_b.VOut_ok = 0;
		psu_mondata_t.PSU_status_b.VOut_UV = 0;
		psu_mondata_t.PSU_status_b.VOut_OV = 1;
	}else{
		if(psu_mondata_t.PSU_status_b.Buck_en){
			if(
				(psu_mondata_t.VOsense_val > THRESHOLD_VO_LO) &&
				(psu_mondata_t.VOsense_val < THRESHOLD_VO_HI)
			){
				psu_mondata_t.PSU_status_b.VOut_ok = 1;
				psu_mondata_t.PSU_status_b.VOut_UV = 0;
				psu_mondata_t.PSU_status_b.VOut_OV = 0;
			}else if(psu_mondata_t.VOsense_val < THRESHOLD_VO_LO){
				psu_mondata_t.PSU_status_b.VOut_ok = 0;
				psu_mondata_t.PSU_status_b.VOut_UV = 1;
				psu_mondata_t.PSU_status_b.VOut_OV = 0;
			}
		}else{
			psu_mondata_t.PSU_status_b.VOut_ok = 0;
			psu_mondata_t.PSU_status_b.VOut_UV = 0;
			psu_mondata_t.PSU_status_b.VOut_OV = 0;
		}
	}	
	
}

void app_mon_checkIout(){
	if(psu_mondata_t.PSU_status_b.VOut_ok){
		// Vout is in good shape
		if(psu_mondata_t.IOsense_val > THRESHOLD_IO_NOM){
			// More than Nominal current
			if(psu_mondata_t.IOsense_val > THRESHOLD_IO_HI){
				// More than High current -> Trip Efuse
				Imon_delay++;
				
				// 10ms IO high spike
				if(Imon_delay > 10){
					Imon_delay = 0;
					psu_mondata_t.PSU_status_b.IOut_ok = 0;
					psu_mondata_t.PSU_status_b.IOut_OC = 1;
					psu_mondata_t.PSU_status_b.IOut_SC = 0;
				}
				
			}else{
				// Less than High current
				if(psu_mondata_t.IOsense_val > THRESHOLD_IO_PEAK){
					// More than Peak current -> OC activate Efuse
					Imon_delay++;
					
					// 10ms IO PEAK
					if(Imon_delay > 10){
						Imon_delay = 0;
						psu_mondata_t.PSU_status_b.IOut_ok = 1;
						psu_mondata_t.PSU_status_b.IOut_OC = 1;
						psu_mondata_t.PSU_status_b.IOut_SC = 0;
					}
				}else{
					// nominal < I < Peak -> ok
					psu_mondata_t.PSU_status_b.IOut_ok = 1;
					psu_mondata_t.PSU_status_b.IOut_OC = 0;
					psu_mondata_t.PSU_status_b.IOut_SC = 0;
					
					Imon_delay = 0;
				}
				
			}
		
		}else{
			// Less than Nominal current -> OK
			psu_mondata_t.PSU_status_b.IOut_ok = 1;
			psu_mondata_t.PSU_status_b.IOut_OC = 0;
			psu_mondata_t.PSU_status_b.IOut_SC = 0;
			
			Imon_delay = 0;
		}
	}else{
		if(
			psu_mondata_t.PSU_status_b.Buck_en			&&
			psu_mondata_t.IOsense_val > THRESHOLD_IO_SC){
			// Short circuit detection
			psu_mondata_t.PSU_status_b.IOut_ok = 0;
			psu_mondata_t.PSU_status_b.IOut_OC = 1;
			psu_mondata_t.PSU_status_b.IOut_SC = 1;
			psu_mondata_t.PSU_status_b.Efuse_Act = 1;
			psu_mondata_t.PSU_status_b.Efuse_Trip = 1;
		}else{
			// In case of Output turned off
			psu_mondata_t.PSU_status_b.IOut_ok = 0;
			psu_mondata_t.PSU_status_b.IOut_OC = 0;
			psu_mondata_t.PSU_status_b.IOut_SC = 0;
			psu_mondata_t.PSU_status_b.Efuse_Act = 0;
			psu_mondata_t.PSU_status_b.Efuse_Trip = 0;
		}
	}
	
}

void app_mon_efuseReset(){
	Efuse_integrator = 0;
	psu_mondata_t.PSU_status_b.Efuse_Act = 0;
	psu_mondata_t.PSU_status_b.Efuse_Trip = 0;
}

int32_t app_mon_getEfuse(){
	return Efuse_integrator;
}

// Efuse I2t algorithm
void app_mon_efuseRunner(){
		
		// Stop counting when Efuse was already tripped.
		if(psu_mondata_t.PSU_status_b.Efuse_Trip)
			return;
		
		Efuse_Sigma = 
			psu_mondata_t.IOsense_val + 
			THRESHOLD_IO_I2T;
		
		Efuse_Delta = 
			(int16_t)(
			psu_mondata_t.IOsense_val -
			THRESHOLD_IO_I2T
			);
		
		Efuse_DifferentOfSquare = 
			(int32_t)(Efuse_Sigma * Efuse_Delta) >> 
			10; // Divided by 1024, approximation of 1000ms
		
		Efuse_integrator += Efuse_DifferentOfSquare;
		
		// Cap lower end when current consumption
		// returns to lower that Inom
		if(Efuse_integrator < 0){
			Efuse_integrator = 0;
			psu_mondata_t.PSU_status_b.Efuse_Act = 0;
			psu_mondata_t.PSU_status_b.Efuse_Trip = 0;
		}else{
			psu_mondata_t.PSU_status_b.Efuse_Act = 1;
		}
		
		if(Efuse_integrator > Efuse_threshold){
			psu_mondata_t.PSU_status_b.Efuse_Trip = 1;
			psu_mondata_t.PSU_status_b.IOut_ok = 0;
		}
}

void app_mon_updateEfuseThreshold(){
	uint16_t dr = 0;
	uint16_t e_sigma = 0;
	int16_t e_delta = 0;
	
	dr = (uint16_t)FLASH_DATA->I2T_AmpHold;
	e_sigma =
		dr + THRESHOLD_IO_NOM;
	
	e_delta = 
		(int16_t)(
		dr - THRESHOLD_IO_NOM
		);
	
	Efuse_threshold = (
		(int32_t)(e_sigma * e_delta) >> 10
	) * FLASH_DATA->I2T_PeriodHold;
}

void app_mon_updateMonitoring(){
	if(adc_getDataAvaible()){
		app_mon_applyADCCal();
		app_mon_checkVin();
		app_mon_checkIout();
		app_mon_checkVout();
		app_mon_efuseRunner();
		
		adc_softTrigger();// Trigger next conversion
	}
}