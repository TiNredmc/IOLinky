#include "usart.h"

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


void usart_initIOLink(
	uint8_t com_mode,				// IO-Link COM speed
	uint8_t read_size_max,	// RX buffer size
	uint8_t *read_ptr,				// RX buffer
	uint8_t *write_ptr				// TX buffer
	){
		
	RCU_APB1EN |= (1 << 17);// Enable USART1 Clock
		
	USART_CTL0(USART1) |=
		(1 << 15)			|	// x8 oversampling
		(1 << 12)			|	// 9th bit for the parity
		(1 << 10)			|	// Enable parity (Even parity)
		(1 << 8)			|	// Enable Parity error interrupt
		(1 << 5)			|	// Enable RX not empty interrupt
		(1 << 3)			| // Enable TX
		(1 << 2)			;	// Enable RX

	USART_CTL1(USART1) |=
		(1 << 15)			; // Swap TX and RX pin
		
	switch(com_mode){
		case COM1:// 4.8 kBaud
			USART_BAUD(USART1) |= 0x4E20;
			break;
		
		case COM2:// 38.4 kBaud
			USART_BAUD(USART1) |= 0x09C0;
			break;
				
		case COM3:// 230.4 kBaud
			USART_BAUD(USART1) |= 0x01A0;
			break;
		
		default:// Default at COM2
			USART_BAUD(USART1) |= 0x09C0;
			break;
	}
	
	rx_size_max = read_size_max;
	rx_ptr = read_ptr;
	tx_ptr = write_ptr;
	
	NVIC_SetPriority(USART1_IRQn, 3);
	NVIC_ClearPendingIRQ(USART1_IRQn);
	NVIC_EnableIRQ(USART1_IRQn);
	
	USART_CTL0(USART1) |= 1;// Enable USART
	
}
	
void usart_enableTX(){
	USART_CTL0(USART1) &= ~(1 << 2);//  Disable Receiver
	GPIO_OCTL(GPIOA) |= (1 << EN_pin);
}

void usart_disableTX(){
	GPIO_OCTL(GPIOA) &= ~(1 << EN_pin);
	USART_CTL0(USART1) |= (1 << 2);//  Enable Receiver
}

void usart_writeRequest(
	uint8_t count){
	
	// Passing null pointer is a no no
	if(tx_ptr == 0)
		return;
	
	// Count is 0, no write.
	if(count == 0)
		return;
	
	// Uart busy writing, request denied
	if(uart_tx_fsm != 0)
		return;
		
	tx_size = count;		
}

uint8_t usart_pollWrite(){
	
	switch(uart_tx_fsm){
		case 0:// Idle state
		{
			// We got write request
			if(tx_size > 0){
				tx_idx = 0;// reset index pointer
				usart_enableTX();
				uart_tx_fsm = 1;
			}
		}
		break;
		
		case 1:// Check if TX is empty and TX is enabled
		{
			if((USART_STAT(USART1) & (1 << 7)))
					uart_tx_fsm = 2;
		}
		break;
		
		case 2:// Write to TX
		{
			USART_TDATA(USART1) = 
					*(tx_ptr + tx_idx);
				
			tx_idx++;
			
			uart_tx_fsm = 3;
		}
		break;
		
		case 3:// Check transfer complete
		{
			if((USART_STAT(USART1) & (1 << 6))){
				tx_size--;
				if(tx_size == 0){
					usart_disableTX();
					uart_tx_fsm = 0;
				}else{
					uart_tx_fsm = 2;
				}
			}
		}
		break;
	}
	
	return tx_size;// Return remaning
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

void usart_dmaWriteStart(
	uint8_t *data_ptr, 
	uint8_t data_len
	){
		
	uint32_t *temp;
	
	temp = (uint32_t *)&data_ptr;
		

}