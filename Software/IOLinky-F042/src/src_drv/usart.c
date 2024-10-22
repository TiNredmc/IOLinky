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

void USART2_IRQHandler(void){
	usart_status_shadow = USART2->ISR;

	// Check parity bit
	if(usart_status_shadow & USART_ISR_PE){
		USART2->ICR |= USART_ICR_PECF;// clear flag
		// TODO: Reject data and send some kind of error up to iol stack
		usart_pe_flag = 1;
	}
	
	// Get data to the buffer 
	if(usart_status_shadow & USART_ISR_RXNE){
		*(rx_ptr + rx_idx) = (uint8_t)USART2->RDR;
		
		// Index pointing
		if(rx_idx < rx_size_max)
			rx_idx++;
		else
			rx_idx = 0;
	
	}
	
	
	// Clear interrupt flag
	USART2->ICR = 0x00121B5F;
}

void DMA1_Channel4_5_IRQHandler(void){
	dma_status_shadow = DMA1->ISR;

	// Clear TC and TE flag
	DMA1->IFCR = 
		DMA_IFCR_CTCIF4	|
		DMA_IFCR_CTEIF4;

}

void usart_initIOLink(
	uint8_t com_mode,				// IO-Link COM speed
	uint8_t read_size_max,	// RX buffer size
	uint8_t *read_ptr,				// RX buffer
	uint8_t *write_ptr				// TX buffer
	){
		
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
	RCC->AHBENR |= RCC_AHBENR_DMA1EN;
		
	USART2->CR1 |=
		USART_CR1_OVER8		|	// x8 oversampling
		USART_CR1_M0			|	// 9th bit for the parity
		USART_CR1_PCE			|	// Enable parity (Even parity)
		USART_CR1_PEIE		|	// Enable Parity error interrupt
		USART_CR1_RXNEIE	|	// Enable RX not empty interrupt
		USART_CR1_TE			| // Enable TX
		USART_CR1_RE			;	// Enable RX
	
//	USART2->CR2	|=
//		USART_CR2_DATAINV ;	// Inverse data logic of TX and RX (for IO-Link)
	
	switch(com_mode){
		case COM1:// 4.8 kBaud
			USART2->BRR |= 0x4E20;
			break;
		
		case COM2:// 38.4 kBaud
			USART2->BRR |= 0x09C2;
			break;
				
		case COM3:// 230.4 kBaud
			USART2->BRR |= 0x01A0;
			break;
		
		default:// Default at COM2
			USART2->BRR |= 0x09C2;
			break;
	}
	
	rx_size_max = read_size_max;
	rx_ptr = read_ptr;
	tx_ptr = write_ptr;
	
	NVIC_SetPriority(USART2_IRQn, 3);
	NVIC_ClearPendingIRQ(USART2_IRQn);
	NVIC_EnableIRQ(USART2_IRQn);
	
	USART2->CR1 |= USART_CR1_UE;// Enable USART
	
}
	
void usart_enableTX(){
	USART2->CR1 &= ~USART_CR1_RE;//  Disable Receiver
	GPIOA->ODR |= (1 << EN_pin);
}

void usart_disableTX(){
	GPIOA->ODR &= ~(1 << EN_pin);
	USART2->CR1 |= USART_CR1_RE;//  Enable Receiver
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
			if((USART2->ISR & (USART_ISR_TXE)))
					uart_tx_fsm = 2;
		}
		break;
		
		case 2:// Write to TX
		{
			USART2->TDR = 
					*(tx_ptr + tx_idx);
				
			tx_idx++;
			
			uart_tx_fsm = 3;
		}
		break;
		
		case 3:
		{
			if((USART2->ISR & USART_ISR_TC)){
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
		
	// Setup peripheral and memory anddress	
	DMA1_Channel4->CPAR = USART2_BASE;
	DMA1_Channel4->CMAR = *(uint32_t *)temp;
	
	DMA1_Channel4->CNDTR = data_len;	
	
	DMA1_Channel4->CCR |=
		DMA_CCR_MINC	| // Memory increment
		DMA_CCR_DIR		|	// Direction Mem to Peripheral
		DMA_CCR_TEIE	|	// Enable Transfer error interrupt
		DMA_CCR_TCIE	;	// Enable Transfer complete interrupt
	
	// Enable Channel 4 (USART2_TX)
	DMA1_Channel4->CCR |=
			DMA_CCR_EN;
			
	NVIC_SetPriority(DMA1_Channel4_5_IRQn, 2);
	NVIC_ClearPendingIRQ(DMA1_Channel4_5_IRQn);
	NVIC_EnableIRQ(DMA1_Channel4_5_IRQn);	
			
	// Trigger USART2 DMA request
	USART2->CR3 |= USART_CR3_DMAT;
}