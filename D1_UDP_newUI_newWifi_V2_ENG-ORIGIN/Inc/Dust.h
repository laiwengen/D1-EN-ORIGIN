#ifndef DUST_H__
#define DUST_H__
#include "stdint.h"
#include "stm32f0xx_hal.h"

void dust_init(ADC_HandleTypeDef* hadc, uint8_t* buffer1,uint8_t* buffer2, uint16_t size);
void dust_deinit(void);
void dust_newSize(uint16_t minWidth, uint16_t maxWidth);

void dust_run(void);
uint32_t dust_getData(uint16_t minWidth, uint16_t maxWidth, uint8_t type);


#endif
