
#include "string.h"
#include "stdio.h"
#include "stdarg.h"
#include "stdlib.h"
#include "esp8266.h"
#include "stm32f0xx_hal.h"
#include "mainConfig.h"
extern uint8_t g_main_status;

//hardware start here...
UART_HandleTypeDef* volatile g_esp8266_huart = NULL;
volatile uint32_t* volatile g_esp8266_dmaRemain = NULL;

//overwrite fputc function, or you will unable to use printf(...) function.
int fputc(int ch,FILE *f)
{
	if (g_esp8266_huart != NULL)
	{
		uint8_t c=ch;
		//PLATFORM uart transmit a single char.
		HAL_UART_Transmit(g_esp8266_huart,&c,1,1000);
	}
	return ch;
}

// this function return a (uint32_t) tick which should increase by 1 every 1ms.
static inline uint32_t getTick(void)
{
	//PLATFORM interrupt and volatile must be used.
	return HAL_GetTick();
}

// delay 1 ms
static inline void hw_delay(uint32_t ms)
{
	//PLATFORM
	HAL_Delay(ms);
}

// size depends on the RAM size. must be 1<<n. if you need more buffer for http response. declare a bigger one.
#define ESP8266_BUFFER_SIZE (1<<9)
#define ESP8266_BUFFER_MASK (ESP8266_BUFFER_SIZE-1)
volatile uint8_t g_esp8266_buffer[ESP8266_BUFFER_SIZE] = {0};
static void softwareInit(void);
//PLATFORM must be volatile, as it can be modified by DMA and IT.
volatile uint16_t g_esp8266_bufferIndex = 0;


//pull down reset pin of the WIFI chip, then pull up.
static void hw_reset(void)
{
	//HARDWARE set the gpio below as your REST pin of WIFI chip.(would be gpio16 instead, see the sch of WIFI moudle)
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_RESET);
	hw_delay(10);
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_SET);
}

void esp8266_hw_init(void)
{
	//PLATFORM 
	//Setp 0: power on
	hw_reset();
	//HARDWARE set the gpio below as your power MOSFET control pole if you have one. otherwise ignore.
  HAL_GPIO_WritePin(GPIOA,GPIO_PIN_7,GPIO_PIN_SET);
	
	//////*over write  IGNORED *//////
	//Step 1: init the buffer to UINT16_MAX, so that we would know the buffer has been wright.
	memset((void*)g_esp8266_buffer,UINT8_MAX,sizeof(g_esp8266_buffer));
//	g_esp8266_dmaRemain = &(g_esp8266_huart->hdmarx->Instance->CNDTR);
//	g_esp8266_huart->hdmarx->Instance->CNDTR = ESP8266_BUFFER_SIZE;
		g_esp8266_dmaRemain = &(g_esp8266_huart->hdmarx->Instance->CNDTR);
	//Step 2: Start DMA. In circle mode, dma would NEVER STOPPED.
	//start an automatic thread to collect the byte comming from RX pin.

	//////*over write  IGNORED *//////
	//the element of the buffer is 16bit, and has been set by 0xffff. 
	//if new byte comes, the element should be 0x00??, otherwise, it should be 0xff??. easy to find where to read.
	//as soon as the byte has be get by other function, please remember to set it by 0xffff. (this has already been done by the function below.)
	HAL_UART_Receive_DMA(g_esp8266_huart, (uint8_t *)g_esp8266_buffer, ESP8266_BUFFER_SIZE);
}

static void hw_deinit(void)
{
  HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_7,GPIO_PIN_RESET);
}


//PLATFORM   init with a uart port to communicate with WIFI chip. 
//the uart must has a dma handle, which from Rx to memory, P-increase disable, M-increase enable, circle mode, byte to byte.
//the dma interrupt must disable(which can be modified in uart.c), or the dma would be stop after the first circle.
void esp8266_init(UART_HandleTypeDef* huart)
{
	g_esp8266_huart = huart;
	__HAL_UART_CLEAR_IT(huart, UART_CLEAR_OREF);
	esp8266_hw_init();

	softwareInit();
}

void esp8266_deinit(void)
{
		if(esp8266_smarting())
	{
		esp8266_smartStop();
	}
	hw_deinit();
}

