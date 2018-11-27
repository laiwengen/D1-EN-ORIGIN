#ifndef QRENCODE_H__
#define QRENCODE_H__
#include "stm32f0xx_hal.h"

uint8_t* QR_stringToBuffer(const char* inputString);
void maskPattern(void);
void setFormatInfoPattern(uint8_t nPatternNo);
#endif
