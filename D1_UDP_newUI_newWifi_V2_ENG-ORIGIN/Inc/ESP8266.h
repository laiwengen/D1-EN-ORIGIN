
#ifndef ESP8266_H_
#define ESP8266_H_

#include "stdint.h"
#include "CJSON.h"
#include "stm32f0xx_hal.h"

//esp8266_tick should be call frequently. it would read the RX buffer which collect the bytes WIFI transmited.
//and do the following case.
// * check if the connect has been closed, and reset to re-connect.
// * check if smartlink has been done, and reset to apply it.
// * check http response received. ONLY json data would be stored, and can be get by esp8266_getHTTPResponse function.
void esp8266_run(void);
char* esp8266_getMAC(void);
//initialization the wifi chip with a giving uart port.
void esp8266_init(UART_HandleTypeDef* huart);
void esp8266_hw_init(void);
void esp8266_deinit(void);

//send a http request, path can be "\\" or "\\somepath\\index.jsp".
void esp8266_sendHTTPRequest(char* path, cJSON* json);

//get the last response json. last but two will not be strored.
//REMEMBER free(json) when you finish.
cJSON* esp8266_getHTTPResponse(void);


void esp8266_smart(uint32_t ms);
void esp8266_smartStop(void);
//enter smart mode if can't join the default Ap for a while. 
//REMEMBER the wifi would be reset by esp8266_resetForCrash function during smart mode. this will quit smart mode.
void esp8266_smartNoAp(uint32_t ms);
uint8_t esp8266_smarting(void);

//dont sure if the wifi would crash for some unknow reason. so just reset it every few minutes.
uint8_t esp8266_resetForCrash(uint32_t ms);


void esp8266_test(void);
void esp8266_reset(void);
char* esp8266_getSSID(void);
uint8_t esp8266_tcp(char * str);
uint8_t esp8266_udp(char * str, char* address, uint16_t port,uint8_t muxId);
uint8_t esp8266_disconncet(uint8_t force);
uint8_t esp8266_getStatus(void);
char* esp8266_getReceiveString(void);
void esp8266_setSmartCallback(void(*fcn)(void));
int32_t esp8266_getSmartTick(void);
#endif