static inline uint16_t getWriteIndex(void)
{
	return ESP8266_BUFFER_SIZE - *(g_esp8266_dmaRemain);
}

/******************************************************************************/
/***********************************WARNING************************************/
/******************************************************************************/
/********software start... beginner would NOT change any code below.***********/
/********if you need call some function, please read ESP8266.h before**********/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

__IO uint8_t g_esp8266_status = 5;
__IO uint32_t g_esp8266_smartTick  = 0;
uint32_t g_esp8266_pairTick = 0;
uint8_t g_esp8266_pairTodo = 0;
uint8_t g_esp8266_isAtMode = 0;
uint32_t g_esp8266_resetTick = 0;
__IO uint8_t g_esp8266_smartStarted = 0;
char* g_esp8266_receiveString = NULL;
char* g_esp8266_ssidString = NULL;
void(*g_esp8266_smartFirstReceive)(void) = NULL; 

void esp8266_setSmartCallback(void(*fcn)(void))
{
	g_esp8266_smartFirstReceive = fcn;
}

cJSON * g_esp8266_responseJson = NULL;

uint8_t jsonResponse(char* str)
{
	if(*str == '{')
	{
		cJSON* json = cJSON_Parse(str);
		if (json)
		{
			if (g_esp8266_responseJson)
			{
				cJSON_Delete(g_esp8266_responseJson);
			}
			g_esp8266_responseJson = json;
			return 1;
		}
	}
	return 0;
}

uint8_t cipClosed(char* str)
{
	if(strcasecmp(str,"CLOSED")==0)
	{
		//reset();
		g_esp8266_status = 2;
		return 1;
	}
	
	if(strcasecmp(str,"link is not valid")==0)
	{
		//reset();
		g_esp8266_status = 2;
		return 1;
	}
	
	return 0;
}

uint8_t smartInfo(char* str)
{
	if (g_esp8266_smartStarted)
	{
		if(strstr(str,"smartconfig connected wifi"))
		{
			esp8266_smartStop();
			return 1;
		}
		if(strstr(str,"smartconfig type:"))
		{
			if (g_esp8266_smartFirstReceive)
			{
				g_esp8266_smartFirstReceive();
			}
			return 1;
		}
		if(strstr(str,"ssid:"))
		{
			char* ssid = str+5;
			if (strlen(ssid)>0)
			{
				if (g_esp8266_ssidString)
				{
					free(g_esp8266_ssidString);
					g_esp8266_ssidString = (char*) malloc(strlen(ssid)+1);
					strcpy(g_esp8266_ssidString,ssid);
				}
			}
		}
	}
	return 0;
}
uint8_t statusChanged(char* str)
{
	uint8_t status = 0;
	if(strcmp(str,"WIFI DISCONNECT") == 0)
	{
		status = 5;
	}
	if(strcmp(str,"WIFI CONNECTED") == 0)
	{
		//status = 4;
	}
	if(strcmp(str,"WIFI GOT IP") == 0)
	{
	//	status = 2;
	}
	if(strstr(str,"STATUS:") && strlen(str) == 8)
	{
		status = str[7] - '0';
	}
	
	if(strcasecmp(str,"CONNECT")==0 || strcasecmp(str,"0,CONNECT")==0)
	{
		status = 3;
	}
	if(strcasecmp(str,"CLOSED")==0)
	{
		status = 2;
	}
	
	if(strcasecmp(str,"link is not valid")==0)
	{
		status = 2;
	}	
	if(strcasecmp(str,"ALREADY CONNECTED")==0)
	{
		status = 3;
	}
	if(strstr(str,"+CIPSTATUS:0,\"UDP\","))
	{
//		status = 3;
	}
	if (status != 0)
	{
		g_esp8266_status = status;
		return 1;
	}
	return 0;
}

