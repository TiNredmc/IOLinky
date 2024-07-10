#include "spi.h"

void spi_init(){
	RCC->APBENR2 |= RCC_APBENR2_SPI1EN;// Enable SPI1 peripheral clock.
	
	SPI1->CR1 = 
		SPI_CR1_SSI |
		SPI_CR1_SSM	|						// SSM = 1
		(2 << SPI_CR1_BR_Pos) | // Divide 64Mhz to 8Mhz SPI clock
		SPI_CR1_MSTR;						// Master SPI mode
	
	SPI1->CR2 = 
		(7 << SPI_CR2_DS_Pos) | // Set SPI data size to 4-bit
		SPI_CR2_FRXTH;
	
	SPI1->CR1 |= SPI_CR1_SPE;
}

void spi_read(uint8_t *data, size_t len){
	uint8_t spi_fsm = 0;	
		
	GPIOA->ODR &= ~(1 << SPI1_SS);	
	
	do{
		
		switch(spi_fsm){
			case 0:// Send dummy data over SPI
			{
				*(__IO uint8_t *)&SPI1->DR = 0x00;
				
				spi_fsm = 1;
			}	
			break;

			case 1:// Wait for RX data
			{
				if((SPI1->SR & (3 << 9)) != 0){
					spi_fsm = 2;
				}
			}
			break;
			
			case 2:// Read back
			{
				if(data != NULL)
					*data++ = *(__IO uint8_t *)&SPI1->DR;
				else
					(void)*(__IO uint8_t *)&SPI1->DR;
				
				len--;
				spi_fsm = 0;
			}
			break;
			
			default:
				spi_fsm = 0;
				return;
		}
		
	}while(len);
		
	
	GPIOA->ODR |= (1 << SPI1_SS);
}

void spi_Write(uint8_t *data, size_t len){
	uint8_t spi_fsm = 0;	
		
	GPIOA->ODR &= ~(1 << SPI1_SS);	
	
	do{
		
		switch(spi_fsm){
			case 0:// Send data to over SPI
			{
				if(data != NULL)
					*(__IO uint8_t *)&SPI1->DR = *data++;
				else	
					*(__IO uint8_t *)&SPI1->DR = 0x00;
				
				spi_fsm = 1;
			}	
			break;

			case 1:// Wait for RX data
			{
				if((SPI1->SR & (3 << 9)) != 0){
					spi_fsm = 2;
				}
			}
			break;
			
			case 2:// Read back dummy data
			{
				(void)*(__IO uint8_t *)&SPI1->DR;
				
				len--;
				spi_fsm = 0;
			}
			break;
			
			default:
				spi_fsm = 0;
				return;
		}
		
	}while(len);
		
	
	GPIOA->ODR |= (1 << SPI1_SS);
}


void spi_WriteAndRead(
	uint8_t *TXbuf, 
	uint8_t *RXbuf,
	size_t len){
		
	uint8_t spi_fsm = 0;	
		
	GPIOA->ODR &= ~(1 << SPI1_SS);	
	
	do{
		
		switch(spi_fsm){
			case 0:// Send data over SPI
			{
				if(TXbuf != NULL)
					*(__IO uint8_t *)&SPI1->DR = *TXbuf++;
				else	
					*(__IO uint8_t *)&SPI1->DR = 0x00;
				
				spi_fsm = 1;
			}	
			break;

			case 1:// Wait for RX data
			{
				if((SPI1->SR & (3 << 9)) != 0){
					spi_fsm = 2;
				}
			}
			break;
			
			case 2:// Read back
			{
				if(RXbuf != NULL)
					*RXbuf++ = *(__IO uint8_t *)&SPI1->DR;
				else
					(void)*(__IO uint8_t *)&SPI1->DR;
				
				len--;
				spi_fsm = 0;
			}
			break;
			
			default:
				spi_fsm = 0;
				return;
		}
		
	}while(len);
		
	
	GPIOA->ODR |= (1 << SPI1_SS);	
}
	
void spi_WriteOnceAndRead(
	uint8_t TX,
	uint8_t *RXbuf,
	size_t len){

	volatile uint8_t spi_fsm;		
		
  GPIOA->ODR &= ~(1 << SPI1_SS);	
	len++;

	do{
		
		switch(spi_fsm){
			case 0:// Send data to SPI
			{
				*(__IO uint8_t *)&SPI1->DR = TX;
				spi_fsm = 1;
			}	
			break;

			case 3:// Send dummy data
			{
				*(__IO uint8_t *)&SPI1->DR = 0x00;
				spi_fsm = 1;
			}
			break;
			
			case 1:// Wait for RX data
			{
				if((SPI1->SR & (3 << 9)) != 0){
					spi_fsm = 2;
				}
			}
			break;
			
			case 2:// Read back
			{
				if(RXbuf != NULL)
					*RXbuf++ = *(__IO uint8_t *)&SPI1->DR;
				else
					(void)*(__IO uint8_t *)&SPI1->DR;
				
				len--;
				spi_fsm = 3;
			}
			break;
			
			default:
				spi_fsm = 0;
				return;
		}
		
	}while(len);

	GPIOA->ODR |= (1 << SPI1_SS);
	
}
	
void spi_WriteOnceAndWrite(
	uint8_t TX,
	uint8_t *TXbuf,
	size_t len){
	
	volatile uint8_t spi_fsm;		

	GPIOA->ODR &= ~(1 << SPI1_SS);	
	len++;

	do{
		
		switch(spi_fsm){
			case 0:// Send address data over SPI
			{
				*(__IO uint8_t *)&SPI1->DR = TX;
				spi_fsm = 1;
			}	
			break;
			
			case 3:// Send the Actual data over SPI
			{
				*(__IO uint8_t *)&SPI1->DR = *TXbuf++;
				spi_fsm = 1;
			}
			break;

			case 1:// Wait for RX data
			{
				if((SPI1->SR & (3 << 9)) != 0){
					spi_fsm = 2;
				}
			}
			break;
			
			case 2:// Read back
			{
				(void)*(__IO uint8_t *)&SPI1->DR;
				
				len--;
				spi_fsm = 3;
			}
			break;
			
			default:
				spi_fsm = 0;
				return;
		}
		
	}while(len);

	GPIOA->ODR |= (1 << SPI1_SS);
}