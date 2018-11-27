#include "stm32f0xx.h"
#include "stdint.h"

#define IAP_VERSION ("0.0.1")
#define IAP_ENCODE_ENABLE 1
#define IAP_ENCODE_XOR 0x57
#define IAP_APP_START_OFFSET UINT32_C(0x00001000)

#define IAP_UART_SYNC_DELAY 40
#define IAP_UART_WAIT 200
#define IAP_UART_KEY 'Y'
#define IAP_UART_PASS 'N'
#define IAP_UART_BAND 921600
#define IAP_UART_AUTODETECT 0

#define IAP_GPIO GPIOA
#define IAP_UART USART1
#define IAP_DMA DMA1_Channel3


typedef struct
{
	uint8_t length;
	uint16_t address;
	uint8_t type;
	uint8_t data[0x10];
	uint8_t verify;
} intelHex_t;

#define g_iap_readBuffer ((volatile const uint8_t*)(SRAM_BASE+0x400))
#define g_iap_readIndex *((volatile uint8_t*)(SRAM_BASE+0x500))
#define g_iap_addressBase *((volatile uint32_t*)(SRAM_BASE+0x504))
#define g_iap_obRdp *((volatile uint32_t*)(SRAM_BASE+0x508))
#define g_iap_writeIndex (uint8_t)(0x100-(IAP_DMA->CNDTR))
#define g_iap_vector (volatile uint8_t*)SRAM_BASE


#if 1 // put stuff
static inline void putc(uint8_t c)
{
	while((IAP_UART->ISR&UART_FLAG_TXE) == 0);
	IAP_UART->TDR = c;

}

static void puts(uint8_t* str)
{
	while(*str)
	{
		putc(*str);
		str++;
	}
}

static void puthex(uint32_t address,uint8_t bits)
{
	for(uint8_t i = 0; i<bits; i+=4)
	{
		uint8_t c = (((uint32_t)address<<i>>(bits-4))&0x0f);
		if(c>9)
		{
			c+='A'-'0'-10;
		}
		putc('0'+c);
	}
}

static void puti(intelHex_t* ih)
{
	putc(':');
	puthex(ih->length,8);
	puthex(ih->address,16);
	puthex(ih->type,8);
	for(uint8_t i = 0; i< ih->length; i++)
	{
		puthex(*(ih->data +i),8);
	}
	puthex(ih->verify,8);
	puts("\r\n");
}
#endif
#if 1 // buffer operation

static inline uint8_t ascii2oct(uint8_t c)
{
	if(c >= 'a')
	{
		c = 0x0a + (c-'a');
	}
	else if(c>='A')
	{
		c = 0x0a + (c-'A');
	}
	else if(c>= '0')
	{
		c = c-'0';
	}
	else
	{
		c = 0;
	}
	return c&0x0f;
}

static inline uint8_t ascii2hex(uint8_t c0,uint8_t c1)
{
	return ascii2oct(c0) | (ascii2oct(c1)<<4);
}

static inline uint32_t msbBytes(uint8_t* buffer, uint8_t size)
{
	uint32_t value = 0;
	while(size--)
	{
		value<<=8;
		value|= *buffer;
		buffer++;
	}
	return value;
}


static inline uint32_t lsbBytes(uint8_t* buffer, uint8_t size)
{
	uint32_t value = 0;
	uint8_t shift = 0;
	while(size--)
	{
		value|= ((uint32_t)(*buffer))<<shift;
		shift+=8;
		buffer++;
	}
	return value;
}
static uint8_t checkAscii(uint8_t index)
{
	uint8_t c1 = *(g_iap_readBuffer + index);
	uint8_t c0 = *(g_iap_readBuffer + ((index + 1)&0xff));
	return ascii2hex(c0,c1);
}
static uint16_t findByteIndex(uint8_t start, uint8_t byteToFind)
{
	uint16_t byte;
	while(1)
	{
		byte = *((g_iap_readBuffer + start));
		if ((start) == g_iap_writeIndex)
		{
			return UINT16_MAX;
		}
		if (byte == byteToFind)
		{
			return start;
		}
		start ++;
	}
}