uint8_t receiveData(char* str)
{

	//TBD
	if(strstr(str,"+IPD,1,"))
	{
		char* douhao = strstr(str,"+IPD,1,") + 6;
		char* maohao = strchr(str,':');
		if (douhao && maohao)
		{
			*douhao = 0;
			*maohao = 0;
			uint16_t len = atoi(douhao+1);
			if (g_esp8266_receiveString)
			{
				free(g_esp8266_receiveString);
			}
			g_esp8266_receiveString = malloc(len+1);
			if (g_esp8266_receiveString)
			{
				strncpy(g_esp8266_receiveString,maohao+1,len);
			}
			//printf("RECV:%d,%s",len,maohao+1);
			return 1;
		}
	}
	else if(strstr(str,"+IPD,0,"))
	{
		char* douhao = strstr(str,"+IPD,0,") + 6;
		char* maohao = strchr(str,':');
		if (douhao && maohao)
		{
			*douhao = 0;
			*maohao = 0;
			uint16_t len = atoi(douhao+1);
			if (g_esp8266_receiveString)
			{
				free(g_esp8266_receiveString);
			}
			g_esp8266_receiveString = malloc(len+1);
			strncpy(g_esp8266_receiveString,maohao+1,len);
			//printf("RECV:%d,%s",len,maohao+1);
			return 1;
		}
	}
	return 0;
}
typedef uint8_t(*focus_t)(char*);
focus_t g_esp8266_focus[] = {smartInfo,statusChanged,receiveData};
static uint16_t checkNewLine(void)
{
	for (uint16_t i = 0; i < ESP8266_BUFFER_SIZE; i++) //#define SIZE_FOR_UART_RX 16
	{
		uint16_t index = (g_esp8266_bufferIndex+i)&ESP8266_BUFFER_MASK;
		if (index == getWriteIndex())
		{
			break;
		}
		uint16_t c = g_esp8266_buffer[index];
		if (c == 0x0a || c == 0x00)
		{
			return i+1;
		}
	}
	return 0;
}

static uint16_t readBuffer(char * data, uint16_t maxlen)
{
	uint16_t current_len = 0;
	for (uint16_t i = 0; i < maxlen; i++) //#define SIZE_FOR_UART_RX 16
	{
		if (g_esp8266_bufferIndex == getWriteIndex())
		{
			break;
		}
		uint16_t c = g_esp8266_buffer[g_esp8266_bufferIndex];
		g_esp8266_bufferIndex = (g_esp8266_bufferIndex + 1)&ESP8266_BUFFER_MASK;
		*(data+current_len) = c;
		current_len++;
	}
	return current_len;
}

static char* readNewLine(void)
{
	uint16_t lineLength = checkNewLine();
	if (lineLength == 0)
	{
		return NULL;
	}
	char* buffer = malloc(lineLength);
	if (buffer)
	{
		readBuffer(buffer,lineLength);
		*(buffer+lineLength-1) = '\0';
		if (lineLength>1)
		{
			char c = *(buffer+lineLength-2);
			if (c == 0x0d || c == 0x0a)
			{
				*(buffer+lineLength-2) = '\0';
			}
		}
	}
	return buffer;
}

static inline uint16_t lastIndex(void)
{
	return (getWriteIndex()-1)&ESP8266_BUFFER_MASK;
}
static char bufferLastChar(void)
{
	if (g_esp8266_bufferIndex == getWriteIndex())
	{
		return '\0';
	}
	else
	{
		return g_esp8266_buffer[lastIndex()];
	}
}



void readFocus(void)
{
	char* line = NULL;
	while(1)
	{
		line = readNewLine();
		if (line)
		{
			for (uint8_t i = 0; i< sizeof(g_esp8266_focus)/sizeof(focus_t); i++)
			{
				if (g_esp8266_focus[i](line))
				{
					break;
				}
			}
			free(line);
		}
		else
		{
			break;
		}
	}
}

uint32_t waitUntilBlankLine(uint32_t ms)
{
	uint32_t timeOutTick = getTick() + ms;
	uint32_t currentTick = getTick();
	while(timeOutTick - currentTick< (UINT32_MAX>>1))
	{
		char* line = readNewLine();
		if (line)
		{
			if (strlen(line) == 0)
			{
				free(line);
				return timeOutTick - currentTick;
			}
			for (uint8_t i = 0; i< sizeof(g_esp8266_focus)/sizeof(focus_t); i++)
			{
				if (g_esp8266_focus[i](line))
				{
					break;
				}
			}
			free(line);
		}
		currentTick = getTick();
	}
	return 0;
}


