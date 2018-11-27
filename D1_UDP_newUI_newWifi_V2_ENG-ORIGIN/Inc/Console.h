#ifndef CONSOLE_H__
#define CONSOLE_H__
#include "stdint.h"
#include "stm32f0xx_hal.h"

typedef struct stringList_t
{
	char* string;
	struct stringList_t* next;
}stringList_t;

typedef UART_HandleTypeDef* uartHandle;

typedef struct consoleCommand_t
{
	struct stringList_t* first;
	struct consoleCommand_t* next;
} consoleCommand_t;

typedef void(*consoleFunction_t)(consoleCommand_t*);

typedef struct consoleListener_t
{
	uint8_t priority;
	const char* command;
	consoleFunction_t function;
	struct consoleListener_t * next;
} consoleListener_t;

typedef struct console_t
{
	uartHandle huart;
	uint8_t lock:1;
	uint8_t echo:1;
	consoleCommand_t* firstCommand;
	consoleListener_t* firstListener;
	volatile uint8_t* buffer;
	volatile uint16_t readIndex;
	volatile uint16_t echoIndex;
	volatile void* pWrite;
	uint16_t bufferSize;
	uint16_t bufferMask;
	struct console_t* next;
} console_t;

console_t* console_init(uartHandle huart, volatile uint8_t* buffer, uint16_t bufferSize);
void console_deinit(uartHandle huart);
void console_tick(void);
void console_run(void);
void console_deleteCommand(consoleCommand_t* c);
consoleCommand_t* console_getCommand(console_t* console, const char* command);
consoleCommand_t* console_getCommandOut(console_t* console, const char* command);

void console_addListener(console_t* console, const char* command, consoleFunction_t function, uint8_t priority);
void console_removeListener(console_t* console, const char* command);

void console_printf(console_t*c, char* fmt,...);
void console_printHex(console_t*c, uint32_t value, uint8_t bits);

#endif
