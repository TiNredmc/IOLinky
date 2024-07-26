#include "spi.h"

void spi_init(){
	RCC->APBENR2 |= RCC_APBENR2_SPI1EN;// Enable SPI1 peripheral clock.
	
	SPI1->CR1 = 
		SPI_CR1_SSI |
		SPI_CR1_SSM	|						// SSM = 1
		(1 << SPI_CR1_BR_Pos) | // Divide 64Mhz to get 16Mhz SPI clock
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
				if((SPI1->SR & SPI_SR_RXNE) != 0){
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
				if((SPI1->SR & SPI_SR_RXNE) != 0){
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
				if((SPI1->SR & SPI_SR_RXNE) != 0){
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

	uint8_t spi_fsm = 0;		

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
				*(__IO uint8_t *)&SPI1->DR = TX & 0x80;
				spi_fsm = 1;
			}
			break;
			
			case 1:// Wait for RX data
			{
				if((SPI1->SR & SPI_SR_RXNE) != 0){
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
	
	uint8_t spi_fsm = 0;		

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
				if((SPI1->SR & SPI_SR_RXNE) != 0){
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
	
uint8_t spi_l6364FastReadFF(
	uint8_t *FR,
	uint8_t *statusReg,
  uint32_t timeout
	){
	uint8_t spi_fsm = 0;		
	uint8_t ff_len = 0;
	uint8_t ret = 0;
  uint32_t t_start = millis();

	GPIOA->ODR &= ~(1 << SPI1_SS);	

	do{
		
		switch(spi_fsm){
			case 0:// Send LINK2 address
			{
				*(__IO uint8_t *)&SPI1->DR = 0x0F;
				spi_fsm = 1;
			}	
			break;
			
			case 1:// Wait for RX data
			{
				if((SPI1->SR & SPI_SR_RXNE) != 0){
					spi_fsm = 2;
				}
			}
			break;
			
			case 2:// Read back status data
			{
				*statusReg = *(__IO uint8_t *)&SPI1->DR;
				
				spi_fsm = 3;
			}
			break;
			
			case 3:// Send dummy byte for reacdback
			{
				*(__IO uint8_t *)&SPI1->DR = 0xFF;
				spi_fsm = 4;
			}
			break;
			
			case 4:// Wait for RX data
			{
				if((SPI1->SR & SPI_SR_RXNE) != 0){
					spi_fsm = 5;
				}
			}
			break;
			
			case 5:// Get reading Length
			{
				ff_len = *(__IO uint8_t *)&SPI1->DR;
				ff_len >>= 2;
				
				ff_len &= 0x0F;
				
				ret = ff_len;
				
				if(ff_len == 0)
					goto exit_ja;
				
				
				spi_fsm = 6;
			}
			break;
			
			case 6:// Send dummy byte for readback
			{
				*(__IO uint8_t *)&SPI1->DR = 0xFF;
				spi_fsm = 7;
			}
			break;
			
			case 7:// Wait for RX data
			{
				if((SPI1->SR & SPI_SR_RXNE) != 0){
					spi_fsm = 8;
				}
			}
			break;
			
			case 8:// Read back
			{
				*FR++ = *(__IO uint8_t *)&SPI1->DR;
				
				ff_len--;
				if(ff_len == 0)
					goto exit_ja;
				
				spi_fsm = 6;
			}
			break;
		}

    if((millis() - t_start) >= timeout)
    {
      ret = 0;
      goto exit_ja;
    }

	}while(1);
	
exit_ja:	

	GPIOA->ODR |= (1 << SPI1_SS);
	return ret;
}

uint8_t spi_l6364FastWriteFF(
		uint8_t count,
		uint8_t *FR,
		uint8_t *statusReg,
    uint32_t timeout
	){
		  
  uint8_t ret = count;  
	uint8_t spi_fsm = 0;		
	uint8_t ff_len = 0;
  uint32_t t_start = millis();

  // Setting ret = count just in case the count > 0x0F results in count
	if(count > 0x0F)
			goto exit_ja;
	
	GPIOA->ODR &= ~(1 << SPI1_SS);	

	do{
		
		switch(spi_fsm){
			case 0:// Send LINK2 (write) address
			{
				*(__IO uint8_t *)&SPI1->DR = 0x0F | 0x80;
				spi_fsm = 1;
			}	
			break;
			
			case 1:// Wait for RX data
			{
				if((SPI1->SR & SPI_SR_RXNE) != 0){
					spi_fsm = 2;
				}
			}
			break;
			
			case 2:// Read back status data
			{
				*statusReg = *(__IO uint8_t *)&SPI1->DR;
				
				spi_fsm = 3;
			}
			break;
			
			case 3:// Send LINK data
			{
				// Send Data buffer fill count and set SND bit
				*(__IO uint8_t *)&SPI1->DR = (count << 2) | 1;
				
				spi_fsm = 4;
			}
			break;
			
			case 4:// Wait for RX data
			{
				if((SPI1->SR & SPI_SR_RXNE) != 0){
					spi_fsm = 5;
				}
			}
			break;
			
			case 5:// Dummy read
			{
				(void)*(__IO uint8_t *)&SPI1->DR;
				
				if(count == 0)
        {
          ret = 0;
					goto exit_ja;
        }
				
				spi_fsm = 6;
			}
			break;
			
			case 6:// Send IO-Link data to FR buffer
			{
				*(__IO uint8_t *)&SPI1->DR = *FR++;				
				count--;

				spi_fsm = 4;
			}
			break;
		}

    if((millis() - t_start) >= timeout)
    {
      // Returning remaining to transmit bytes
      ret = count;
      goto exit_ja;
    }
		
	}while(1);
	
exit_ja:	

	GPIOA->ODR |= (1 << SPI1_SS);		
	return ret;
}