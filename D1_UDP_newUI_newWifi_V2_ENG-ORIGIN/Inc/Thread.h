#ifndef THREAD_H__
#define THREAD_H__
#include "stdint.h"

typedef struct thread_t
{
	uint8_t priority;
	uint32_t remainTimes;
	uint32_t intervalTick;
	uint32_t minNeedTick;
	uint32_t executeTick;
	void (*function) (void);
	struct thread_t* next;
} thread_t;

typedef void (*threadFunction_t) (void);

void thread_init(uint16_t id, uint32_t tick);
void thread_deinit(uint16_t id);
uint8_t thread_has(uint16_t id, threadFunction_t function);
uint8_t thread_add(uint16_t id, thread_t* t, threadFunction_t follow);
void thread_quickAdd(uint16_t id, threadFunction_t function,uint32_t interval,uint32_t minNeed,uint8_t priority);
void thread_remove(uint16_t id, threadFunction_t function);
void thread_execute(uint16_t id, threadFunction_t function,uint32_t tick);
void thread_run(uint16_t id, uint32_t tick, uint8_t priority, uint8_t miniPriority);


#endif
