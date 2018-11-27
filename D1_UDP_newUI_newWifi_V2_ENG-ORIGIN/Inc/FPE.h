#ifndef FPE_H__
#define FPE_H__
#include "stdint.h"

void fpe_init(uint32_t firstPage, uint32_t sencondPage);
uint32_t fpe_read(uint32_t address);
void fpe_write(uint32_t address, uint32_t value);
void fpe_readString(uint32_t address,char* str,uint16_t maxSize);
void fpe_writeString(uint32_t address, char* str);

#endif
