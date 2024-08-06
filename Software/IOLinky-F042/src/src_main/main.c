// IO-Link device example with L6362 on STM32F042
#include "main.h"


void NMI_Handler(void){


}

void HardFault_Handler(void){
	while(1);
}


int main(){
	app_initIO();

	while(1){
		app_runner();
	}


}