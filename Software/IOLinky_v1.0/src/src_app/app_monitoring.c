#include "app_psu.h"

// Private variables
uint32_t Efuse_integrator = 0;
uint32_t Efuse_Delta = 0;
int32_t Efuse_Sigma = 0;
uint32_t Efuse_DifferentOfSquare = 0;

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
				psu_mondata_t.PSU_status_b.IOut_ok = 1;
				psu_mondata_t.PSU_status_b.IOut_OC = 1;
				psu_mondata_t.PSU_status_b.IOut_SC = 0;
				psu_mondata_t.PSU_status_b.Efuse_Act = 1;
				psu_mondata_t.PSU_status_b.Efuse_Trip = 1;
			}else{
				// Less than High current
				if(psu_mondata_t.IOsense_val > THRESHOLD_IO_PEAK){
					// More than Peak current -> OC activate Efuse
					psu_mondata_t.PSU_status_b.IOut_ok = 1;
					psu_mondata_t.PSU_status_b.IOut_OC = 1;
					psu_mondata_t.PSU_status_b.IOut_SC = 0;
					psu_mondata_t.PSU_status_b.Efuse_Act = 1;
					psu_mondata_t.PSU_status_b.Efuse_Trip = 0;
				}else{
					// nominal < I < Peak -> ok
					psu_mondata_t.PSU_status_b.IOut_ok = 1;
					psu_mondata_t.PSU_status_b.IOut_OC = 0;
					psu_mondata_t.PSU_status_b.IOut_SC = 0;
					psu_mondata_t.PSU_status_b.Efuse_Act = 0;
					psu_mondata_t.PSU_status_b.Efuse_Trip = 0;
				}
			}
		
		}else{
			// Less than Nominal current -> OK
			psu_mondata_t.PSU_status_b.IOut_ok = 1;
			psu_mondata_t.PSU_status_b.IOut_OC = 0;
			psu_mondata_t.PSU_status_b.IOut_SC = 0;
			psu_mondata_t.PSU_status_b.Efuse_Act = 0;
			psu_mondata_t.PSU_status_b.Efuse_Trip = 0;
		}
	}else{
		if(psu_mondata_t.IOsense_val > THRESHOLD_IO_SC){
			// Short circuit detection
			psu_mondata_t.PSU_status_b.IOut_ok = 0;
			psu_mondata_t.PSU_status_b.IOut_OC = 1;
			psu_mondata_t.PSU_status_b.IOut_SC = 1;
		}else{
			// In case of Output turned off
			psu_mondata_t.PSU_status_b.IOut_ok = 0;
			psu_mondata_t.PSU_status_b.IOut_OC = 0;
			psu_mondata_t.PSU_status_b.IOut_SC = 0;
		}
	}
	
}

void app_mon_efuseRunner(){
	if(
	psu_mondata_t.PSU_status_b.IOut_ok && 
	psu_mondata_t.PSU_status_b.IOut_OC &&
	!psu_mondata_t.PSU_status_b.IOut_SC &&
	psu_mondata_t.PSU_status_b.Efuse_Act &&
	!psu_mondata_t.PSU_status_b.Efuse_Trip 
	){
		Efuse_Sigma = 
			psu_mondata_t.IOsense_val + 
			THRESHOLD_IO_NOM;
		
		Efuse_Delta = 
			(int32_t)psu_mondata_t.IOsense_val -
			THRESHOLD_IO_NOM;
		
		Efuse_DifferentOfSquare = 
			(Efuse_Delta * Efuse_Delta);
		Efuse_DifferentOfSquare = 
			Efuse_DifferentOfSquare  >> 10;
		
		Efuse_integrator += Efuse_DifferentOfSquare;
		
		if(Efuse_integrator > THRESHOLD_EFUSE){
			psu_mondata_t.PSU_status_b.Efuse_Trip = 1;
			psu_mondata_t.PSU_status_b.IOut_ok = 0;
		}
		
	}
}

void app_mon_efuseReset(){
	Efuse_integrator = 0;
}

void app_mon_updateMonitoring(){
	if(adc_getDataAvaible()){
		app_mon_checkVin();
		app_mon_checkVout();
		app_mon_checkIout();
		app_mon_efuseRunner();
		
		adc_softTrigger();// Trigger next conversion
	}
}