uint32_t waitUntilRxStopped(uint32_t ms, uint16_t interval)
{
	uint32_t timeOutTick = getTick() + ms;
	uint32_t lastRxTick = getTick();
	uint32_t currentTick = getTick();
	uint16_t i = lastIndex(),j;
	while(timeOutTick - currentTick< (UINT32_MAX>>1))
	{
		j = i;
		i = lastIndex();
		if(i!=j)
		{
			lastRxTick = getTick();
		}
		if (currentTick - (lastRxTick + interval) < (UINT32_MAX>>1))
		{
			return timeOutTick - currentTick;
		}
		currentTick = getTick();
	}
	return 0;
}

uint8_t waitUntilOk(uint32_t ms)
{
	uint32_t timeOutTick = getTick() + ms;
	uint32_t currentTick = getTick();
	while(timeOutTick - currentTick< (UINT32_MAX>>1))
	{
		char* line = readNewLine();
		if (line)
		{
			if (strcmp(line,"OK") == 0 || strcmp(line,"SEND OK") == 0)
			{
				free(line);
				return timeOutTick - currentTick;
			}
			if (strcmp(line,"ERROR") == 0 || strcmp(line,"busy s...") == 0)
			{
				free(line);
				return 0;
			}
			for (uint8_t i = 0; i< sizeof(g_esp8266_focus)/sizeof(focus_t); i++)
			{
				if (g_esp8266_focus[i](line))
				{
					break;
				}
			}
			free(line);
		}
		currentTick = getTick();
	}
	return 0;
}
uint8_t waitUntilStrcmp(uint32_t ms, char* str)
{
	uint32_t timeOutTick = getTick() + ms;
	uint32_t currentTick = getTick();
	while(timeOutTick - currentTick< (UINT32_MAX>>1))
	{
		char* line = readNewLine();
		if (line)
		{
			if (strcmp(line,str) == 0)
			{
				free(line);
				return timeOutTick - currentTick;
			}
			for (uint8_t i = 0; i< sizeof(g_esp8266_focus)/sizeof(focus_t); i++)
			{
				if (g_esp8266_focus[i](line))
				{
					break;
				}
			}
			free(line);
		}
		currentTick = getTick();
	}
	return 0;
}
	
uint8_t waitUntilStrstr(uint32_t ms, char* str)
{
	uint32_t timeOutTick = getTick() + ms;
	uint32_t currentTick = getTick();
	while(timeOutTick - currentTick< (UINT32_MAX>>1))
	{
		char* line = readNewLine();
		if (line)
		{
			if (strstr(line,str) != 0)
			{
				free(line);
				return timeOutTick - currentTick;
			}
			for (uint8_t i = 0; i< sizeof(g_esp8266_focus)/sizeof(focus_t); i++)
			{
				if (g_esp8266_focus[i](line))
				{
					break;
				}
			}
			free(line);
		}
		currentTick = getTick();
	}
	return 0;
}
static void vSetCommand(char * commond,va_list argptr)
{
	char* connect = "=";
	char * para;
	if (commond!=NULL)
	{
		printf("AT+%s",commond);
		while ((para = va_arg(argptr,char*))!=NULL)
		{
			printf("%s%s",connect,para);
			connect = ",";
		}
	}
	else
	{
		printf("AT");		
	}
	printf("\r\n");
}
static void setCommand(char* commond, ...)
{
	va_list argptr;
	va_start(argptr, commond);
	vSetCommand(commond,argptr);
	va_end(argptr);
}

static void setCommandWait(uint32_t ms, char * commond,...)
{
	waitUntilRxStopped(10,2);
	readFocus();
	va_list argptr;
	va_start(argptr, commond);
	vSetCommand(commond,argptr);
	va_end(argptr);
	waitUntilRxStopped(waitUntilBlankLine(ms),25);
}
char* getCommand(uint32_t ms, char* commond)
{
	printf("AT+%s\r\n",commond);
	uint32_t timeOutTick = getTick() + ms;
	uint32_t currentTick = getTick();
	while(timeOutTick - currentTick< (UINT32_MAX>>1))
	{
		char* line = readNewLine();
		if (line)
		{
			if(line[0] == '+')
			{
				uint16_t i;
				for(i = 1; i<strlen(line);i++)
				{
					if(line[i] == ':' || line[i] == '=')
					{
						break;
					}
				}
				uint16_t size = strlen(line) - i;
				if (size)
				{
					char* rst = malloc(size);
					if (rst)
					{
						strcpy(rst,line+i+1);
					}
					free(line);
					return rst;
				}
			}
			for (uint8_t i = 0; i< sizeof(g_esp8266_focus)/sizeof(focus_t); i++)
			{
				if (g_esp8266_focus[i](line))
				{
					break;
				}
			}
			free(line);
		}
		currentTick = getTick();
	}
	return NULL;
}

