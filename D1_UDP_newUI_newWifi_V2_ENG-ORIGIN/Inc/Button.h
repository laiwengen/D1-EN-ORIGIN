#ifndef BUTTON_H__
#define BUTTON_H__
#include "stdint.h"

typedef enum{
	BUTTON_STATUS_NONE,
	BUTTON_STATUS_CLICKED,
	BUTTON_STATUS_DOUBLE_CLICKED,
	BUTTON_STATUS_CLICKED_NO_NEXT,
	BUTTON_STATUS_HOLD,
	BUTTON_STATUS_CLICKED_HOLD,
	BUTTON_STATUS_SIZE,
} button_status_t;

void button_init(uint16_t id, uint8_t(*isButtonPressed)(uint16_t));
uint32_t button_getStatus(uint16_t id, button_status_t status, uint8_t ignoreClear); 
void button_clearStatus(uint16_t id);
void button_tick(void);
void button_run(void);
void button_addListener(uint16_t id, button_status_t status, void(*function)(void), uint8_t priority);
void button_removeListener(uint16_t id, button_status_t status);

#endif