static uint8_t readLine(intelHex_t* ih)
{
	uint8_t error = 1;
	uint16_t startIndex = findByteIndex(g_iap_readIndex,':');
	if (startIndex != UINT16_MAX)
	{
		uint16_t endIndex = findByteIndex(startIndex,'\n');
		if (endIndex != UINT16_MAX)
		{
			error = 2;
			g_iap_readIndex = endIndex;
			if ((((uint8_t)((endIndex-1) - (startIndex+1))>>1)) == 1+2+1+1+checkAscii(startIndex+1))
			{
				startIndex += 1;
				endIndex = (endIndex - 1)&UINT8_MAX;
				uint8_t sum = 0;
				for(uint8_t i = startIndex; i!=endIndex && i!=endIndex+1; i+=2)
				{
					sum += (uint16_t)checkAscii(i);
				}
				if (sum == 0)
				{
					ih->length = checkAscii(startIndex+0);
					ih->address = (uint16_t)checkAscii(startIndex+4) | (((uint16_t)checkAscii(startIndex+2))<<8);
					ih->type = checkAscii(startIndex+6);
					for(uint8_t i=0; i<ih->length; i++)
					{
						ih->data[i] = checkAscii(startIndex+8+(i<<1));
					}
					ih->verify = checkAscii(startIndex+8+(ih->length<<1));
					error = 0;
				}
			}
		}
	}
	return error;
}


#endif

#if 1 // flash operation
static void flash_erease(uint16_t page)
{
	while(FLASH->SR & FLASH_SR_BSY);
	if (g_iap_addressBase + page >= OB_BASE)
	{
		FLASH->CR|=FLASH_CR_OPTER;
	}
	else
	{
		FLASH->CR|=FLASH_CR_PER;
	}
	FLASH->AR = g_iap_addressBase + page;
	FLASH->CR|= FLASH_CR_STRT;
	while(FLASH->SR & FLASH_SR_BSY);
	FLASH->CR&=~FLASH_CR_OPTER;
	FLASH->CR&=~FLASH_CR_PER;
}
static void flash_write(uint16_t offset, uint8_t* data, uint8_t size)
{
	offset &= ~1;
	while(FLASH->SR & FLASH_SR_BSY);
	if (g_iap_addressBase + offset >= OB_BASE)
	{
		FLASH->CR|=FLASH_CR_OPTPG;
	}
	else
	{
		FLASH->CR|=FLASH_CR_PG;
	}
	for(uint8_t i = 0; i<size; i+=2)
	{
		if (((g_iap_addressBase)+offset+i)>=FLASH_BASE+IAP_APP_START_OFFSET)
		{
			uint16_t value = lsbBytes(data+i,2);
			uint32_t addr = (g_iap_addressBase)+offset+i;
			while(FLASH->SR & FLASH_SR_BSY);
			if ( addr == (uint32_t)&(OB->RDP))
			{
				if ((value&0xff) == 0xaa && g_iap_obRdp != 0xaa)
				{
					continue;
				}
			}
			*(__IO uint16_t*)(addr) = value;
		}
	}
	FLASH->CR&=~FLASH_CR_OPTPG;
	FLASH->CR&=~FLASH_CR_PG;
}
static void flash_read(uint16_t offset, uint8_t* data, uint8_t size)
{
	for(uint8_t i = 0; i<size; i++)
	{
		*(data + i) = *(__IO uint8_t*)((g_iap_addressBase)+offset+i);
	}
}



#endif

#if 1 //code stuff
static inline uint8_t decode(uint8_t byte, uint32_t address)
{
	#if IAP_ENCODE_ENABLE
	return (byte^IAP_ENCODE_XOR) + (uint8_t)address;
	#else
	return byte;
	#endif
}

static inline uint8_t encode(uint8_t byte, uint32_t address)
{
	#if IAP_ENCODE_ENABLE
	return (byte - (uint8_t)address)^IAP_ENCODE_XOR;
	#else
	return byte;
	#endif
}

static void encodeBuffer(uint32_t address, uint8_t* buffer, uint8_t size)
{
	for(uint8_t i =0; i<size; i++)
	{
		*(buffer+i) = encode(*(buffer+i),address++);
	}
}

static void decodeBuffer(uint32_t address, uint8_t* buffer, uint8_t size)
{
	for(uint8_t i =0; i<size; i++)
	{
		*(buffer+i) = decode(*(buffer+i),address++);
	}
}

#endif
static void intelHexVerify(intelHex_t* ih)
{
	uint8_t sum = 0;
	sum += ih->length;
	sum += ih->address>>8;
	sum += ih->address;
	sum += ih->type;
	for(uint8_t i=0;i<ih->length;i++)
	{
		sum += *(ih->data + i);
	}
	ih->verify = -sum;
}

