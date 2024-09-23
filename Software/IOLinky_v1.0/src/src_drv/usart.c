#include "usart.h"

#define UART_CLK		72000000UL

// Private pointers
uint8_t *tx_ptr;
uint8_t *rx_ptr;

// Private variables
uint8_t tx_size = 0;
uint8_t tx_idx = 0;
uint8_t rx_size_max = 0;
uint8_t rx_idx = 0;

uint8_t uart_tx_fsm = 0;

volatile uint32_t usart_status_shadow = 0;
volatile uint8_t usart_pe_flag = 0;

volatile uint32_t dma_status_shadow = 0;

void USART1_IRQHandler(void){
	usart_status_shadow = USART_STAT(USART1);

	// Check parity bit
	if(usart_status_shadow & (1 << 0)){// PE ?
		USART_INTC(USART1) |= (1 << 0);// clear flag
		// TODO: Reject data and send some kind of error up to iol stack
		usart_pe_flag = 1;
	}
	
	// Get data to the buffer 
	if(usart_status_shadow & (1 << 5)){// RXNE ?
		*(rx_ptr + rx_idx) = (uint8_t)USART_RDATA(USART1);
		
		// Index pointing
		if(rx_idx < rx_size_max)
			rx_idx++;
		else
			rx_idx = 0;
	
	}
	
	
	// Clear interrupt flag
	USART_INTC(USART1) = 0x0000004F;
}


void usart_init(
	uint32_t baud_rate,			// UART baud rate
	uint8_t read_size_max,	// RX buffer size
	uint8_t *read_ptr				// RX buffer
	){
		
	RCU_APB1EN |= (1 << 17);// Enable USART1 Clock
		
	USART_CTL0(USART1) |=
		(1 << 12)			|	// 9th bit for the parity
		(1 << 10)			|	// Enable parity (Even parity)
		(1 << 8)			|	// Enable Parity error interrupt
		(1 << 5)			|	// Enable RX not empty interrupt
		(1 << 3)			| // Enable TX
		(1 << 2)			;	// Enable RX

	USART_CTL1(USART1) |=
		(1 << 15)			; // Swap TX and RX pin
		
	USART_BAUD(USART1) = 	baud_rate;
	
	rx_size_max = read_size_max;
	rx_ptr = read_ptr;
	
	NVIC_SetPriority(USART1_IRQn, 3);
	NVIC_ClearPendingIRQ(USART1_IRQn);
	NVIC_EnableIRQ(USART1_IRQn);
	
	USART_CTL0(USART1) |= 1;// Enable USART
	
}
	
void usart_setReadPtr(uint8_t *read_ptr){
	if(read_ptr == 0)
		return;
	
	rx_ptr = read_ptr;
}

void usart_enableTX(){
	USART_CTL0(USART1) &= ~USART_CTL0_REN;//  Disable Receiver
	GPIO_OCTL(GPIOA) |= (1 << EN_pin);
}

void usart_disableTX(){
	GPIO_OCTL(GPIOA) &= ~(1 << EN_pin);
	USART_CTL0(USART1) |= USART_CTL0_REN;//  Enable Receiver
}

uint8_t usart_getTransferCompleted(){
	return (USART_STAT(USART1) & (1 << 6)) ? 1 : 0 ;
}

void usart_write(uint8_t wdata){
	USART_TDATA(USART1) = wdata;
}

void usart_writePtr(uint8_t *wdata_ptr){
	USART_TDATA(USART1) = *wdata_ptr;
}

uint8_t usart_getReadIdx(){
	return rx_idx;
}

void usart_resetReadIdx(){
	rx_idx = 0;
}

uint8_t usart_getPEStatus(){
	if(usart_pe_flag == 0)
		return 0;
	
	usart_pe_flag = 0;
	return 1;
}
