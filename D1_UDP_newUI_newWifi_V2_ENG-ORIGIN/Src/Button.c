#include "Button.h"
#include "Thread.h"
#include "stdlib.h"
//#include "stm32f0xx_hal.h"
#define TICK_PER_MS 1
#define CLICK_TICKS (10*TICK_PER_MS)
#define DOUBLE_CLICK_INTERVAL_TICKS (200*TICK_PER_MS)
#define HOLD_TICKS (1000*TICK_PER_MS)
#define LONG_TICKS (8000*TICK_PER_MS)
typedef struct button_t
{
	uint16_t id;
	uint8_t (*isButtonPressed)(uint16_t);
	uint8_t lastPressed;
	uint32_t lastRiseTick;
	uint32_t riseTick;
	uint32_t lastFallTick;
	uint32_t fallTick;
	uint32_t clearTick;
	struct button_t* next;
} button_t;

typedef struct listener_t
{
	uint16_t id;
	uint8_t priority;
	button_status_t status;
	void (*function)(void);
	struct listener_t * next;
} listener_t;

button_t* g_button_start = NULL;
listener_t* g_button_listener = NULL;
volatile uint32_t g_button_tick = 0;

static button_t* getButtonById(uint16_t id)
{
	button_t* b = g_button_start;
	while(b)
	{
		if (b->id == id)
		{
			return b;
		}
		b = b->next;
	}
	return NULL;
}

void button_init(uint16_t id, uint8_t(*isButtonPressed)(uint16_t))
{
	if (getButtonById(id))
	{
		return;
	}
	button_t* button = malloc(sizeof(button_t));
	button->id = id;
	button->clearTick = g_button_tick+1;
	button->lastRiseTick = g_button_tick;
	button->riseTick = g_button_tick;
	button->lastFallTick = g_button_tick;
	button->fallTick = g_button_tick;
	button->isButtonPressed = isButtonPressed;
	button->lastPressed = isButtonPressed(id);
	button->next = g_button_start;
	g_button_start = button;
}
void button_deinit(uint16_t id)
{
	button_t* b = g_button_start;
	button_t* pre = NULL;
	while(b)
	{
		if (b->id == id)
		{
			if (pre)
			{
				pre->next = b->next;
			}
			else
			{
				g_button_start = b->next;
			}
			free(b);
			return;
		}
		pre = b;
		b = b->next;
	}
}

