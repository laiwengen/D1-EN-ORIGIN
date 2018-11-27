#ifndef TICK_H__
#define TICK_H__
#include "stm32f0xx_hal.h"
#include "Thread.h"

typedef TIM_HandleTypeDef * timerHandle;

static inline uint32_t tick_ms()
{
	return HAL_GetTick();
}

void tick_init(timerHandle htim);
void tick_add(threadFunction_t function, uint32_t interval);
void tick_start(void);
void tick_stop(void);

#endif
