#include "Tick.h"
#include "Thread.h"
#define TICK_THREAD_ID 0xe6

volatile uint32_t g_tick_tick = 0;
timerHandle g_tick_htim = NULL;
uint8_t g_tick_start = 0;

void tick_start(void)
{
	g_tick_start = 1;
}

void tick_stop(void)
{
	g_tick_start = 0;
}

void tick_init(timerHandle htim)
{
	g_tick_htim = htim;
	HAL_TIM_Base_Start_IT(htim);
	thread_init(TICK_THREAD_ID,g_tick_tick);
}

void tick_add(threadFunction_t function, uint32_t interval)
{
	thread_quickAdd(TICK_THREAD_ID,function,interval,0,1);
}

void HAL_TIM_PeriodElapsedCallback(timerHandle htim)
{	
	if(htim == g_tick_htim && g_tick_start)
	{
		g_tick_tick ++;
		thread_run(TICK_THREAD_ID,g_tick_tick,1,1);
	}
}