uint32_t button_getStatus(uint16_t id, button_status_t status, uint8_t ignoreClear)
{
	button_t* b = getButtonById(id);
	if (!b)
	{
		return 0;
	}
	switch(status)
	{
		case (BUTTON_STATUS_CLICKED):
		{
			uint32_t downtime = b->riseTick-b->fallTick;
			if (downtime < (UINT32_MAX>>1))
			{
				if(downtime > CLICK_TICKS && downtime <= HOLD_TICKS)
				{
					if (ignoreClear || b->fallTick - b->clearTick<(UINT32_MAX>>1))
					{
						return g_button_tick - b->riseTick;
					}
				}
			}
		}
		break;
		case (BUTTON_STATUS_DOUBLE_CLICKED):
		{
			uint32_t downtime1 = b->lastRiseTick-b->lastFallTick;
			uint32_t uptime = b->fallTick-b->lastRiseTick;
			uint32_t downtime2 = b->riseTick-b->fallTick;
			if (downtime1 < (UINT32_MAX>>1) && uptime < (UINT32_MAX>>1) && downtime2 < (UINT32_MAX>>1))
			{
				if(downtime1 > CLICK_TICKS && downtime1 <= HOLD_TICKS\
					&& uptime <= DOUBLE_CLICK_INTERVAL_TICKS\
					&& downtime2 > CLICK_TICKS && downtime2 <= HOLD_TICKS)
				{
					if (ignoreClear || b->fallTick - b->clearTick<(UINT32_MAX>>1))
					{
						return g_button_tick - b->riseTick;
					}
				}
			}
		}
		break;
		case (BUTTON_STATUS_CLICKED_NO_NEXT):
		{
			uint32_t downtime = b->riseTick-b->fallTick;
			uint32_t uptime = g_button_tick-b->riseTick;
			if (downtime < (UINT32_MAX>>1) && uptime < (UINT32_MAX>>1))
			{
				if(downtime > CLICK_TICKS && downtime <= HOLD_TICKS\
					&& uptime > DOUBLE_CLICK_INTERVAL_TICKS)
				{
					if (ignoreClear || b->fallTick - b->clearTick<(UINT32_MAX>>1))
					{
						return g_button_tick - b->riseTick;
					}
				}
			}
		}
		break;
		case (BUTTON_STATUS_HOLD):
		{
			volatile uint32_t uptime = b->fallTick-b->riseTick;
			volatile uint32_t downtime = g_button_tick-b->fallTick;
			if (uptime>0 && uptime < (UINT32_MAX>>1) && downtime < (UINT32_MAX>>1))
			{
				if (downtime > HOLD_TICKS)
				{
					if (ignoreClear || b->fallTick - b->clearTick<(UINT32_MAX>>1))
					{
						return downtime - BUTTON_STATUS_HOLD;
					}
				}
			}
		}
		break;
		case (BUTTON_STATUS_CLICKED_HOLD):
		{
			uint32_t downtime1 = b->riseTick-b->lastFallTick;
			uint32_t uptime = b->fallTick-b->riseTick;
			uint32_t downtime2 = g_button_tick-b->fallTick;
			if (downtime1 < (UINT32_MAX>>1) && uptime>0 && uptime < (UINT32_MAX>>1) && downtime2 < (UINT32_MAX>>1))
			{
				if(downtime1 > CLICK_TICKS && downtime1 <= HOLD_TICKS\
					&& uptime <= DOUBLE_CLICK_INTERVAL_TICKS\
					&& downtime2 > HOLD_TICKS)
				{
					if (ignoreClear || b->fallTick - b->clearTick<(UINT32_MAX>>1))
					{
						return downtime2 - BUTTON_STATUS_HOLD;
					}
				}
			}
		}
		break;
	}
	return 0;
}

void button_clearStatus(uint16_t id)
{
	button_t* b = getButtonById(id);
	if (!b)
	{
		return;
	}
	b->clearTick = g_button_tick;
}

void button_tick(void)
{
	g_button_tick++;
	button_t* b = g_button_start;
	while(b)
	{
		uint8_t pressed = b->isButtonPressed(b->id);
		if (pressed && !b->lastPressed)
		{
			b->lastFallTick = b->fallTick;
			b->fallTick = g_button_tick;
		}
		if (!pressed && b->lastPressed)
		{
			b->lastRiseTick = b->riseTick;
			b->riseTick = g_button_tick;
		}
		b->lastPressed = pressed;
		b = b->next;
	}
	listener_t* l = g_button_listener;
	while(l)
	{
		if(l->priority == 1 && button_getStatus(l->id,l->status,0))
		{
			button_clearStatus(l->id);
			l->function();
		}
		l=l->next;
	}
}
void button_run(void)
{
	listener_t* l = g_button_listener;
	while(l)
	{
		if(l->priority == 0 && button_getStatus(l->id,l->status,0))
		{
			button_clearStatus(l->id);
			l->function();
		}
		l=l->next;
	}
}

static listener_t * getListener(uint16_t id, button_status_t status)
{
	listener_t* l = g_button_listener;
	while(l)
	{
		if(l->id == id && l->status == status)
		{
			return l;
		}
		l = l->next;
	}
	return NULL;
}

void button_addListener(uint16_t id, button_status_t status, void(*function)(void), uint8_t priority)
{
	if(getListener(id,status))
	{
		button_removeListener(id,status);
	}
	listener_t* l = malloc(sizeof(listener_t));
	l->function = function;
	l->id = id;
	l->priority = priority;
	l->status = status;
	l->next = g_button_listener;
	g_button_listener = l;
}

void button_removeListener(uint16_t id, button_status_t status)
{
	listener_t* pre = NULL;
	listener_t* l = g_button_listener;
	while(l)
	{
		if(l->id == id && l->status == status)
		{
			if (pre)
			{
				pre->next = l->next;
			}
			else
			{
				g_button_listener = l->next;
			}
			free(l);
		}
		pre = l;
		l = l->next;
	}
}
