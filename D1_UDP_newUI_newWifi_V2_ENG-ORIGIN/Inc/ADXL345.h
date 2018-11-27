#ifndef ADXL345_H__
#define ADXL345_H__
#include "stm32f0xx_hal.h"
#include "stdint.h"

void adxl345_init(I2C_HandleTypeDef *hi2c);
uint8_t adxl345_getXYZ(int16_t* x,int16_t* y,int16_t* z);
uint8_t	adxl345_getShaking(void);

#endif
