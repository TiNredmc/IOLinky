#include "app_psu.h"

// Private variables
uint8_t PSU_fsm = 0;

uint16_t cycledelay_sc_trip = 0;
uint16_t cycledelay_vout_not_ok = 0;

uint16_t sc_trip_cnt = 0;

uint16_t hiccup_delay = 0;

uint8_t buck_pwrOn_flag = 0;
uint8_t buck_pwrOff_flag = 0;


// Private typedefs
psu_data_t psu_mondata_t;

// Private prototype
void app_mon_updateMonitoring();
void app_mon_efuseReset();

void app_psu_init(){
	// Setup ADC
	adc_initScanDMA(
		(uint16_t*)(&psu_mondata_t.IOsense_val)
	);
	adc_softTrigger();// Initial ADC conversion
}

void app_psu_enableBuck(){
	psu_mondata_t.PSU_status_b.Buck_en = 1;
	GPIO_OCTL(GPIOB) |= (1 << ENBuck_Pin);
}

void app_psu_disableBuck(){
	psu_mondata_t.PSU_status_b.Buck_en = 0;
	GPIO_OCTL(GPIOB) &= ~(1 << ENBuck_Pin);
}

void app_psu_requestPWRON(){
	buck_pwrOn_flag = 1;
	buck_pwrOff_flag = 0;
}

void app_psu_requestPWROFF(){
	buck_pwrOn_flag = 0;
	buck_pwrOff_flag = 1;
}

void app_psu_powerCMD(){
	if(!buck_pwrOn_flag && !buck_pwrOff_flag)
		return;
	
	if(buck_pwrOn_flag){
		buck_pwrOn_flag = 0;// clear flag
		app_psu_enableBuck();
	}else if(buck_pwrOff_flag){
		buck_pwrOff_flag = 0;// clear flag
		app_psu_disableBuck();
	}

}