static void parseLine(intelHex_t* ih)
{
	switch (ih->type)
	{
		case 0x00:
			//program flash
			decodeBuffer(ih->address,ih->data,ih->length);
			flash_write(ih->address,ih->data,ih->length);
			//puthex(ih->address,16);
			puts(".\r\n");
			break;
		case 0x01:
			//program end
			puts("Chip reset.\r\n");
			{
				uint16_t i = 8000;
				while(i--)
				{
					__NOP();
				}
			}
			NVIC_SystemReset();
			break;
		case 0xa1:
			//ob lanch
			puts("OB lanched.\r\n");
			{
				uint16_t i = 8000;
				while(i--)
				{
					__NOP();
				}
			}
			FLASH->CR |= FLASH_CR_OBL_LAUNCH;
			break;
		case 0x04:
			//set address base
			if (ih->length == 2)
			{
				g_iap_addressBase = msbBytes(ih->data,ih->length) << 16;
				puts("Address offset is: 0x");
				puthex(g_iap_addressBase,32);
				puts(".\r\n");
			}
			break;
		case 0x10:
			//erase flash
			if ( ih->length > 0 && ih->length<=2)
			{
				uint16_t length = msbBytes(ih->data,ih->length);
				for(uint32_t address = ih->address&(~(FLASH_PAGE_SIZE-1));address<ih->address+length;address+=FLASH_PAGE_SIZE)
				{
					flash_erease(address);
				}
				puts("IAP Erease pages, address is: 0x");
				puthex(ih->address,16);
				puts(" length: 0x");
				puthex(length,16);
				puts(".\r\n");
			}
			break;
		case 0x11:
			//read flash
		
			if ( ih->length > 0 && ih->length<=2)
			{
				ih->type = 0x00;
				uint16_t length = msbBytes(ih->data,ih->length);
				puts("IAP Read flash, address is: 0x");
				puthex(ih->address,16);
				puts(" length: 0x");
				puthex(length,16);
				puts(":\r\n");
				while(length)
				{
					uint16_t thisl = length;
					if (thisl>0x10)
					{
						thisl = 0x10;
					}
					flash_read(ih->address,ih->data,thisl);
					encodeBuffer(ih->address,ih->data,thisl);
					ih->length = thisl;
					intelHexVerify(ih);
					puti(ih);
					length -= thisl;
					ih->address += thisl;
				}
				puts("Read done.\r\n");
			}
			break;
		case 0x20:
			//get iap version
			puts("IAP Version:");
			puts(IAP_VERSION);
			puts(".\r\n");
			break;
		default:
			puts("Ignore record.\r\n");
			break;
	}
}

static void iap_enter(void)
{
	g_iap_readIndex = 0;
	g_iap_addressBase = FLASH_BASE;
	puts("IAP has initalized.\r\n");
	g_iap_obRdp = (OB->RDP)&0xff;
	intelHex_t ih;
	while(1)
	{
		if(readLine(&ih) == 0)
		{
			parseLine(&ih);
		}
	}
}

static uint8_t waitSync(uint32_t delayms, uint32_t waitms)
{
	while(delayms --)
	{
		uint16_t i = 6000;
		while(i--)
		{
			__NOP();
		}
	}
	putc('U');
	while(waitms --)
	{
		uint16_t i = 6000;
		while(i--)
		{
			__NOP();
		}
		if (IAP_UART->RDR == IAP_UART_KEY)
		{
			return 1;
		}
		if (IAP_UART->RDR == IAP_UART_PASS)
		{
			return 0;
		}
	}
	return 0;
}
static void pre_init(void)
{
	__set_PRIMASK(1); //disable all interrupt;
	
	  /* Reset the RCC clock configuration to the default reset state ------------*/
  /* Set HSION bit */
  RCC->CR |= (uint32_t)0x00000001;

#if defined (STM32F051x8) || defined (STM32F058x8)
  /* Reset SW[1:0], HPRE[3:0], PPRE[2:0], ADCPRE and MCOSEL[2:0] bits */
  RCC->CFGR &= (uint32_t)0xF8FFB80C;
#else
  /* Reset SW[1:0], HPRE[3:0], PPRE[2:0], ADCPRE, MCOSEL[2:0], MCOPRE[2:0] and PLLNODIV bits */
  RCC->CFGR &= (uint32_t)0x08FFB80C;
#endif /* STM32F051x8 or STM32F058x8 */
  
  /* Reset HSEON, CSSON and PLLON bits */
  RCC->CR &= (uint32_t)0xFEF6FFFF;

  /* Reset HSEBYP bit */
  RCC->CR &= (uint32_t)0xFFFBFFFF;

  /* Reset PLLSRC, PLLXTPRE and PLLMUL[3:0] bits */
  RCC->CFGR &= (uint32_t)0xFFC0FFFF;

  /* Reset PREDIV[3:0] bits */
  RCC->CFGR2 &= (uint32_t)0xFFFFFFF0;

#if defined (STM32F071xB) || defined (STM32F072xB) || defined (STM32F078xB)
  /* Reset USART2SW[1:0] USART1SW[1:0], I2C1SW, CECSW, USBSW and ADCSW bits */
  RCC->CFGR3 &= (uint32_t)0xFFFCFE2C;
#elif defined (STM32F091xC)
  /* Reset USART3SW[1:0], USART2SW[1:0], USART1SW[1:0], I2C1SW, CECSW bits */
  RCC->CFGR3 &= (uint32_t)0xFFF0FFAC;
#else
  /* Reset USART1SW[1:0], I2C1SW, CECSW, USBSW  and ADCSW bits */
  RCC->CFGR3 &= (uint32_t)0xFFFFFE2C;
#endif

  /* Reset HSI14 bit */
  RCC->CR2 &= (uint32_t)0xFFFFFFFE;

  /* Disable all interrupts */
  RCC->CIR = 0x00000000;

	
	RCC->CFGR = 0x00280000;
	RCC->CR |= 0x01000000;
	while((RCC->CR&RCC_FLAG_PLLRDY) == 0);
	RCC->CFGR |= 0x0000000a;
	
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGCOMPEN;
	RCC->AHBENR |= (RCC_AHBENR_GPIOAEN);
	RCC->AHBENR |= (RCC_AHBENR_GPIOBEN);
	RCC->APB2ENR |= (RCC_APB2ENR_USART1EN);
	
	IAP_GPIO->MODER |= 0x00280000;
	IAP_GPIO->AFR[1] |= 0x00000110;
	IAP_GPIO->PUPDR |= 0x00140000;
	IAP_UART->CR1 = 0x0000000C;
#if IAP_UART_AUTODETECT
	IAP_UART->CR2 = USART_CR2_ABRMODE_0|USART_CR2_ABREN;
#endif
	IAP_UART->BRR = (48000000/IAP_UART_BAND);
	IAP_UART->CR1 |= 0x00000001;
}