static void setATMode(void)
{
	hw_delay(500);
	readFocus();
	printf("+++");
	hw_delay(500);
	printf("\n");
	setCommandWait(5000,NULL);
	setCommandWait(5000,NULL);
	g_esp8266_isAtMode = 1;
}

static void reset(void)
{
	hw_reset();
	if (g_esp8266_isAtMode)
	{
	//	setATMode();
	}
	if (g_esp8266_smartStarted)
	{
		uint32_t smartRemain = g_esp8266_smartTick - getTick();
		if (smartRemain<(UINT32_MAX>>1) && smartRemain>0)
		{
			esp8266_smart(smartRemain);
		}
	}
	g_esp8266_resetTick = getTick();
}

uint8_t getStatus(uint8_t force)
{
	if (!force)
	{
		if (g_esp8266_status == 3)
		{
			return g_esp8266_status;
		}
	}
	setCommand("CIPSTATUS",NULL);
	uint32_t timeOutTick = getTick() + 1000;
	uint32_t currentTick = getTick();
	while(timeOutTick - currentTick< (UINT32_MAX>>1))
	{
		char* line = readNewLine();
		if (line)
		{
			if(strstr(line,"STATUS:") && strlen(line) == 8)
			{
				g_esp8266_status = line[7] - '0';
				free(line);
				return g_esp8266_status;
			}
			for (uint8_t i = 0; i< sizeof(g_esp8266_focus)/sizeof(focus_t); i++)
			{
				if (g_esp8266_focus[i](line))
				{
					break;
				}
			}
			free(line);
		}
		currentTick = getTick();
	}
	return 5;
	
}
void esp8266_run(void)
{
	if (g_main_status == STATUS_POWEROFF)
	{
		return;
	}
	readFocus();
	if(g_esp8266_smartStarted)
	{
		if ((getTick() - g_esp8266_smartTick)<(UINT32_MAX>>1))
		{
			esp8266_smartStop();
			reset();
		}
	}
	//esp8266_resetForCrash(60000);
}

void esp8266_sendHTTPRequest(char* path, cJSON* json)
{
	if (g_esp8266_isAtMode) 
	{
		return;
	}
	char * str = cJSON_PrintUnformatted(json);
	if (str)
	{
		uint16_t length = strlen(str);
		char* head = "POST %s HTTP/1.1\r\n"
		"Content-Length: %i\r\n"
		"Content-Type: text/plain\r\n"
		"Host: api.hanwanglantian.com\r\n"		
		"Connection: keep-alive\r\n"
		"User-Agent: konggan\r\n"
		"Accept: \r\n"		
		"\r\n";
		printf(head,path,length);
		printf(str);
//	printf("\r\n");
		free(str);
	}
}

cJSON* esp8266_getHTTPResponse(void)
{
	if (g_esp8266_responseJson)
	{
		cJSON* json = g_esp8266_responseJson;
		g_esp8266_responseJson = NULL;
		return json;
	}
	else
	{
		return NULL;
	}
}