void app_psu_runner(){
	
	app_mon_updateMonitoring();
	
	switch(PSU_fsm){
		// State : Initialize
		// Cause : Power on and app started
		case PSU_STATE_INIT:// PSU initialize state
		{
			// Init all Status bit
			app_mon_efuseReset();
			PSU_fsm = PSU_STATE_IDLE;
		}
		break;
	
		// State : Idle
		// Cause : Startup and wait for Vin to reach ok
		case PSU_STATE_IDLE:
		{
			if(
				(
				!psu_mondata_t.PSU_status_b.VIn_OV 		&& // No Vin overvolt
				!psu_mondata_t.PSU_status_b.VIn_UV 		&& // No Vin undervolt
				!psu_mondata_t.PSU_status_b.Efuse_Act	&& // Efuse is not counting
				!psu_mondata_t.PSU_status_b.Efuse_Trip   // No Efuse tripped
				
				)	// Check for not having bad stuffs	
			&&
				(
					psu_mondata_t.PSU_status_b.VIn_ok
				)	// Check for having good stuffs
			){
				// If good to go, enable buck converter.
				app_psu_enableBuck();
				app_mon_efuseReset();
				PSU_fsm = PSU_STATE_NORMAL;
			}
		}
		break;
		
		// State : Normal
		// Cause : Initialization 
		case PSU_STATE_NORMAL:
		{
			// Short circuit detection
			if(
			!psu_mondata_t.PSU_status_b.VOut_ok && // Vout not ok
			!psu_mondata_t.PSU_status_b.IOut_ok	&& // Iout not ok
			psu_mondata_t.PSU_status_b.IOut_SC		 // Short circuit detected
			){
				PSU_fsm = PSU_STATE_SC;
				break;
			}
			
			// Check if Vin is not OK
			if(
			!psu_mondata_t.PSU_status_b.VIn_ok
			){
				app_psu_disableBuck();
				PSU_fsm = PSU_STATE_VIN_NOK;
				break;
			}
			
			// Check Vout error
			if(
			!psu_mondata_t.PSU_status_b.VOut_ok			
			){
				PSU_fsm = PSU_STATE_VOUT_NOK;
				
				break;
			}
			
			// Check overcurrent and Efuse
			if(
			psu_mondata_t.PSU_status_b.IOut_ok &&
			psu_mondata_t.PSU_status_b.IOut_OC &&
			psu_mondata_t.PSU_status_b.Efuse_Act
			){
				PSU_fsm = PSU_STATE_OLP;
				break;
			}
			
			// If powered off (by IO-Link command)
			if(!psu_mondata_t.PSU_status_b.Buck_en){
				PSU_fsm = PSU_STATE_PWROFF;
			}
			
		}
		break;
		
		// State : Vout is not ok
		// Cause : Over voltage
		case PSU_STATE_VOUT_NOK:
		{
			cycledelay_vout_not_ok++;
			
			if(
			psu_mondata_t.PSU_status_b.VOut_ok 	&&
			psu_mondata_t.PSU_status_b.IOut_ok	&&
			!psu_mondata_t.PSU_status_b.IOut_SC
			){
				cycledelay_vout_not_ok = 0;// Reset SC trip
				PSU_fsm = PSU_STATE_NORMAL;
			}else{
				// Overvoltage more than TIMEOUT_VO_OV
				// Trip and enter halt
				if(cycledelay_vout_not_ok > TIMEOUT_VO_OV){
					cycledelay_vout_not_ok = 0;// Reset SC trip
					app_psu_disableBuck();
					PSU_fsm = PSU_STATE_HALT;
				}
			}
			
		}
		break;
		
		// State : Vin is not ok
		// Cause : Under or Over voltage
		case PSU_STATE_VIN_NOK:
		{
			if(
				(
				!psu_mondata_t.PSU_status_b.VIn_OV 		&& // No Vin overvolt
				!psu_mondata_t.PSU_status_b.VIn_UV 		&& // No Vin undervolt
				!psu_mondata_t.PSU_status_b.Efuse_Act	&& // Efuse is not counting
				!psu_mondata_t.PSU_status_b.Efuse_Trip   // No Efuse tripped
				
				)	// Check for bad stuffs	
			&
				(
					psu_mondata_t.PSU_status_b.VIn_ok
				)	// Check for good stuffs
			){
				// If good to go, enable buck converter.
				app_mon_efuseReset();
				app_psu_enableBuck();
				PSU_fsm = PSU_STATE_NORMAL;
			}else{
				// TODO : timeout
				
			}
			
		}
		break;
		
		// State : Over Load Protection
		// Cause : Overcurrent and Efuse kicked in
		case PSU_STATE_OLP:
		{
			if(
				psu_mondata_t.PSU_status_b.Efuse_Act &&
				!psu_mondata_t.PSU_status_b.Efuse_Trip
			){// Efuse integrating but not tripped yet.
				// Wait
			
				PSU_fsm = PSU_STATE_OLP;
			}else if(
				psu_mondata_t.PSU_status_b.Efuse_Act &&
				psu_mondata_t.PSU_status_b.Efuse_Trip
			){// Efuse tripped, enter halt (for now)
				// TODO : implement hiccup mode
				PSU_fsm = PSU_STATE_HALT;
				
			
			}else{// Efuse back to normal
				app_mon_efuseReset();
				PSU_fsm = PSU_STATE_NORMAL;
			}
			
			
		}
		break;
		
		// State : Short circuit
		// Cause : Short circuit (Low Vout and High Iout)
		case PSU_STATE_SC:
		{
			cycledelay_sc_trip++;
			
			// If SC was gone before TIMEOUT_SC 
			// return to run mode
			if(
			psu_mondata_t.PSU_status_b.VOut_ok 	&&
			psu_mondata_t.PSU_status_b.IOut_ok	&&
			psu_mondata_t.PSU_status_b.IOut_SC
			){
				cycledelay_sc_trip = 0;// Reset SC trip
				app_mon_efuseReset();
				PSU_fsm = PSU_STATE_NORMAL;
			}else{
				// Short circuit more than TIMEOUT_SC
				// Shutdown buck converter and 
				// enter halt state
				if(cycledelay_sc_trip > TIMEOUT_SC){
					cycledelay_sc_trip = 0;// Reset SC trip
					app_psu_disableBuck();
					PSU_fsm = PSU_STATE_HALT;
				}
			}
			
		}
		break;
		
		// State : Halt
		// Cause : Short circuit, Vin or Vout fault
		case PSU_STATE_HALT:
		{
			hiccup_delay++;
			
			if(hiccup_delay == 3000){
				hiccup_delay = 0;
				app_mon_efuseReset();
				app_psu_enableBuck();
				PSU_fsm = PSU_STATE_NORMAL;
			}
		}
		break;
		
		// State : Powered off
		// Cuase : IO-Link command
		case PSU_STATE_PWROFF:
		{
			// If powered on
			if(
				psu_mondata_t.PSU_status_b.VIn_ok &&
				psu_mondata_t.PSU_status_b.Buck_en){
				PSU_fsm = PSU_STATE_INIT;
			}else{
				// TODO : Always check Vin even though
				// the PSU is off

				
				PSU_fsm = PSU_STATE_PWROFF;
			}
		}
		break;
		
	}
	
	app_psu_powerCMD();
}

uint8_t app_psu_status(){
	return PSU_fsm;
}