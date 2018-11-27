#include "DelayCall.h"
#include "stdlib.h"

#define DELAY_CALL_ID 0x5c
volatile uint32_t g_delay_tick = 0;
void delayCall_call(threadFunction_t function, uint32_t delayTick, uint32_t times, uint8_t priority)
{
	thread_init(DELAY_CALL_ID,g_delay_tick);
	thread_t* t = (thread_t*) malloc(sizeof(thread_t));
	t->priority = priority;
	t->remainTimes = times;
	t->executeTick = g_delay_tick + delayTick;
	t->function = function;
	t->intervalTick = delayTick;
	t->minNeedTick = 0;
	t->next = NULL;
	thread_remove(DELAY_CALL_ID,function);
	thread_add(DELAY_CALL_ID,t,NULL);
}

void delayCall_run(void)
{
	thread_run(DELAY_CALL_ID,g_delay_tick,0,0);
}

void delayCall_tick(void)
{
	g_delay_tick++;
	thread_run(DELAY_CALL_ID,g_delay_tick,1,1);
}
