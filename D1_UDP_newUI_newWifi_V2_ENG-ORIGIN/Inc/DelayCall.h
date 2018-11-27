#ifndef DELAY_CALL_H__
#define DELAY_CALL_H__

#include "stdint.h"
#include "thread.h"

void delayCall_call(threadFunction_t function, uint32_t delayTick, uint32_t times, uint8_t priority);
void delayCall_run(void);
void delayCall_tick(void);

#endif
