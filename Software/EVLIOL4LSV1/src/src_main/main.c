// IO-Link device example for EVLIOL4LSV1
// The Quad Low-side IO-Link DO.
#include "main.h"


void NMI_Handler(void){

	// If HSE clock failure was detected
	clk_CSSHandler();

}


int main(){
	app_initIO();
	
	

	while(1){
		app_runner();
		
	}


}