static void app_init(void)
{
	
	IAP_GPIO->MODER &=~ 0x00280000;
	IAP_GPIO->AFR[1] &=~ 0x00000110;
	
	IAP_UART->CR1 &=~ 0x00000001;
	IAP_UART->CR1 = 0;
#if IAP_UART_AUTODETECT
	IAP_UART->CR2 = 0;
#endif
	IAP_UART->BRR = 0;
	
	RCC->AHBENR &=~ (RCC_AHBENR_GPIOAEN);
	RCC->APB2ENR &=~ (RCC_APB2ENR_USART1EN);
	
	for(uint16_t i = 0; i<0x10; i+=2)
	{
		*(uint16_t *)(g_iap_vector+i) = *(uint16_t*)(FLASH_BASE + IAP_APP_START_OFFSET + i);
	}
	for(uint16_t i = 0x10; i<0xc0; i+=2)
	{
		*(uint16_t *)(g_iap_vector+i) = *(uint16_t*)(FLASH_BASE + IAP_APP_START_OFFSET +i);
	}
	__HAL_REMAPMEMORY_SRAM();
	__set_PRIMASK(0);
}

static void iap_init(void)
{
	RCC->AHBENR |= (RCC_AHBENR_DMA1EN);
	
	IAP_UART->CR1 &=~ 0x00000001;
	IAP_UART->CR3 = 0x00000040;
	IAP_UART->CR1 |= 0x00000001;
	
	IAP_DMA->CNDTR = 0x00000100;
	IAP_DMA->CPAR = (uint32_t)&(IAP_UART->RDR);
	IAP_DMA->CMAR = (uint32_t)g_iap_readBuffer;
	IAP_DMA->CCR = 0x000000af;
	
	FLASH->KEYR = FLASH_FKEY1;
	FLASH->KEYR = FLASH_FKEY2;
	FLASH->OPTKEYR = FLASH_OPTKEY1;
	FLASH->OPTKEYR = FLASH_OPTKEY2;
	
	
}

void iap_main(void)
{
	__set_MSP(SRAM_BASE+0x0800); //set stack

	
	pre_init();  //init ips
//	      temp = GPIOA->MODER;
//      temp &= ~(GPIO_MODER_MODER0 << (position * 2));
//      temp |= ((GPIO_Init->Mode & GPIO_MODE) << (position * 2));
//      GPIOx->MODER = temp;
	GPIOA->MODER |= 0x01400000;//sel cs
	GPIOA->BSRRH |= 0x1800;//sel cs

	if (waitSync(IAP_UART_SYNC_DELAY, IAP_UART_WAIT))
	{
		iap_init();
		iap_enter();
	}
	else
	{
		app_init();
		void((*app_start)()) = (void((*)()))(*((volatile uint32_t *)(FLASH_BASE+IAP_APP_START_OFFSET+4)));
		__set_MSP(*(volatile uint32_t*)(FLASH_BASE+IAP_APP_START_OFFSET));
		(*app_start)();
	}
	
}

