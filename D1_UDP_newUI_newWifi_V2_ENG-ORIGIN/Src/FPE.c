#include "fpe.h"
#include "string.h"
#include "stm32f0xx_hal.h"

uint32_t g_fpe_page1, g_fpe_page2;

#define flashAt(addr) *(volatile uint32_t *)(addr)


static uint32_t getCurrenPage(void)
{
	if(flashAt(g_fpe_page1) != UINT32_MAX)
	{
		return g_fpe_page1;
	}
	else
	{
		return g_fpe_page2;
	}
}

static uint32_t getFirstFreeOffset(uint32_t base)
{
	for(uint16_t i = 8; i < FLASH_PAGE_SIZE; i+= 8)
	{
		if(flashAt(base + i) == UINT32_MAX)
		{
			return i;
		}
	}
	return FLASH_PAGE_SIZE;
}

static uint8_t readInPage(uint32_t base, uint32_t address, uint32_t* pValue)
{
	//* for each backward
	for (int16_t i = FLASH_PAGE_SIZE - 8; i>=8; i-=8)
	{
		if (flashAt(base + i) == address)
		{
			if (pValue)
			{
				*pValue = flashAt(base + i+4);
			}
			return 1;
		}
	}
	return 0;
}

static uint8_t writeInPage(uint32_t base, uint32_t address, uint32_t value)
{
	//* read if already the value
	uint32_t rValue;
	if ( readInPage(base,address,&rValue))
	{
		if ( rValue == value)
		{
			return 1;
		}
	}
	else
	{
		//* or is erase the value
		if (value == UINT32_MAX)
		{
			return 1;
		}
	}
	//* check enough space
	uint32_t offset = getFirstFreeOffset(base);
	if (offset>= FLASH_PAGE_SIZE)
	{
		return 0;
	}
	//* program flash
	HAL_FLASH_Program(TYPEPROGRAM_WORD,base + offset, address);
	HAL_FLASH_Program(TYPEPROGRAM_WORD,base + offset+4, value);
	return 1;
}

static void reorder(uint32_t from, uint32_t to)
{
	//* erase the "to" page
	FLASH_EraseInitTypeDef eraseInit;
	eraseInit.TypeErase = TYPEERASE_PAGES;
	eraseInit.PageAddress = to;
	eraseInit.NbPages = 1;
	uint32_t error;
	HAL_FLASHEx_Erase(&eraseInit,&error);
	//* copy the data to the "to" page, backward.
	uint16_t toIndex = 8;
	for (int16_t i = FLASH_PAGE_SIZE - 8; i>=8; i-=8)
	{
		uint32_t addr = flashAt(from+i);
		uint32_t value = flashAt(from+i+4);
		uint32_t fvalue = 0;
		readInPage(from,addr,&fvalue);
		if (addr != UINT32_MAX && value!=UINT32_MAX && !readInPage(to,addr,NULL) && fvalue!=UINT32_MAX)
		{
			HAL_FLASH_Program(TYPEPROGRAM_WORD,to + toIndex, addr);
			HAL_FLASH_Program(TYPEPROGRAM_WORD,to + toIndex+4, value);
			toIndex += 8;
		}
	}
	// * set the flage of the "to" page
	HAL_FLASH_Program(TYPEPROGRAM_WORD,to, 0);
	//* erase the "from" page, so that the "from" flag would be clear.
	eraseInit.PageAddress = from;
	HAL_FLASHEx_Erase(&eraseInit,&error);
}


void fpe_init(uint32_t firstPage, uint32_t sencondPage)
{
	g_fpe_page1 = FLASH_BASE + FLASH_PAGE_SIZE*firstPage;
	g_fpe_page2 = FLASH_BASE + FLASH_PAGE_SIZE*sencondPage;
}
uint32_t fpe_read(uint32_t address)
{
	uint32_t currentPage = getCurrenPage();
	uint32_t value = UINT32_MAX;
	readInPage(currentPage,address,&value);
	return value;
}
void fpe_write(uint32_t address, uint32_t value)
{
	HAL_FLASH_Unlock();
	//* try to write to current page
	uint32_t currentPage = getCurrenPage();
	if ( !writeInPage(currentPage,address,value))
	{
		uint32_t nextPage;
		if ( currentPage == g_fpe_page1)
		{
			nextPage = g_fpe_page2;
		}
		else
		{
			nextPage = g_fpe_page1;
		}
		reorder(currentPage,nextPage);
		//* write again
		writeInPage(nextPage,address,value);
	}
	//* current is full, need reorder
	HAL_FLASH_Lock();
}

void fpe_readString(uint32_t address,char* str,uint16_t maxSize)
{
	uint32_t currentPage = getCurrenPage();
	uint32_t value = 0;
	for(uint16_t i = 0; i<maxSize; i++)
	{
		uint8_t shift = i&0x03;
		if (shift == 0)
		{
			value = 0;
			if (!readInPage(currentPage,address,&value))
			{
				*(str+i) = '\0';
				break;
			}
			address+=4;
		}
		uint8_t c = ((value)>>(shift<<3))&0xff;
		*(str+i)=c;
		if (c==0)
		{
			break;
		}
		if (i==maxSize-1)
		{
			*(str+i)='\0';
		}
	}
}

void fpe_writeString(uint32_t address, char* str)
{
	HAL_FLASH_Unlock();
	//* try to write to current page
	uint32_t currentPage = getCurrenPage();
	uint32_t value = 0;
	uint8_t reordered = 0;
	for(uint16_t i = 0; i<strlen(str)+1; i++)
	{
		uint8_t shift = i&0x03;
		value+=(*(str+i))<<(shift<<3);
		if (shift == 3 || i == strlen(str))
		{
			if (!writeInPage(currentPage,address,value))
			{
				if(reordered)
				{
					break;
				}
				uint32_t nextPage;
				if (currentPage == g_fpe_page1)
				{
					nextPage = g_fpe_page2;
				}
				else
				{
					nextPage = g_fpe_page1;
				}
				reorder(currentPage,nextPage);
				//* write again
				writeInPage(nextPage,address,value);
				reordered = 1;
			}
			value = 0;
			address += 4;
		}
	}
	//* current is full, need reorder
	HAL_FLASH_Lock();
}