static void softwareInit(void)
{
	//Step 3: reset
	
	reset();
	//Step 4: wait until the "ready" appeared.
	hw_delay(500);
	waitUntilRxStopped(2000,100);
	readFocus();
	//Step 5: check if in TM mode.
	printf("\n");
	hw_delay(1000);
	uint8_t test = lastIndex();
	char c = bufferLastChar();
	if (c != '\n' )
	{
		//Setp 5.1: if not in TM mode. Set the default properties.
		setATMode();
	}
	//AT+UART_CUR=115200,8,1,0,3
	//setCommandWait(5000,"UART_CUR","115200","8","1","0","0",NULL);  //sta mode.
	setCommandWait(1000,"CWMODE_DEF","1",NULL);  //sta mode.
	//*
//	char* ssid = "\"AIRJ_NB\"";
//	setCommandWait(20000,"CWJAP_DEF?",NULL);
//	setCommandWait(20000,"CWJAP_DEF",ssid,"\"airj2123\"",NULL); //default ssid.
	//*/
	setCommandWait(1000,"CWAUTOCONN","1",NULL);  //auto connect after reset.
//		setCommandWait(5000,"CIPSTART","\"TCP\"",g_esp8266_tcpServerIP,g_esp8266_tcpServerPort,NULL);  //set tcp target
	setCommandWait(1000,"CIPMODE","0",NULL);  //
	setCommandWait(1000,"SAVETRANSLINK","0",NULL);  //
	setCommandWait(1000,"CIPMUX","1",NULL);  //
	
	g_esp8266_isAtMode = 0;
//		reset();  // reset to enter tm mode.
//		waitUntilRxStopped(waitUntilBlankLine(5000),25);
//		readFocus();
}
uint8_t smart(int8_t type)
{
	if (type == 0)
	{
		setCommandWait(0,"CWSTARTSMART",NULL);
	}
	else
	{
		char buffer[4];
		sprintf(buffer,"%d",type);
		setCommandWait(0,"CWSTARTSMART",buffer,NULL);
	}
	if (waitUntilOk(5000))
	{
		return 1;
	}
	else
	{
		type--;
		if(type == 4 || type == -1)
		{
			return 0;
		}
		else
		{
			return smart(type);
		}
	}
}
void esp8266_smart(uint32_t ms)
{
	if(smart(3))
	{
		g_esp8266_smartStarted = 1;
		g_esp8266_smartTick = getTick()+ms;
	}
	readFocus();
}

void esp8266_smartStop(void)
{
	setCommandWait(5000,"CWSTOPSMART",NULL);
	if (waitUntilOk(5000))
	{
		g_esp8266_smartStarted = 0;
	}
}


uint8_t esp8266_smarting(void)
{
	return g_esp8266_smartStarted;
}
uint8_t esp8266_getStatus(void)
{
	if (g_esp8266_smartStarted)
	{
		return 1;
	}
	if (g_esp8266_status == 2)
	{
		return 4;
	}
	return g_esp8266_status;
}
/*
void esp8266_connectTCP(char* address, uint16_t port)
{
	char buffer[0x10];
	sprintf(buffer,"%d",port);
	char* addressForAT = (char*)malloc(strlen(address)+3);
	sprintf(addressForAT,"\"%s\"",address);
	setCommandWait(5000,"CIPSTART","\"TCP\"",address,buffer,NULL);
	free(addressForAT);
}

uint8_t esp8266_tcp(char * str)
{
//	char* status = getCommand(1000,"CIPSTATUS");
	if (!g_esp8266_smartStarted)
	{
		uint8_t status = getStatus(0);
		if(status == 2 || status == 4 || status == 3)
		{
			esp8266_connectTCP("192.168.2.89",82);
		}
		else if (status == 3)
		{
			char buffer[0x10];
			sprintf(buffer,"%d",strlen(str));
			setCommandWait(0,"CIPSENDEX",buffer,NULL);
			if(waitUntilOk(500))
			{
				printf(str);
				waitUntilOk(5000);
			}
		}
	}
	return esp8266_getStatus();
}*/


