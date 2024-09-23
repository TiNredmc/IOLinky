#include "iol_sm.h"

// Private Includes
#include "usart.h"
#include "iol_l6362.h"			// PHY

// Private variables
uint8_t iol_commode = 0;

// Private Typedef

// Transceiver to MCU PHY 
// Link UART driver from BSP to the LL PHY
l6362_uart_handler_t phy_uart_pHandler_t = {
	.uart_init 							= usart_init,
	
	.uart_getPEStatus 			= usart_getPEStatus,
	.uart_setReadPointer 		= usart_setReadPtr,
	.uart_getReadIndex			= usart_getReadIdx,
	.uart_resetReadIndex		= usart_resetReadIdx,
	
	.uart_enableTX					= usart_enableTX,
	.uart_disableTX					= usart_disableTX,
	.uart_getTCStatus				= usart_getTransferCompleted,
	.uart_write							= usart_write
};

// MCU PHY to IO-Link physical layer
// Link the LL PHY to IO-Link physical layer
iol_pl_handler_t phy_l6362_pHandler_t = {
	.phy_init								= iol_sm_phyInit,
	
	.phy_getParityError			= l6362_parityChk,
	.phy_readFIFO						= l6362_readFIFO,
	.phy_writeFIFO					= l6362_writeFIFO,
	.phy_setMsequenceLength	= l6362_setMseq
};

// Device's direct parameter data
device_directparam_t device_dp_t = {
	// Page 1
	{
		// Master related stuff, leave it zero as default
		.MasterCommand 							= 0,									// MasterCommand
		.MasterCycleTime 						= 0,									// MasterCycleTime
		
		// Cycle time with 0.4ms time base
		// 6.4ms + (0.4ms * Multiplier)
		// This cycle time is 20.0ms
		.MinCycleBit.MTMult 				= 34,	// multiplier is 34
		.MinCycleBit.MTTimeBase			= 1,	// 0.4ms time base
		
		// M-Sequence Capacility
		.MSeqPreOpCode							= PREOP_M_0,// TYPE0 in PREOPERATE mode
		.MSeqOpCode									= OP_M_2_V8,// OPERATE : TYPE2_V 8 PDin octets
		.MSisdu											= 1,// ISDU supported

		// IO-Link revision V1.1
		.MajorRev										= 1,
		.MinorRev										= 1,
		
		// Process Data In
		.PDIByte										= 1,// PD In more than 1 byte
		.PDISIO											= 0,// SIO mode unsupported
		.PDIlength									= 8 - 1,// PD in 8 bytes long
		
		// Process Data Out -> No PD Out
		.PDOByte										= 0,// PD Out is less than 2 bytes
		.PDOlength									= 0,// No PD Out
		
		// Vendor ID
		.VendorID										= 0xFFFF,// I wish I can have one
		
		// Device ID
		.DeviceID1									= 0x12,
		.DeviceID2									= 0x34,
		.DeviceID3									= 0x56,

		// Function ID -> Reserved for IO-Link later version
		.FunctionID									= 0x00,
		
		// System command -> default startup 0x00
		.SystemCommand							= 0x00
	},
	
	// Page 2 // Device Specific profile
	{
		1, 2, 3, 4,
		5, 6, 7, 8, 
		9, 10, 11, 12, 
		13, 14, 15, 16					
	}
	
};

// Mandatory String of ISDU
isdu_data_t isdu_d_t = {
	"KMITL",										// Vendor Name
	"Automation Eng. KMITL", 		// Vendor Text
	"IOLinky V1.0",							// Product name
	"10Linky",									// Product ID
	"Digital Buck converter",		// Product text
	"1234ABCD",									// Serial number
	"E230_6362",								// Hardware version
	"E230_V1"										// Firmware version
};


// Initialize your IO-Link PHY here
void iol_sm_phyInit(){
	switch(iol_commode){
		case COM3:
		{
			l6362_init(
				&phy_uart_pHandler_t,
				0x012A
			);
		}
		break;
		
		case COM2:
		{
			l6362_init(
				&phy_uart_pHandler_t,
				0x0750
			);
		}
		break;
		
		case COM1:
		default:
		{
			l6362_init(
				&phy_uart_pHandler_t,
				0x3AA0
			);
		}
		break;
	
	}
}

void iol_sm_init(
	uint8_t commode,
	uint8_t *pd_data_ptr){
		
	// Get com mode	
	iol_commode = commode;
		
	iol_pl_init(&phy_l6362_pHandler_t);		
		
	iol_al_init(
		&device_dp_t,
		&isdu_d_t,
		pd_data_ptr
	);
		
}