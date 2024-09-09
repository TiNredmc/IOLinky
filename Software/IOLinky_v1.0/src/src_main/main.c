// IO-Linky V1.0 
#include "main.h"

typedef struct myFunctions{
	void (*testA)(void);
	void (*testB)(uint8_t input);
}myFunctions;

void testAFunct(){

}

void testBFunct(uint8_t input){

}

myFunctions testAPI = {
	testAFunct,
	testBFunct
};

void NMI_Handler(void){


}

void HardFault_Handler(void){
	while(1);
}


int main(){
	app_initIO();

	testAPI.testA();
	
	while(1){
		app_runner();
	}


}