void esp8266_connectUDP(char* address, uint16_t port,uint8_t muxId)
{
	
	char buffer[0x10];
	char idBuffer[4];
	sprintf(idBuffer,"%d",muxId);
	sprintf(buffer,"%d",port);
	char* addressForAT = (char*)malloc(strlen(address)+3);
	
	sprintf(addressForAT,"\"%s\"",address);
	setCommandWait(5000,"CIPSTART",idBuffer,"\"UDP\"",addressForAT,buffer,NULL);
	free(addressForAT);
}
uint8_t esp8266_udp(char * str, char* address, uint16_t port,uint8_t muxId)
{
	static char* lastAddress = NULL;
	static uint16_t lastPort = 0;
	if (!g_esp8266_smartStarted)
	{
		uint8_t status;
		if (port!=lastPort || strncmp(address,lastAddress,strlen(address)))
		{
			esp8266_disconncet(0);
			status = g_esp8266_status;
		} 
		else
		{
			status = getStatus(0);
		}
		if(status == 2 || status == 4)
		{
			setCommandWait(1000,"CIPMUX","1",NULL); 
			esp8266_connectUDP(address,port,0);
			lastAddress = address;
			lastPort = port;
		}
		else if (status == 3)
		{
			char buffer[0x10];
			char idBuffer[4];
			sprintf(idBuffer,"%d",muxId);
			sprintf(buffer,"%d",strlen(str));
			setCommandWait(0,"CIPSENDEX",idBuffer,buffer,NULL);
			if(waitUntilOk(500))
			{
				printf(str);
				waitUntilOk(5000);
			}
		}
	}
	return esp8266_getStatus();
}

uint8_t esp8266_disconncet(uint8_t force)
{
	if (!g_esp8266_smartStarted)
	{
		uint8_t status = getStatus(0);
		if(force || status == 3)
		{
			setCommandWait(0,"CIPCLOSE","0",NULL);//be ware multi PORT close should add 0,1
			waitUntilOk(200);
			setCommandWait(0,"CIPCLOSE","1",NULL);//be ware multi PORT close should add 0,1
			if(waitUntilOk(200))
			{
				return 1;
			}
		}
	}
	return 0;
}

void esp8266_reset(void)
{
	reset();
}
char* esp8266_getSSID(void)
{
	if (g_esp8266_ssidString == NULL)
	{
		if(!g_esp8266_smartStarted)
		{
			char* get = getCommand(200,"CWJAP_DEF?");
			if(get)
			{
				char* start,*end;
				start = strstr(get,"\"");
				end = strstr(get,"\",\"");
				if(start && end && start+1<end)
				{
					char* ssid = malloc(end-start);
					*end = '\0';
					strcpy(ssid,start+1);
					g_esp8266_ssidString = ssid;
				}
				free(get);
			}
		}
	}
	if (g_esp8266_ssidString)
	{
		char* ssid = malloc(strlen(g_esp8266_ssidString)+1);
		strcpy(ssid,g_esp8266_ssidString);
		return ssid;
	}
	return NULL;
}
char* esp8266_getMAC(void)
{
//	if (g_esp8266_ssidString == NULL)
	{
		if(!g_esp8266_smartStarted)
		{
			char* get = getCommand(200,"CIPSTAMAC?");
			if(get)
			{
				char* start;
				//char* end;
				start = strstr(get,"\"");
			//	end = strstr(get,"\"");
				if(start && *(start + 18) == '\"')
				{
					char* ssid = malloc(13);
					memset(ssid,0,13);
					strncpy(ssid ,start+1,2);
					for (uint8_t i = 2; i<18;i++)
					{
						if (*(start + i) == ':')
						{
							strncpy(ssid + strlen(ssid),start+i+1,2);
						}
					}
					//*(ssid + 13) = '\0';
					free(get);
					return ssid;
				}
				free(get);
			}
		}
	}
	return NULL;
//	if (g_esp8266_ssidString)
//	{
//		char* ssid = malloc(strlen(g_esp8266_ssidString)+1);
//		strcpy(ssid,g_esp8266_ssidString);
//		return ssid;
//	}
//	return NULL;
}
char* esp8266_getReceiveString(void)
{
	char* str = g_esp8266_receiveString;
	g_esp8266_receiveString = NULL;
	return str;
}
void esp8266_test(void)
{
	setCommandWait(1000,"GMR",NULL);
	char* ssid = "\"AIRJ_NB\"";
	setCommandWait(20000,"CWJAP_DEF",ssid,"\"airj2123\"",NULL); //default ssid.
//	setCommandWait(0,"CIUPDATE",NULL);
//	if(waitUntilStrcmp(5000,"+CIPUPDATE:4"))
//	{
//		waitUntilStrstr(60000,"jump to run user");
//	}
	
//	setCommandWait(0,"CWSAP","\"AIRJESPAIRJ\"","\"airj2123\"","11","3",NULL);
}
int32_t esp8266_getSmartTick(void)
{
return (g_esp8266_smartTick - getTick());
}
