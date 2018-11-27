#include "Thread.h"
#include "stdlib.h"

typedef struct threadGroup_t
{
	uint16_t id;
	uint32_t executeTick;
	thread_t* first;
	struct threadGroup_t* next;
} threadGroup_t;

threadGroup_t* g_thread_start = NULL;

threadGroup_t * getThreadGroup(uint16_t id)
{
	threadGroup_t* tg = g_thread_start;
	while (tg!=NULL)
	{
		if (tg->id == id)
		{
			return tg;
		}
		tg = tg->next;
	}
	return NULL;
}

void thread_init(uint16_t id, uint32_t tick)
{
	if (getThreadGroup(id))
	{
		return;
	}
	threadGroup_t* tg = (threadGroup_t*)malloc(sizeof(threadGroup_t));
	tg->id = id;
	tg->executeTick = tick;
	tg->first = NULL;
	tg->next = g_thread_start;
	g_thread_start = tg;
}
void thread_deinit(uint16_t id)
{
	{
		threadGroup_t* tg = getThreadGroup(id);
		if (tg == NULL)
		{
			return;
		}
		thread_t* t = tg->first;
		while(t)
		{
			thread_t* lt = t;
			t = t->next;
			free(lt);
		}
	}
	{
		threadGroup_t* tg = g_thread_start;
		threadGroup_t* pre = NULL;
		while(tg)
		{
			if (tg->id == id)
			{
				if (pre)
				{
					pre->next = tg->next;
				}
				else
				{
					g_thread_start = tg->next;
				}
				free(tg);
				return;
			}
			pre = tg;
			tg = tg->next;
		}
	}
}

uint8_t thread_has(uint16_t id, threadFunction_t function)
{
	threadGroup_t* tg = getThreadGroup(id);
	if (tg == NULL)
	{
		return 0;
	}
	thread_t* t = tg->first;
	while(t)
	{
		if (t->function == function)
		{
			return 1;
		}
		t = t->next;
	}
	return 0;
}

uint8_t thread_add(uint16_t id, thread_t* t, threadFunction_t follow)
{
	threadGroup_t* tg = getThreadGroup(id);
	if (tg == NULL)
	{
		return 0;
	}
	t->next = NULL;
	if(tg->first == NULL)
	{
		tg->first = t;
	}
	else
	{
		thread_t* pre = tg->first;
		while(pre ->next)
		{
			if (follow && pre->function == follow)
			{
				break;
			}
			pre = pre->next;
		}
		t->next = pre->next;
		pre->next = t;
	}
	return 1;
}
void thread_quickAdd(uint16_t id, threadFunction_t function,uint32_t interval,uint32_t minNeed,uint8_t priority)
{
	thread_t* t = (thread_t*) malloc(sizeof(thread_t));
	t->priority = priority;
	t->remainTimes = 0;
	t->executeTick = 0;
	t->function = function;
	t->intervalTick = interval;
	t->minNeedTick = minNeed;
	t->next = NULL;
	if (thread_add(id,t,NULL) == 0)
	{
		free(t);
	}
}
void thread_remove(uint16_t id, threadFunction_t function)
{
	threadGroup_t* tg = getThreadGroup(id);
	if (tg == NULL)
	{
		return;
	}
	thread_t* t = tg->first;
	thread_t* pre = NULL;
	while(t)
	{
		if (t->function == function)
		{
			if (pre)
			{
				pre->next = t->next;
			}
			else
			{
				tg->first = t->next;
			}
			free(t);
			return;
		}
		pre = t;
		t = t->next;
	}
}

static void execute(uint16_t id,thread_t* t,uint32_t tick)
{
	t->function();
	if (t->remainTimes == 1)
	{
		thread_remove(id,t->function);
	}
	else
	{
		if (t->remainTimes > 0)
		{
			t->remainTimes --;
		}
		t->executeTick = tick + t->intervalTick;
		threadGroup_t* tg = getThreadGroup(id);
		if (tg != NULL)
		{
			tg->executeTick = tick + t->minNeedTick;
		}
	}
}

void thread_execute(uint16_t id, threadFunction_t function,uint32_t tick)
{
	threadGroup_t* tg = getThreadGroup(id);
	if (tg == NULL)
	{
		return;
	}
	thread_t* t = tg->first;
	while(t)
	{
		if (t->function == function)
		{
			execute(id,t,tick);
		}
		t = t->next;
	}
}
void thread_run(uint16_t id, uint32_t tick, uint8_t priority, uint8_t miniPriority)
{
	threadGroup_t* tg = getThreadGroup(id);
	if (tg == NULL)
	{
		return;
	}
	thread_t* t = tg->first;
	while(t)
	{
		//check which thread should be call this time
		if((tick - t->executeTick) < (UINT32_MAX>>1) && (tick - tg->executeTick) < (UINT32_MAX>>1) && t->priority==priority)
		{
			execute(id,t,tick);
		}
		t = t->next;
	}
	if (priority >miniPriority)
	{
		priority --;
		thread_run(id,tick,priority,miniPriority);
	}
}

