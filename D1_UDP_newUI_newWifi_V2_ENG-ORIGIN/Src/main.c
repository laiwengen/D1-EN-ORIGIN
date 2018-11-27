/**
  ******************************************************************************
  * File Name          : main.c
  * Date               : 10/09/2015 13:58:37
  * Description        : Main program body
  ******************************************************************************
  *
  * COPYRIGHT(c) 2015 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx_hal.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc;
DMA_HandleTypeDef hdma_adc;


I2C_HandleTypeDef hi2c2;

IWDG_HandleTypeDef hiwdg;

SPI_HandleTypeDef hspi2;
DMA_HandleTypeDef hdma_spi2_rx;

TIM_HandleTypeDef htim1;
RTC_HandleTypeDef hrtc;
RTC_TimeTypeDef sTime;
RTC_DateTypeDef sDate;
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart1_rx;
DMA_HandleTypeDef hdma_usart2_rx;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC_Init(void);
//static void MX_I2C1_Init(void);
static void MX_I2C2_Init(void);
//static void MX_IWDG_Init(void);
//static void MX_SPI2_Init(void);
static void MX_TIM1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_RTC_Init(void);
/* USER CODE BEGIN PFP */
#define Select_Flash()       HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_RESET)//GPIO_ResetBits(GPIOC, GPIO_Pin_8)
#define NotSelect_Flash()    HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_SET)//GPIO_SetBits(GPIOC, GPIO_Pin_8)
/* USER CODE END PFP */

/* USER CODE BEGIN 0 */
#include "math.h"
#include "stdio.h"
#include "stdlib.h"

#include "Tick.h"
#include "Thread.h"
#include "DelayCall.h"
#include "FPE.h"

#include "Button.h"
//#include "Console.h"
#include "LCD.h"
#include "CJSON.h"
#include "Tick.h"
//#include "Dust.h"
#include "ESP8266.h"
#include "ADXL345.h"
#include "MD5.h"
#include "mainConfig.h"

uint32_t g_main_lastWifiTick = RESET_WHILE_NO_DATEGET;
uint32_t g_main_pairTick = 0;
#if MAC2UID
char* g_main_udpAddressDefault = "sensor.hw99lt.com";//"172.28.50.1";//
//char* g_main_udpAddressDefault = "192.168.10.104";//"172.28.50.1";//
#else
char* g_main_udpAddressDefault = "www.hwlantian.com";//"172.28.50.1";//
#endif
//char* g_main_udpAddressDefault = "www.qinest.com";//"172.28.50.1";//
char* g_main_udpAddress = NULL;
uint16_t g_main_udpPortDefault = 59830;
uint16_t g_main_udpPort = 0;
char* g_main_uidDefault = "D1";
char* g_main_uid = NULL;
char* g_main_uploadId = NULL;
char* g_main_uploadIdDefault = "D1";
__IO uint8_t g_main_ssidToDo = 1;
__IO uint8_t g_main_pairToDo = 1;
__IO uint8_t g_main_neverUsed;
uint32_t g_main_lastResponseTick = (UINT32_MAX - 5000);
//__IO uint8_t g_main_power = 1;
__IO float PM_modifier = 1;
extern uint16_t xOffset,yOffset;
extern int8_t g_main_10s_countDown;
extern int16_t g_main_120s_countDown;
uint8_t g_main_is_warmUp;
uint8_t lcd_refreshed = 0;
uint32_t g_battery_notify_tick = 0;
enum BUTTON_ID
{
	BUTTON_DOME = 1,
	BUTTON_TOUCH = 2,
	BUTTON_NUMBER,
};


//uint8_t g_main_consoleBuffer[0x100];
//uint8_t g_main_dustBuffer1[800];
//uint8_t g_main_dustBuffer2[800];

//console_t* g_main_console = NULL;

uint16_t g_main_rotation = 0;
int32_t g_main_temp = 0, g_main_humidity = 0;
int32_t g_main_ch2o = 0;//, g_main_ch2oL = 0; g_main_ch2oH = 0;
typedef struct {
	uint32_t PM1Value;	
	uint32_t PM2_5Value;
	uint32_t PM5Value;	
	uint32_t PM10Value;
	uint32_t PC0_3Count;
	uint32_t PC1_0Count;
	uint32_t PC2_5Count;
	uint32_t PC5_0Count;
	uint32_t PC10Count;
}pm_pc_value_t;
pm_pc_value_t pm_pc_value;
uint32_t g_main_co2 = 400;
__IO int16_t g_main_battery = 100;
__IO uint8_t g_main_status = STATUS_ALL_SHOW;
uint8_t g_main_gas_type = GAS_PM25;
int32_t g_last_turnoff_time = 0;
int32_t g_main_temp_fix = 0;
uint8_t g_main_t_air_mode = 0;
uint8_t MAX_TEMP_FIX = 4;
__IO uint8_t RUN_MODE = 'M';
__IO uint8_t B2_OUTPUT_FIX_MODE = 0;
__IO uint8_t g_main_is_charging = 0;

uint8_t g_main_isShaked = 0;
enum wifi_status
{
	WIFI_INIT = 0,
	WIFI_SMARTING,
	WIFI_REMOTE_ERROR,
	WIFI_CONNECTED,
	WIFI_RETRYING,
	WIFI_NO_AP,
	WIFI_SERVER_ACK
};
RTC_DateTypeDef g_main_date;
RTC_TimeTypeDef g_main_time;
uint32_t g_main_outDoor_pm25 = 0;

__IO uint8_t g_main_wifi_status = WIFI_NO_AP;

//uint32_t g_main_wifi_interval = 5000;
//uint32_t g_main_wifi_getDelay = 1500;
//uint32_t g_main_wifi_lastConncetTick = 0;

//#define WIFI_RESMARTTICK 30000
//#define WIFI_MAXSMARTTICK 90000
#define HISTORY_SIZE 30
#define UART2_BUFFER_SIZE 64
#define UART2_BUFFER_MASK (UART2_BUFFER_SIZE - 1)

#define getSetting(name) (name?name:name##Default)
uint8_t g_uart2Buffer[UART2_BUFFER_SIZE];
uint8_t g_B2_configBuffer[8] = {0};
typedef struct
{
	uint16_t historyBuffer[HISTORY_SIZE];
	uint8_t historyBufferIndex;
	uint8_t historyBuffersize;
	
}history_t;
history_t g_pm25_history;
history_t g_pm1_history; 
history_t g_pm10_history; 
//history_t g_co2_history;
//history_t g_ch2o_history;
history_t g_temprature_history; 
history_t g_humidity_history; 
_Bool g_isIniting = 0;
__IO uint32_t g_main_activeMs = 0;

//static inline  void pushhistory(history_t *buffer,uint16_t value)
//{
//	buffer->historyBuffer[buffer->historyBufferIndex] = value;
//	(buffer->historyBufferIndex)++;
//	if ((buffer->historyBufferIndex)>=HISTORY_SIZE)
//	{
//		(buffer->historyBufferIndex) = 0;
//	}
//	(buffer->historyBuffersize) = fmin((buffer->historyBuffersize)+1,HISTORY_SIZE);
//}

//static inline uint32_t readhistory(history_t *buffer,uint8_t index)
//{
//	if (index>=(buffer->historyBuffersize))
//	{
//		return 0;
//	}
//	if ((buffer->historyBufferIndex)<=index)
//	{
//		index = (buffer->historyBufferIndex)+(buffer->historyBuffersize)-index-1;
//	}
//	else
//	{
//		index = (buffer->historyBufferIndex)-index-1;
//	}
//	return (buffer->historyBuffer[index]);
//}




uint8_t isButtonPressed(uint16_t id);

void onPowerButtonPressed(void);
void onModeButtonPressed(void);
void onMode2ButtonPressed(void);
void onTouchButtonPressed(void);

void lcd_update(void);
void lcd_setOffset(uint16_t rotation);
void co2_tick(void);
void ch2o_update(void);
void dust_update(void);
void th_update(void);
void battery_update(void);
void dust_upadte(void);
void adxl345_update(void);
void B2_output_fix(void);
void wifi_update(void);
void wifi_deal(void);
void updateWifiStatus(void);
void UART_config_B2(char *stringToSend);
void B2_output_fix(void);
void history_record(void);
void mainThread(void);
void pm2d5_init(void);
void hts221_init(void);
//void setLevelColor(uint8_t dataType,uint32_t data,uint16_t *color,uint8_t *level);
void pairStart(void);
void EnterBootLoader(void);
void enterSleep(void);
void updateOutdoorAQI(void);
/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC_Init(void);
//static void MX_I2C1_Init(void);
static void MX_I2C2_Init(void);
//static void MX_IWDG_Init(void);
static void MX_TIM1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_RTC_Init(void);
static void sensorPowerOn(void)
{
	HAL_GPIO_WritePin(GPIOF,GPIO_PIN_7,GPIO_PIN_SET);//B2 power on
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_6,GPIO_PIN_SET); // co2 power on
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_9,GPIO_PIN_SET); // ch2o power on
//	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_4,GPIO_PIN_SET); // th power on
//	adxl345_init(&hi2c1);

}
static void sensorPowerOff(void)
{
	HAL_GPIO_WritePin(GPIOF,GPIO_PIN_7,GPIO_PIN_RESET);//B2 power off
//	HAL_GPIO_WritePin(GPIOF,GPIO_PIN_7,GPIO_PIN_RESET); // alx345 power off
//	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_6,GPIO_PIN_RESET); // co2 power off
//	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8,GPIO_PIN_RESET); // ch2o power off
//	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_4,GPIO_PIN_RESET); // th power off

}
static void UARTswitchChannel(uint8_t channel)
{
 if (channel)//esp8266
 {
 	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_12,GPIO_PIN_SET); // sel UART0 cs
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_11,GPIO_PIN_RESET); // sel UART0 cs

 }
 else
 {
  HAL_GPIO_WritePin(GPIOA,GPIO_PIN_12,GPIO_PIN_RESET); // sel UART0 cs
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_11,GPIO_PIN_RESET); // sel UART0 cs
 
 }
}
static void esp8266PowerOn(void)
{
//	HAL_UART_DMAPause(&huart1);
	UARTswitchChannel(1); //UART->esp8266
	B2_OUTPUT_FIX_MODE = 0;
	thread_remove(0,B2_output_fix);
//	HAL_UART_DMAStop(&huart1);	
	HAL_DMA_Abort(huart1.hdmarx);
	huart1.State= HAL_UART_STATE_READY;  
//	huart1.hdmarx->Instance->CNDTR ++;
	esp8266_hw_init();
//	HAL_UART_DMAResume(&huart1);
	
}
//static void esp8266PowerOff(void)
//{
//HAL_GPIO_WritePin(GPIOA,GPIO_PIN_11,GPIO_PIN_RESET);

//}
static void loadFlashData(void)
{
	uint32_t value;
	value = fpe_read(MAIN_STATUS_ADDRESS);
	if (value != UINT32_MAX)
	{
		g_main_status = STATUS_ALL_SHOW;
		
   if (g_main_is_charging)
	 {
	 g_main_status = STATUS_POWEROFF;
	 }
	}
//		value = fpe_read(LAST_TEMP_FIX_ADDR);
//	if (value != UINT32_MAX)
//	{
//		
//		g_main_temp_fix = value;
//	}
//		value = fpe_read(LAST_TURNOFF_TIME_ADDR);
//	if (value != UINT32_MAX)
//	{
//		g_last_turnoff_time = value;
//				HAL_RTC_GetTime(&hrtc, &sTime, FORMAT_BIN);
//		HAL_RTC_GetDate(&hrtc, &sDate, FORMAT_BIN);
//		__IO uint32_t current_time = sTime.Minutes  +  sTime.Hours * 60 +  ((sDate.Date-1)) * 24 * 60 + ((sDate.Month-1))*30*24*60;
//		__IO int32_t elapse_time = current_time - g_last_turnoff_time;
//		if  (elapse_time<0)
//		{
//			fpe_write(LAST_TURNOFF_TIME_ADDR,UINT32_MAX);
//		}
//		else
//		{
//			if (elapse_time >= COOL_DOWN_MINUTINES)
//			{
//				g_main_temp_fix = 0;
//				g_main_t_air_mode = 1;
//			}
//			else if (g_main_temp_fix>=0)
//			{
//				int16_t fix = g_main_temp_fix - (elapse_time>>2);
//				g_main_temp_fix  = fmax(fix,0);			
//			}
//		}
//	}

	__IO uint32_t readoutRatio = fpe_read(PM_RATIO_ADDRESS);	
	if (readoutRatio!= UINT32_MAX)
	{
		PM_modifier = readoutRatio/100.0  ;
	}
	else
	{
		PM_modifier = 1;		
	}	
	char strToSend[10] = {0}; 
	sprintf(strToSend,"%4d",(uint32_t)(PM_modifier*100));
	HAL_UART_Transmit(&huart1,(uint8_t *)strToSend,6,10);
	#if (!MAC2UID)
	
	char uid[0x40] = {0};
	fpe_readString(FPE_UID_ADDRESS,uid,sizeof(uid));
	if (strlen(uid)!=0)
	{
		g_main_uid = malloc(strlen(uid)+1);
		strcpy(g_main_uid,uid);
	}
	#endif
	g_main_neverUsed = fpe_read(FPE_NEVERUSED_ADDRESS);
}

static void hw_init(void)
{
	
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC_Init();
//  MX_I2C1_Init();
//  MX_I2C2_Init();
//  MX_IWDG_Init();
//  MX_SPI2_Init();
  MX_TIM1_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
	
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_SET);// turn off esp8266
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_6,GPIO_PIN_RESET);
	
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_13,GPIO_PIN_SET); // dc-dc on
	HAL_Delay(100);
	
	HAL_Delay(50);

	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_6,GPIO_PIN_RESET); // co2 power off
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_9,GPIO_PIN_RESET); // ch2o power off

	HAL_GPIO_WritePin(GPIOF,GPIO_PIN_7,GPIO_PIN_RESET);//B2 power off
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_4,GPIO_PIN_RESET); // battery charge enable
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_2,GPIO_PIN_RESET); // th power off
	
//	while (endTime>tick_ms())
//	{
//	if  (HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_4))
//	{
//	buttonCount++;
//	}
//	HAL_Delay(1);
//	}
//	if (buttonCount>450)
//	{
//	lcd_init();
//	}
//	else
//		{
//		g_main_force_sleep = 1;
//		}


	lcd_init();	
//	lcd_erase(240,320);
	//g_main_rotation = 0;
	
	
//	lcd_setOffset(g_main_rotation);
	lcd_setOrigin(xOffset,yOffset);
	g_main_is_charging = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_3); 
	if (g_main_is_charging)
	{
	//	sensorPowerOff();
		
		lcd_showlogo(rgb2c(255,255,255),1);
	}
	else
	{
		lcd_showlogo(rgb2c(0,64,255),0);
	}
#if	TH_ENABLE
	MX_RTC_Init();
	HAL_ADCEx_Calibration_Start(&hadc);
#endif	
	
//	dust_init(&hadc, g_main_dustBuffer1,g_main_dustBuffer2,sizeof(g_main_dustBuffer1));
}

static void hw_deinit(void)
{
	
//	dust_deinit();
	
	lcd_deinit();
	while (!HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_15));
	esp8266_deinit();
	sensorPowerOff();
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_6,GPIO_PIN_RESET); // co2 power off
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_9,GPIO_PIN_RESET); // ch2o power off

//	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_15,GPIO_PIN_RESET); // battery charge enable
	
//	HAL_GPIO_DeInit(GPIOB,GPIO_PIN_3);
	
	HAL_ADC_DeInit(&hadc);//all adc
	HAL_UART_DeInit(&huart1);//iap
	HAL_UART_DeInit(&huart2);//esp8266
	HAL_I2C_DeInit(&hi2c2);//th
//	HAL_I2C_DeInit(&hi2c1);//alx345
	HAL_TIM_Base_DeInit(&htim1);//tick
	
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_13,GPIO_PIN_RESET); // dc-dc off
	
	
	__DMA1_CLK_DISABLE();
  __GPIOA_CLK_DISABLE();

  __GPIOC_CLK_DISABLE();
  __GPIOD_CLK_DISABLE();
  __GPIOF_CLK_DISABLE();
	
	
}


static void adcResolutionBits(ADC_HandleTypeDef* h, uint32_t resolution)
{
	uint32_t tmp = h->Instance->CFGR1;
	tmp &=~ADC_CFGR1_RES;
	tmp |= resolution;
	h->Instance->CFGR1 = tmp;
	
	if ( h->DMA_Handle)
	{
		HAL_DMA_Abort(h->DMA_Handle);
		tmp = h->DMA_Handle->Instance->CCR;
		tmp &=~ (DMA_CCR_MSIZE|DMA_CCR_PSIZE);
		if (resolution == ADC_RESOLUTION6b || resolution == ADC_RESOLUTION8b)
		{
			tmp |= DMA_PDATAALIGN_BYTE|DMA_MDATAALIGN_BYTE;
		}
		else
		{
			tmp |= DMA_PDATAALIGN_HALFWORD|DMA_MDATAALIGN_HALFWORD;
		}
		h->DMA_Handle->Instance->CCR = tmp;
	}
}

uint16_t g_main_touchButton = 0;
#define ADC_BUTTON_CHANNEL 7
//void touchButton(void)
//{
//	HAL_ADC_PollForConversion(&hadc,10);
//	if (!(hadc.Instance->CHSELR & (1<<ADC_BUTTON_CHANNEL)))
//	{
//		HAL_ADC_Stop(&hadc);
//		hadc.Instance->CHSELR = 1<<ADC_BUTTON_CHANNEL;
//		adcResolutionBits(&hadc,ADC_RESOLUTION12b);
//	}
//	GPIO_InitTypeDef GPIO_InitStruct;
//	GPIO_InitStruct.Pin = GPIO_PIN_2;
//	GPIO_InitStruct.Pull = GPIO_NOPULL;
//	GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
//	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
//	uint16_t buffer = 0;
//	__set_PRIMASK(1);
////	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
//	GPIOB->MODER&=~(0x3<<6);
//	ADC1->CR |= ADC_CR_ADEN;
//	for (uint16_t i =0; i<480; i++)
//	{
//		__nop();
//	}
////	HAL_ADC_Start(&hadc);
//	ADC1->CR |= ADC_CR_ADSTART;
//	__set_PRIMASK(0);
//	for (uint16_t i =0; i<100; i++)
//	{
//		__nop();
//	}
////	HAL_ADC_Start_DMA(&hadc, (uint32_t*)&buffer,1);
////	HAL_ADC_PollForConversion(&hadc,2);
//	buffer = ADC1->DR;
//	if (buffer > 2000)
//	{
//		g_main_touchButton = 1;
//	}
//	else
//	{
//		g_main_touchButton = 0;
//	}
//	g_main_touchButton = buffer;
//	
//	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
//	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
//	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_3, GPIO_PIN_SET);
//}


uint8_t isButtonPressed(uint16_t id)
{
	switch (id)
	{
		case BUTTON_DOME:
			return !HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_15);
		case BUTTON_TOUCH:
			return !HAL_GPIO_ReadPin(GPIOD,GPIO_PIN_2);
		default:
			return 0;
	}
}

//void StandbyMode_Measure(void)
//{
//  /* Enable Power Clock*/
//  __PWR_CLK_ENABLE();

//  /* Allow access to Backup */
//  HAL_PWR_EnableBkUpAccess();

//  /* Reset RTC Domain */
//  __HAL_RCC_BACKUPRESET_FORCE();
//  __HAL_RCC_BACKUPRESET_RELEASE();
//  
//  /* The Following Wakeup sequence is highly recommended prior to each Standby mode entry
//     mainly  when using more than one wakeup source this is to not miss any wakeup event.
//       - Disable all used wakeup sources,
//       - Clear all related wakeup flags, 
//       - Re-enable all used wakeup sources,
//       - Enter the Standby mode.
//  */
//  
//  /*#### Disable all used wakeup sources: WKUP pin ###########################*/
//  HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN1);
//  
//  /*#### Clear all related wakeup flags ######################################*/
//  /* Clear PWR wake up Flag */
//  __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
//  
//  /* Enable WKUP pin */
//  HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1);

//  /* Request to enter STANDBY mode */
//  HAL_PWR_EnterSTANDBYMode();
//}



void enterSleep(void)
{
	fpe_write(MAIN_STATUS_ADDRESS,g_main_status);
	hw_deinit();
	
	HAL_Delay(100);
	{
		GPIO_InitTypeDef GPIO_InitStruct;
		__GPIOB_CLK_ENABLE();
		__GPIOA_CLK_ENABLE();
		__SYSCFG_CLK_ENABLE();
		

		GPIO_InitStruct.Pin = GPIO_PIN_15 ;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
		GPIO_InitStruct.Mode = GPIO_MODE_EVT_FALLING;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		GPIO_InitStruct.Pin = GPIO_PIN_3;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
		GPIO_InitStruct.Mode = GPIO_MODE_EVT_RISING;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	/* Enable and set Button EXTI Interrupt to the lowest priority */
		HAL_NVIC_SetPriority(EXTI4_15_IRQn, 0x02, 0x00);
		HAL_NVIC_SetPriority(EXTI2_3_IRQn, 0x02, 0x00);
		HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);
		HAL_NVIC_EnableIRQ(EXTI2_3_IRQn);
		
		__GPIOB_CLK_DISABLE();
		__GPIOA_CLK_DISABLE();
	}
	g_main_status = STATUS_POWEROFF;
	
	
	HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
	//StandbyMode_Measure();
	
	HAL_NVIC_SystemReset();
	
}
void onPowerButtonPressed(void)
{
//	console_printf(g_main_console,"Power off\r\n");
//	
	g_main_activeMs = tick_ms() + SYSTEM_ATUOPOFF_MS;
	if (g_main_status == STATUS_POWEROFF && g_main_is_charging)
	{
		g_main_status = STATUS_ALL_SHOW;
		
		//	HAL_NVIC_SystemReset();
		sensorPowerOn();
		esp8266PowerOn();
		return;
	}
	
	if (g_main_status!= STATUS_WIFI)
	{
		g_main_status = STATUS_POWEROFF;
		esp8266_smartStop();
				esp8266_deinit();
				sensorPowerOff();
	}
	else 
	{
		esp8266_smart(73000);
		updateWifiStatus();
	}
		

//	HAL_GPIO_WritePin(GPIOF,GPIO_PIN_4,GPIO_PIN_RESET);//power
//	HAL_Delay(100);
//	HAL_GPIO_WritePin(GPIOF,GPIO_PIN_4,GPIO_PIN_SET);//power
	//debug


}
void onModeButtonPressed(void)
{	
	g_main_activeMs = tick_ms() + SYSTEM_ATUOPOFF_MS;
//	g_main_key_test++;
//	if (g_isIniting ==1)
//	{
//		return;
//	} 
	if (g_main_status == STATUS_POWEROFF && g_main_is_charging)
	{
		
		g_main_status = STATUS_ALL_SHOW;
		

		sensorPowerOn();
		esp8266PowerOn();		
	}
	else if (g_main_status != STATUS_POWEROFF)
	{
		g_main_status++;
		if (g_main_status>STATUS_WIFI)
		{
		g_main_status = STATUS_ALL_SHOW;
		}
	lcd_update();
	}
//	
//	console_printf(g_main_console,"Status = %d\r\n",g_main_status);
}

void onMode2ButtonPressed(void)
{
	g_main_activeMs = tick_ms() + SYSTEM_ATUOPOFF_MS;
	if (g_main_status == STATUS_WIFI)
	{
			pairStart();
			if(esp8266_smarting())
		{
			esp8266_smartStop();
			esp8266_reset();
		}
	}
//		
//	//	g_main_wifi_interval = 3000;
//	}
//	else if (g_main_status != STATUS_POWEROFF)
//	{
//		g_main_status++;
//		if (g_main_status>STATUS_WIFI)
//		{
//		g_main_status = STATUS_SINGLE_SHOW;
//		}
//	}
}
//void onTouchButtonPressed(void)
//{
//	
//	onModeButtonPressed();
//	
//}
//#define abs(a) (a<0?-a:a)





void pairStart(void)
{
	g_main_pairToDo = 1;
	g_main_pairTick = tick_ms();

}
uint8_t tID[32];

//static void getID_PWD(void)
//{
//	uint8_t tea_key_ID[16]=
//{
// 1,8,6,1,2,5,2,6,8,8,4,1,3,6,8,1
//};
//uint8_t tea_key_pwd[16]=
//{
// 1,3,6,8,1,4,8,1,4,5,8,1,8,6,1,2
//};
//	char nBuffer[16];
////uint32_t DevID[2];
//	for (uint8_t i = 0;i<4;i++)
//	{
//	tID[i] = (*(uint32_t*)(0x1ffff7ac))>> (i*8) &0xff;
//	}
//	for (uint8_t i = 4;i<8;i++)
//	{
//	tID[i] = (*((uint32_t*)(0x1ffff7ac)+1))>> ((i-4)*8) &0xff;
//	}
//	for (uint8_t i = 8;i<12;i++)
//	{
//	tID[i] = (*((uint32_t*)(0x1ffff7ac)+2))>> ((i-8)*8) &0xff;
//	}	
//	for (uint8_t i = 12;i<16;i++)
//	{
//	tID[i] = 0;
//	}	
////	sprintf(tID, "M2-1234567B90ABC");	
////	DevID[0] = *(uint32_t*)(0x1ffff7b4);
//	encrypt(tID,16,tea_key_ID);
//	memcpy(nBuffer,tID,16);
//	encrypt(tID,16,tea_key_pwd);
//	for (uint8_t i=0;i<16;i++)
//	{
//	tID[16+i] = nBuffer[i];
//	};


	
//sprintf(tBuffer, "%02X%02X%02X%02X%02X%02X%02X%02X",  tID[0],tID[1],tID[2],tID[3],tID[4],tID[5],tID[6],tID[7]);		
//sprintf(tBuffer+16, "%02X%02X%02X%02X%02X%02X%02X%02X",  tID[0],tID[1],tID[2],tID[3],tID[4],tID[5],tID[6],tID[7]);	

//}
void lcd_setOffset(uint16_t rotation)
{
	switch (rotation)
	{
		case 0:
			xOffset = LCD_LEFT_X;
			yOffset = LCD_TOP_Y;
			break;
		case 90:
			xOffset = 320-LCD_TOP_Y-LCD_CIRCLE_D;
			yOffset = LCD_LEFT_X;
			break;
		case 180:
			xOffset = 240-LCD_LEFT_X-LCD_CIRCLE_D;
			yOffset = 320-LCD_TOP_Y-LCD_CIRCLE_D;

			break;
		case 270:
			xOffset = LCD_TOP_Y;
			yOffset = 240-LCD_LEFT_X-LCD_CIRCLE_D;

			break;
		default:
			return;
	}
}
//void drawCircle(void)
//{
//	if (g_main_status == STATUS_ALL_SHOW && g_isIniting != 1 )
//	{
//			static uint16_t color = 0x001f;
//		
//			static uint16_t angle = 325;
//			angle += 10;
//			if (angle > 315 +260)
//			{
//				angle = 315;

//				if (color == 0x001f)
//				{
//					color = rgb2c(0,144,255);
//				}
//				else
//				{
//					color = 0x001f;
//				}
//			}
//			lcd_circleMotion(angle,color);
//			
//	}

//}
//para：uid：MAC地址字符串
//			len：字符串长度
//			deviceType：设备类型字符串，如"D1"
//output	带前缀（如D1-），的加密后的字符串
char* uidEncyption(char* uid,uint16_t len,char* deviceType)
{
	uint8_t headerLength = strlen(deviceType);
	char* uidAfterEncyption = malloc(len);
	char* uid2ASCii = malloc((len<<1) + headerLength);
	if (uidAfterEncyption && uid2ASCii)
	{
		char key[8] = {1,8,6,1,2,5,2,1};
		uint16_t i = 0;
		for (i = 0; i < len;i ++)
		{
			*(uidAfterEncyption + i) = *(uid + i) ^ key[i & 0x07];	
		}
		for (i = 0; i < len;i ++)
		{
			sprintf(uid2ASCii +  headerLength + 1 + (i<<1),"%02X",*(uidAfterEncyption + i));
		}		
		for (i = 0; i < headerLength ; i ++)
		{
			*(uid2ASCii + i) = *(deviceType + i);				
		}
		*(uid2ASCii + i) = '-';
		free(uidAfterEncyption); 
		return uid2ASCii;	
	}
	return NULL;

}
void lcd_update(void)
{
	static uint8_t needRedraw = 0;

	
	static uint8_t status = STATUS_ALL_SHOW;
	if(status!= g_main_status || g_isIniting)
	{
		if (g_isIniting)
		{		
			g_isIniting = 0;
		}
		needRedraw = 1;		
		status = g_main_status;

//		lcd_setColor(NULL,&color);
	//	lcd_setOrigin(xOffset,yOffset);
		lcd_erase(240,320);
	}
	//	g_main_PM2_5Value = g_main_battery;
			
	#if (HCHO_ENABLE && CO2_ENABLE)
	uint32_t pmValue[5]={pm_pc_value.PM2_5Value,g_main_ch2o,g_main_co2,pm_pc_value.PM10Value,pm_pc_value.PM1Value};
	#elif (HCHO_ENABLE)
	uint32_t pmValue[5]={pm_pc_value.PM2_5Value,g_main_ch2o,0,pm_pc_value.PM10Value,pm_pc_value.PM1Value};
	#elif (CO2_ENABLE)
	uint32_t pmValue[5]={pm_pc_value.PM2_5Value,0,g_main_co2,pm_pc_value.PM10Value,pm_pc_value.PM1Value};
	#elif (PM25_SINGLE_MODE)
	uint32_t pmValue[6]={pm_pc_value.PM2_5Value,0,0,pm_pc_value.PM10Value,pm_pc_value.PM1Value,g_main_outDoor_pm25};
	#endif
	uint32_t dataValue[9] = {pm_pc_value.PM1Value,pm_pc_value.PM2_5Value,pm_pc_value.PM5Value,pm_pc_value.PM10Value,pm_pc_value.PC0_3Count,pm_pc_value.PC1_0Count,pm_pc_value.PC2_5Count,pm_pc_value.PC5_0Count,pm_pc_value.PC10Count};


	switch (g_main_status)
	{
		case STATUS_ALL_SHOW:
		{
//			static uint32_t test =567;
//			static uint32_t test1 =3;
//			static uint32_t test2 =5;
//			test1+=10;test2+=15;
//			uint8_t changetype = g_main_gas_type;
//			if (g_main_gas_type == GAS_CH2O)
//			{
//				changetype = 1;//bringup ch2o value
//			}		
//pm_pc_value.PM2_5Value = 10000;
			lcd_showAll (pmValue,needRedraw);
//			uint32_t angle = pm_pc_value.PM2_5Value  * 250/99999;
			lcd_showInfoHeader(g_main_wifi_status,g_main_battery,g_main_is_charging,&g_main_time,&g_main_date);
//			lcd_circleMotion(angle,rgb2c(0,144,255));


		break;
		}
//#if (!PM25_SINGLE_MODE)
		#if !NO_OUTDOOR_UI
		case STATUS_OUTDOOR:
//		#if HCHO_ENABLE
//		case STATUS_SINGLE_HCHO:
		{
//			static uint32_t test =567;
//			static uint32_t test1 =3;
//			static uint32_t test2 =5;
//			test1+=10;test2+=15;
//			uint8_t changetype = g_main_gas_type;
//			if (g_main_gas_type == GAS_CH2O)
//			{
//				changetype = 1;//bringup ch2o value
//			}		
		
			lcd_showInfoHeader(g_main_wifi_status,g_main_battery,g_main_is_charging,&g_main_time,&g_main_date);
//			lcd_showSingle(g_main_wifi_status,pmValue[g_main_status],g_main_status,g_main_battery,g_main_is_charging,needRedraw);
			lcd_showOutDoor(pm_pc_value.PM2_5Value,g_main_outDoor_pm25,needRedraw);			
		break;
		}
		#endif
	//	#endif
//#endif
		case STATUS_DATA_LIST:
		{
			lcd_showDataList(dataValue,needRedraw);
			lcd_showInfoHeader(g_main_wifi_status,g_main_battery,g_main_is_charging,&g_main_time,&g_main_date);
		break;
		}
		case STATUS_WIFI:
		{
			#if MAC2UID
			char * QRkey = uidEncyption(g_main_uid,strlen(g_main_uid),DEVICE_VERSION);
			if (QRkey)
			{
//				lcd_showQR(getSetting(g_main_uid),needRedraw);
				lcd_showQR(QRkey,needRedraw);
				free(QRkey);
			}
			#else
			lcd_showQR("http://www.hwlantian.com/static/pd/i.html#D1",needRedraw);
			#endif
			uint32_t smartTick = MAX(0,esp8266_getSmartTick());
			smartTick = MIN(99000,smartTick);
			lcd_showWifiStatus(g_main_wifi_status,needRedraw,smartTick);
			lcd_showInfoHeader(g_main_wifi_status,g_main_battery,g_main_is_charging,&g_main_time,&g_main_date);
		//	lcd_showTest(fmin(2000,wifistatus),0xffff);

			break;	
		}
//		case STATUS_ALL_SHOW:
//		{
//				#if DEBUG_MODE
//			g_main_PM2_5Value = g_main_battery;
//			#endif
//			
//			//uint32_t pmValue[3]={123,456,789};
//				lcd_showAll(pmValue,g_main_battery,g_main_is_charging,needRedraw);
//		
//		break;
//		}
//		case STATUS_HISTORY:
//		{
//			
//			history_t *pHistory[]={&g_pm25_history,&g_pm1_history,&g_pm10_history,&g_temprature_history,&g_humidity_history};

//			lcd_showBattery(g_main_battery,g_main_is_charging,0);	
//			uint32_t buffer[HISTORY_SIZE];
//			for (uint8_t i=0;i<HISTORY_SIZE;i++)
//			{
//				buffer[i] = readhistory(pHistory[g_main_gas_type],i);
//		
//			}
//		
//			
//			lcd_showhistory(buffer,HISTORY_SIZE,g_main_gas_type,pmValue[g_main_gas_type],needRedraw);					
//		
//		break;
//		}

		case STATUS_POWEROFF:
		{
		//	lcd_erase(LCD_CIRCLE_D,LCD_CIRCLE_D);
			if (g_main_is_charging)
			{
				UARTswitchChannel(0); //UART->USB
				if (B2_OUTPUT_FIX_MODE == 0)
				{
					B2_OUTPUT_FIX_MODE = 1;
				//	HAL_UART_DMAStop(&huart1);
					HAL_DMA_Abort(huart1.hdmarx);					
					huart1.State= HAL_UART_STATE_READY;  
					HAL_UART_Receive_DMA(&huart1, (uint8_t *)g_B2_configBuffer, 8);
//					huart1.pRxBuffPtr = (uint8_t *)g_B2_configBuffer;
//					huart1.RxXferSize = 8;
//					__IO uint32_t dst = (uint32_t )&g_B2_configBuffer;
//					HAL_DMA_Start_IT(huart1.hdmarx, (uint32_t)&huart1.Instance->RDR,dst, 8);
				//	HAL_UART_DMAResume(&huart1);
					thread_quickAdd(0,B2_output_fix,200,0,0);
				}			
			lcd_showBattery(g_main_battery,1,1);
			}
			else if (g_main_is_charging == 0)
			{
//				static uint8_t sleepCount = 6;
//				sleepCount --;
//				if (sleepCount == 0) 
//				{
//					sleepCount = 6;
//				enterSleep();
//			
//				}
				enterSleep();
			}			
		break;
		}
		default:
			break;
	}
	needRedraw = 0;
	lcd_refreshed =1;
}

#if 0


#endif




void history_record(void)
{
	//static uint32_t test = 12345;
	//test+=200;
//  pushhistory(&g_pm25_history,g_main_PM2_5Value/10);
//	
//	pushhistory(&g_pm10_history,g_main_PM10Value/10);
////	pushhistory(&g_co2_history,g_main_co2);
//	
//	pushhistory(&g_temprature_history,g_main_temp);


//		pushhistory(&g_pm1_history,g_main_PM1Value/10);

//	
//	pushhistory(&g_humidity_history,g_main_humidity);
}

void co2_tick(void)
{
	const uint16_t CO2T = 10;
	static uint16_t high,all;
	if(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_4) == GPIO_PIN_SET)
	{
		high ++;
	}
	all ++;
	if (all == 1004*CO2T)
	{
		if (high<2*CO2T)
		{
			g_main_co2 = 0;
		}
		else
		{
			g_main_co2 = (high-2*CO2T)*(CO2_MAX_PPM)/(all - 4*CO2T);
			g_main_co2 = MIN(CO2_MAX_PPM,g_main_co2);	
		}
		high = 0;
		all = 0;
	
	}
}


static void stable(int32_t value, int32_t* average, int32_t* variance)
{
	*average += (value-*average)*0.2 + 0.5;
	int32_t v;
	if (value-*average < INT16_MAX)
	{
		v = (value-*average)*(value-*average);
	}
	else
	{
		v = INT32_MAX;
	}
	*variance += (v-*variance)*0.5 + 0.5;
}
__IO uint8_t g_adcConvertDone = 0;



#define max(a,b) (a>b?a:b)
#define min(a,b) (a>b?b:a)
#define ADC_CH2O_CHANNEL 5
#if 1
void ch2o_update(void)
{
		//debug
	const uint16_t CH2O_GROUND = 390;
	uint8_t error = 0;
	uint16_t buffer[16] = {0};
	g_adcConvertDone = 0;
	HAL_ADC_PollForConversion(&hadc,10);
	if (!(hadc.Instance->CHSELR & (1<<ADC_CH2O_CHANNEL)))
	{
		HAL_ADC_Stop(&hadc);
		hadc.Instance->CHSELR = 1<<ADC_CH2O_CHANNEL;
		adcResolutionBits(&hadc,ADC_RESOLUTION12b);
	}
//	uint16_t tbuffer[8] = {0};
  __HAL_DMA_CLEAR_FLAG(hadc.DMA_Handle, __HAL_DMA_GET_TC_FLAG_INDEX(hadc.DMA_Handle));
	HAL_ADC_Start_DMA(&hadc, (uint32_t*)buffer,16);
//	HAL_ADC_PollForConversion(&hadc,1000);
	uint32_t startTick = HAL_GetTick();
	while(g_adcConvertDone == 0)
	{
		if (HAL_GetTick() - (startTick+2) < (UINT32_MAX>>1))
		{
			error = 1;
			break;
		}
	}
	if (error)
	{
		return;
	}
	uint32_t sum = 0;
	uint16_t cnt = 0;
	for (uint16_t i =0; i<sizeof(buffer)/sizeof(uint16_t);i++)
	{
		sum += buffer[i];
		cnt ++;
	}
	
	__IO float mv = UINT32_C(3300)*sum/cnt/(UINT32_C(1)<<12)+10;
	//mv = 2000;
	g_main_ch2o = (max(min(mv,2000),CH2O_GROUND) - CH2O_GROUND)*(5000)/(2000-CH2O_GROUND);
	
	g_main_ch2o = min(1500,max(g_main_ch2o*0.4,10));

	g_main_ch2o /= 10;
//	g_main_ch2o = 1;
//	static uint16_t test = 3;
//	test+=121;
//	g_main_ch2o = test;
}




#endif 
void pm2d5_init(void)
{
	//HAL_GPIO_WritePin(GPIOF,GPIO_PIN_5,GPIO_PIN_SET);
	memset(g_uart2Buffer,UINT8_MAX-1,sizeof(g_uart2Buffer));
	__IO uint8_t status = HAL_UART_Receive_DMA(&huart2, (uint8_t *)g_uart2Buffer, UART2_BUFFER_SIZE);
}

const static uint16_t dx[] = {3850,3600,8050,7450,13250,14350};
const static uint16_t dy[] = {5000,5000,10000,5200,5500,8500};
//const static uint16_t dx[] = {333,100,33,300,67,100,1033,400,333,1133,1000,600,1067,3333,1867,5333,1967,5133,4300,9933,11633,10000 };
//const static uint16_t dy[] = {1275,250,125,1250,50,425,1500,650,1125,1725,1725,1150,2025,4150,3650,5450,3375,4525,4500,6375,4700,10000 };
static uint32_t data2pm25(uint32_t value)
{
	#if !M1_MODE
	return MIN(99999,value);
	#endif
	uint8_t i=0;
	uint32_t pm25;

	uint32_t data = value;//dustinfo[i].tmp_value+dustinfo[i].toohigh_value
	uint32_t base;
	base = 0;
	for(i=0;i<(sizeof(dx)/sizeof(uint16_t))-1;i++)
	{
		if (data<dx[i])
		{
			break;
		}
		data -= dx[i];
		base += dy[i];
	}
	pm25 = base+ (data*dy[i])/dx[i];
	//pm25=data;
	if (pm25>99999UL)
	{
		pm25 = 99999UL;
	}
	return (uint32_t)pm25;
}
void dust_update(void)
{

	
	if (g_main_is_warmUp == 1 )
	{
		g_main_10s_countDown = MAX(g_main_10s_countDown-1,0);
		g_main_120s_countDown = MAX(g_main_120s_countDown-1,0);		
		if (g_main_120s_countDown == 0)
		{
//			lcd_showTest(221,rgb2c(255,0,0));
		  g_main_is_warmUp = 0; 
		}
	}	
	__IO uint32_t tbuffer[UART2_BUFFER_SIZE] = {0};
	__IO uint8_t start = 0;
	for(uint16_t i=0; i<UART2_BUFFER_SIZE; i++)
	{
		uint16_t c = g_uart2Buffer[i];
		if (c == 0x5a && g_uart2Buffer[(i+1) & UART2_BUFFER_MASK] == 0xa5 && g_uart2Buffer[(i+2) & UART2_BUFFER_MASK] == 0x34)
		{
			start = i;
			
			__IO uint16_t check = 0;
			//buffer[0] = 0;
			for( uint16_t j = 0; j<UART2_BUFFER_SIZE; j++)
			{
				tbuffer[j] = g_uart2Buffer[j+start & UART2_BUFFER_MASK];
			}
			check = 0;
			for( uint16_t j = 0; j<56; j=j+4)
			{				
			//if (j<=57)
				{
				check += tbuffer[j] + tbuffer[j+1] + (tbuffer[j+2]<<8) +(tbuffer[j+3]<<16) ;
				}
			
			}
			if (((tbuffer[58]<<8)+(tbuffer[59]))==check)
			{
				uint32_t number2_5,number1,number5,number10,pc0_3,pc1_0,pc2_5,pc5_0,pc10;
				 
				number1 = 	(tbuffer[11]<<24) +(tbuffer[10]<<16) +(tbuffer[9] <<8) + (tbuffer[8]);
				number2_5 =	(tbuffer[19]<<24) +(tbuffer[18]<<16) +(tbuffer[17]<<8) + (tbuffer[16]);
				number5 = 	(tbuffer[27]<<24) +(tbuffer[26]<<16) +(tbuffer[25]<<8) + (tbuffer[24]);
				number10 = 	(tbuffer[35]<<24) +(tbuffer[34]<<16) +(tbuffer[33]<<8) + (tbuffer[32]);
				
				pc0_3 = (tbuffer[39]<<24) +(tbuffer[38]<<16) +(tbuffer[37]<<8) + (tbuffer[36]);
				pc1_0 = (tbuffer[43]<<24) +(tbuffer[42]<<16) +(tbuffer[41]<<8) + (tbuffer[40]);
				pc2_5 = (tbuffer[47]<<24) +(tbuffer[46]<<16) +(tbuffer[45]<<8) + (tbuffer[44]);
				pc5_0 = (tbuffer[51]<<24) +(tbuffer[50]<<16) +(tbuffer[49]<<8) + (tbuffer[48]);
				pc10 = 	(tbuffer[55]<<24) +(tbuffer[54]<<16) +(tbuffer[53]<<8) + (tbuffer[52]);
				
				pc0_3 = MIN(99999,pc0_3);
				pc1_0 = MIN(99999,pc1_0);
				pc2_5 = MIN(99999,pc2_5);
				pc5_0 = MIN(99999,pc5_0);
				pc10 = 	MIN(99999,pc10);
				number1 = data2pm25(number1*PM_modifier);
				number2_5 = data2pm25(number2_5*PM_modifier);
				number5 = data2pm25(number5*PM_modifier);
				number10 = data2pm25(number10*PM_modifier);					
				pm_pc_value.PM1Value = 	number1;
				#if 0
				static uint32_t test = 200;
				test *= 1.1;
				pm_pc_value.PM2_5Value = test;
				#else
				pm_pc_value.PM2_5Value = number2_5;
				#endif

				pm_pc_value.PM5Value = number5;
				pm_pc_value.PM10Value 	= number10;
				
				pm_pc_value.PC0_3Count = pc0_3;
				pm_pc_value.PC1_0Count = pc1_0;
				pm_pc_value.PC2_5Count = pc2_5;
				pm_pc_value.PC5_0Count = pc5_0;
				pm_pc_value.PC10Count = pc10;

			}				
		}
		else if (c == 0xAA && g_uart2Buffer[(i+6) & UART2_BUFFER_MASK] == 0xFF)
		{				
			char str[15] = "at+outtype=4\r\n";
			UART_config_B2(str);				
		}
	}
	UART_config_B2("at+fco\r\n");	
	lcd_update();
}
void UART_config_B2(char *stringToSend)
{	
	
	HAL_UART_Transmit(&huart2,stringToSend,strlen(stringToSend),100);
	
}

void B2_output_fix(void)
{	
		//HAL_UART_Receive(&huart1,tbuffer,8,200);
		for(uint16_t i=0; i<6; i++)
		{
			uint8_t c = g_B2_configBuffer[i];
			
			if (c== 0x4c && g_B2_configBuffer[(i+1)&0x07] == 0x4A)
			{
				if (g_B2_configBuffer[(i+2)&0x07] == '+')
				{
					PM_modifier *= 1.05;				
				}
				else if (g_B2_configBuffer[(i+2)&0x07] == '-')
				{
					PM_modifier /= 1.05;			 	
				}
				PM_modifier = fmax(0.5,PM_modifier);

			//	uint8_t check = c + g_uart2Buffer[i+1] + runMode;
			//	if (ratio == 'N' || ratio == 'M')
				{
				fpe_write(PM_RATIO_ADDRESS,(uint32_t)(PM_modifier*100));
				}	
				char strToSend[10] = {0}; 
				sprintf(strToSend,"%4d",(uint32_t)(PM_modifier*100));
				HAL_UART_Transmit(&huart1,(uint8_t *)strToSend,6,10);
				//HAL_UART_Transmit(&huart1,tbuffer[(i+2)&0x07],1,100);
				break;
			}
		}
	memset(g_B2_configBuffer,0xff,8);
//	HAL_UART_Transmit(&huart1,"test",4,10);
}

typedef struct
{
	uint8_t H0_rH;
	uint8_t H1_rH;
	int16_t H0_T0_out;
	int16_t H1_T0_out;
	
	uint16_t  T0_degC;
	uint16_t  T1_degC;
	int16_t T0_out; 
	int16_t T1_out;	
	
	int16_t T_out;
	int16_t H_out;
}hts221_t;
hts221_t hts221_var;
//#define TH_I2C_ADDRESS 0xe0
#define ST_I2C_ADDRESS 0xBE
#define AMS_I2C_ADDRESS 0xB5
void hts221_init(void)
{
	uint8_t setupBytes[] = {0x81};
	uint8_t sendBytes[] = {0xB0};
	uint8_t retries = 3;
	while(retries>0)
	{
		if	(HAL_I2C_Mem_Write(&hi2c2,ST_I2C_ADDRESS,0x20,I2C_MEMADD_SIZE_8BIT, setupBytes,sizeof(setupBytes),15)==HAL_OK)
		{
			if (HAL_I2C_Master_Transmit(&hi2c2,ST_I2C_ADDRESS,sendBytes,sizeof(sendBytes),30) == HAL_OK)
			{	
				
				uint8_t receiveBytes[16]= {0};
				memset(receiveBytes,0xff,16);
				//HAL_Delay(10);
				if (HAL_I2C_Master_Receive(&hi2c2,ST_I2C_ADDRESS,receiveBytes,sizeof(receiveBytes),15) == HAL_OK)
				{
					hts221_var.T0_degC = (receiveBytes[2] | ((uint16_t)receiveBytes[5] & 0x03 )<<8)>>3;
					hts221_var.T1_degC = (receiveBytes[3] | ((uint16_t)receiveBytes[5] & 0x0C )<<6)>>3;
					hts221_var.T0_out = receiveBytes[12] | ((int16_t)receiveBytes[13]<<8 );
					hts221_var.T1_out = receiveBytes[14] | ((int16_t)receiveBytes[15]<<8 );
					
					hts221_var.H0_rH = receiveBytes[0]>>1;
					hts221_var.H1_rH = receiveBytes[1]>>1;
					hts221_var.H0_T0_out = receiveBytes[6] | ((uint16_t)receiveBytes[7]<<8 );
					hts221_var.H1_T0_out = receiveBytes[10] | ((uint16_t)receiveBytes[11]<<8 );
					break;
				}
			}
		}
		retries--;
	}
}
void th_fix(void)
{ 
	g_main_temp_fix = 0;
	if (tick_ms()>60000)
	{
		if (g_main_temp_fix<MAX_TEMP_FIX)
		{
			g_main_temp_fix ++;
		//g_main_temp_fix = fmin(3,g_main_temp_fix);
		}
}	
	
}
//void th_check_end(void)
//{

//	uint8_t receiveBytes[5]= {0};
//	memset(receiveBytes,0xff,5);
//	if (HAL_I2C_Master_Receive(&hi2c2,ST_I2C_ADDRESS,receiveBytes,sizeof(receiveBytes),15) == HAL_OK)
//	{
//		if (receiveBytes[0]==0x03)//check data ready register
//		{

//			hts221_var.T_out = (int16_t)(((uint16_t)receiveBytes[4] << 8) | receiveBytes[3] ); //actual temperature before calibration		
//			float slope = ( hts221_var.T0_degC*1.0 - hts221_var.T1_degC)/(hts221_var.T0_out*1.0 - hts221_var.T1_out); //slope of temp calibration line
//			float intercept = hts221_var.T0_degC*1.0  - slope * hts221_var.T0_out ; 							//intercept of temp calibration line

//			g_main_temp = (fmin(60,fmax(0,slope * hts221_var.T_out + intercept-g_main_temp_fix)))*100;

//			__IO int32_t nHumidity = 0;
//			hts221_var.H_out = (int16_t)(((uint16_t)receiveBytes[2] << 8) | receiveBytes[1] );//actual humidity before calibration
//			slope = ( hts221_var.H0_rH*1.0 - hts221_var.H1_rH)/(hts221_var.H0_T0_out*1.0 - hts221_var.H1_T0_out);//slope of humi calibration line
//			intercept = hts221_var.H0_rH*1.0  - slope * hts221_var.H0_T0_out;										//intercept of humi calibration line
//			nHumidity = fmin(9000,fmax(0,slope * hts221_var.H_out + intercept)*105);															//actual humidity

//			g_main_humidity = nHumidity;

//		}	
//	}

//}
//uint32_t g_main_voc = 0;
//void th_update(void)
//{

//		uint8_t sendBytes2[] = {0xA7};//ST
//		if (HAL_I2C_Master_Transmit(&hi2c2,ST_I2C_ADDRESS,sendBytes2,sizeof(sendBytes2),30) == HAL_OK)
//		{
//			delayCall_call(th_check_end,30,1,0);			
//		}
//		else 
//		{		
//			uint8_t receiveBytes[9] = {0}; 
//			if (HAL_I2C_Master_Receive(&hi2c2,AMS_I2C_ADDRESS,receiveBytes,sizeof(sendBytes2),30) == HAL_OK)
//			{
//				if (receiveBytes[2]==0)
//				{
//					g_main_voc = (receiveBytes[7]<<8) + receiveBytes[8];
//				
//				}
//			
//			}
//				
//		}

//}

#define ADC_BATTERY_CHANNEL 8
void battery_update(void)
{
	uint8_t error = 0;
	uint16_t buffer[16] = {0};
	g_adcConvertDone = 0;
//	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_15,GPIO_PIN_SET); // battery charge disable
	HAL_ADC_PollForConversion(&hadc,10);
	if (!(hadc.Instance->CHSELR & (1<<ADC_BATTERY_CHANNEL)))
	{
		HAL_ADC_Stop(&hadc);
		hadc.Instance->CHSELR = 1<<ADC_BATTERY_CHANNEL;
		adcResolutionBits(&hadc,ADC_RESOLUTION12b);
	}
	
	HAL_ADC_Start_DMA(&hadc, (uint32_t*)buffer,sizeof(buffer)/sizeof(uint16_t));
//	HAL_ADC_PollForConversion(&hadc,2);
		uint32_t startTick = HAL_GetTick();
	while(g_adcConvertDone == 0)
	{
		if (HAL_GetTick() - (startTick+5) < (UINT32_MAX>>1))
		{
			error = 1;
			break;
		}
	}
	if (error)
	{
		return;
	}
	
		uint32_t sum = 0;
	uint16_t cnt = 0;
	for (uint16_t i =1; i<sizeof(buffer)/sizeof(uint16_t);i++)
	{
		sum += buffer[i];
		cnt ++;
	}
	
	__IO int32_t voltage = UINT32_C(6600)*sum/cnt/(UINT32_C(1)<<12);
	
	static int32_t average = 0, variance = 1000;

	if (g_main_is_charging)
	{
		voltage = max(2500,1.35*voltage - 1562);		
//		fpe_write(0x0108,g_main_battery);
	}
	stable(voltage,&average,&variance);
	
	
//	if (variance>25)
//	{
//		g_main_battery = 100;
//	}
// 	else
		__IO uint16_t valueBeforeFC = 0;
		__IO uint16_t valueAfterFC = 0;
		uint16_t maxVoltage=0;
	if (g_main_is_charging)
	{\
		maxVoltage = 3900;		
	}
	else
	{
		maxVoltage = 3850;
	}
	{
		if (voltage < 3300)
		{
			valueBeforeFC = 0;
		}
		else if( voltage > maxVoltage)
		{
			valueBeforeFC = 115;
		}
		else
		{
			valueBeforeFC = (voltage - 3300)*100/(maxVoltage - 3300);
		}
	}
	__IO uint8_t USBtest = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_3);
	if (g_main_is_charging == 0 && USBtest )
	{		
		g_battery_notify_tick = tick_ms() + 2000;
		if (g_isIniting)
			g_battery_notify_tick += 1500;
		g_main_is_charging = 0xaa;
	}
	else if (g_main_is_charging!= 0xaa)
	{
		g_main_is_charging = USBtest;
	
	}
	static uint16_t lastValue = 20;
//	static uint16_t lastValue2 = 50;
	int16_t dealta = valueBeforeFC - lastValue;	
	if (g_main_is_charging == 0 && dealta>0)
	{
	dealta = dealta /2;
	}
	valueAfterFC = lastValue + dealta*0.4;
	lastValue = valueAfterFC;	


	if (USBtest == 0 )
	{		
	//	if (valueAfterFC2<valueAfterFC)
		{
		g_main_battery = valueAfterFC;
		}
		if (g_main_battery <= 12)
		{
			
			lcd_erase(240,320);
			lcd_showlogo(rgb2c(255,255,255),1);
			HAL_Delay(500);
			lcd_erase(240,320);
			HAL_Delay(500);
			lcd_showlogo(rgb2c(255,255,255),1);
			HAL_Delay(500);
			lcd_erase(240,320);
			g_main_status = STATUS_POWEROFF;
			
			enterSleep();
		}
#if AUTO_POWEROFF
		if (tick_ms() - g_main_activeMs < (UINT32_MAX>>1))
		{
			lcd_erase(240,320);
			g_main_status = STATUS_POWEROFF;
			enterSleep();
		}	
#endif		
	}

	else
	{
		g_main_activeMs = tick_ms() + SYSTEM_ATUOPOFF_MS;
		g_main_battery = valueAfterFC;		
	}
	
	if (g_main_is_charging ==0xaa && g_battery_notify_tick<(tick_ms()))
	{
		g_main_is_charging = 1;
	}
	
}





void updateWifiStatus(void)
{
	if (g_main_status == STATUS_POWEROFF)
	{
		return;
	}
	uint8_t status = esp8266_getStatus();
	switch(status)
	{
		case 1: //smart link
		{
		//	setLedList(LED_LIST_RAINBOW,sizeof(LED_LIST_RAINBOW)>>2,5000);
			g_main_wifi_status = WIFI_SMARTING;
			break;
		}
		case 3: //connceted		
		case 4: //unconncet
		{
			
			if (g_main_lastResponseTick+5000 - tick_ms() < (UINT32_MAX>>1))
			{
//				if (g_main_requestLedList && g_main_requestLedListSize && g_main_requestLedListTime)
//				{
//					setLedList(g_main_requestLedList,g_main_requestLedListSize,g_main_requestLedListTime);
//				}
//				else
//				{
//					setLedList(LED_LIST_GREEN_BREATH,sizeof(LED_LIST_GREEN_BREATH)>>2,3000);
//				}
				g_main_wifi_status = WIFI_SERVER_ACK;
				if ( g_main_neverUsed )
				{
					fpe_write(FPE_NEVERUSED_ADDRESS,0);
					g_main_neverUsed = 0;
				}
			}
			else 
			{
				if (status == 3)
				{
				g_main_wifi_status = WIFI_CONNECTED;
				}
				else
				{
				g_main_wifi_status = WIFI_REMOTE_ERROR;
				}
				//setLedList(LED_LIST_YELLOW_BLINK,sizeof(LED_LIST_YELLOW_BLINK)>>2,200);
				
			}
			break;
		}
		case 5: //no wifi
		{
			g_main_wifi_status = WIFI_NO_AP;
			//setLedList(LED_LIST_BLUE_BLINK,sizeof(LED_LIST_BLUE_BLINK)>>2,200);
			break;
		}
		case 2: //no wifi
		{
			g_main_wifi_status = WIFI_REMOTE_ERROR;;
			//setLedList(LED_LIST_BLUE_BLINK,sizeof(LED_LIST_BLUE_BLINK)>>2,200);
			break;
		}
	}
}
static void updateStringSetting(char* value, char** pCurrent)
{
	if (*pCurrent)
	{
		free(*pCurrent);
	}
	if (strlen(value) == 0)
	{
		*pCurrent = NULL;
	}
	else
	{
		char* copy = malloc(strlen(value)+1);
		if (copy)
		{
			strcpy(copy,value);
			*pCurrent = copy;
		}
	}
}

//static inline uint8_t hex2int(char c)
//{
//	uint8_t n = c;
//	if (n>='a')
//	{
//		n-='a'-10;
//	}
//	else if (n>='A')
//	{
//		n-='A'-10;
//	}
//	else if (n>='0')
//	{
//		n-='0';
//	}
//	else
//	{
//		n = 0;
//	}
//	if (n>0x10)
//	{
//		n = 0;
//	}
//	return n;
//}
char* g_main_tokenString = NULL;
__IO uint32_t g_main_wifi_interval = 1000;
void wifi_response(void)
{
	if (g_main_status == STATUS_POWEROFF)
	{
		return;
	}
	char* receive = esp8266_getReceiveString();
	if (!receive)
	{
		if(tick_ms() - g_main_lastWifiTick < (UINT32_MAX>>1))
		{
			if (!esp8266_smarting())
			{
				esp8266_reset();
				g_main_lastWifiTick = tick_ms() + RESET_WHILE_NO_DATEGET;
			}
		}
	}
	else
	{
		g_main_lastWifiTick = tick_ms() + RESET_WHILE_NO_DATEGET;
	}
	if (receive)
	{
		cJSON* json = cJSON_Parse(receive);
		if(json)
		{
			g_main_lastResponseTick = tick_ms();
			cJSON* child;
			child = cJSON_GetObjectItem(json,"udpAddress");
			if (child)
			{
				updateStringSetting(child->valuestring, &g_main_udpAddress);
			}
			child = cJSON_GetObjectItem(json,"udpPort");
			if (child)
			{
				g_main_udpPort = child->valueint;
			}
			child = cJSON_GetObjectItem(json,"slope");
			if (child)
			{
				uint32_t modifier =  child->valueint;
				if (modifier != (PM_modifier*100)) 
				{
					PM_modifier = (child->valueint)/100.0;
					fpe_write(PM_RATIO_ADDRESS,(uint32_t)(PM_modifier*100));
				}					
			}
			#if (!MAC2UID)
			child = cJSON_GetObjectItem(json,"setUid");
			if (child)
			{
				updateStringSetting(child->valuestring, &g_main_uid);
				char* uid = child->valuestring;
				fpe_writeString(FPE_UID_ADDRESS,uid);
			}
			#endif
			if (g_main_pairToDo)
			{
				child = cJSON_GetObjectItem(json,"pairConfirmed");
				if (child)
				{
					int16_t confirmed = child->valueint;
					int16_t pairTick = tick_ms() - g_main_pairTick;
					if (ABS(confirmed-pairTick)<1000)
					{
						g_main_pairToDo = 0;
					}
				}
			}
			if (g_main_ssidToDo)
			{
				child = cJSON_GetObjectItem(json,"ssidConfirmed");
				if (child)
				{
					char* confirmed = child->valuestring;
					char* ssid = esp8266_getSSID();
					if (strcmp(confirmed,ssid) == 0)
					{
						g_main_ssidToDo = 0;
					}
					if (ssid)
					{
						free(ssid);
					}
				}
			}
			
			child = cJSON_GetObjectItem(json,"setInterval");
			uint32_t interval = 0;
			if (child)
			{
				interval = child->valueint;//1 to 10s
			}	
			if (interval>=1000)
			{
				if (interval != g_main_wifi_interval)// || g_main_wifi_status != WIFI_CONNECT)
				{
					g_main_wifi_interval = interval;
					thread_remove(0,mainThread);
					thread_t* t = (thread_t*) malloc(sizeof(thread_t));
					t->priority = 0;
					t->remainTimes = 0;
					t->executeTick = tick_ms()+g_main_wifi_interval;
					t->function = mainThread;
					t->intervalTick = g_main_wifi_interval;
					t->minNeedTick = 0;
					t->next = NULL;
					if (thread_add(0,t,esp8266_run) == 0)
					{
						free(t);
					}
				//	g_main_wifi_getDelay = 5000;
				//	g_main_wifi_triggerTick = tick_ms()+g_main_wifi_interval+g_main_wifi_getDelay;
				}
			}
			#if 0
			cJSON* timer;
			timer = cJSON_GetObjectItem(json,"timer");
			if (timer)
			{
				//cJSON* child;

				child = cJSON_GetObjectItem(timer,"year");
				if (child)
				{
					g_main_date.Year = child->valueint%100;
				}
				child = cJSON_GetObjectItem(timer,"month");
				if (child)
				{
					g_main_date.Month = child->valueint;
				}				
				child = cJSON_GetObjectItem(timer,"day");
				if (child)
				{
					g_main_date.Date = child->valueint;
				}
//				child = cJSON_GetObjectItem(timer,"weekday");
//				if (child)
//				{
//					g_main_date.WeekDay = child->valueint;
//				}
				child = cJSON_GetObjectItem(timer,"hour");
				if (child)
				{
					g_main_time.Hours = child->valueint;
				}
				child = cJSON_GetObjectItem(timer,"minute");
				if (child)
				{
					g_main_time.Minutes = child->valueint;
				}
			}
			#endif

//{  
//   "air":{  
//      "aqi":46.0,
//      "pm25":16.0,
//      "pm10":46.0,
//      "so2":11.0,
//      "no2":35.0,
//      "co":0.7,
//      "o3":48.0,
//      "station":"东四",
//      "timestamp":1489456800000
//   },
//   "location":{  
//      "latitude":116.427767,
//      "longitude":39.950804
//   },
//   "distance":{  
//      "value":4.488183949496645,
//      "metric":"kilometers"
//   },
//   "token":{  
//      "value":"9f8365fb-a49b-4b9a-9f3f-afc6f6772cac"
//   }
//}

			cJSON* AQIcontent;
			AQIcontent = cJSON_GetObjectItem(json,"air");

			if (AQIcontent)
			{
				child = cJSON_GetObjectItem(AQIcontent,"pm25");
				if (child)
				{
//					cJSON* nexgtChild;
//					nexgtChild = cJSON_GetObjectItem(child,"pm25");				
//					if (nexgtChild)
					{
						g_main_outDoor_pm25 = (child->valuedouble)*100;
					}
				}

			}
//			child = cJSON_GetObjectItem(json,"token");
//			if (child)
//			{
//				cJSON* token = cJSON_GetObjectItem(child,"value");
//				if (token)
//				{
//					if (g_main_tokenString)
//					{
//						free(g_main_tokenString);
//					}
//					uint8_t len = strlen(token->valuestring);
//					g_main_tokenString = malloc(len + 1);
//					strncpy(g_main_tokenString,token->valuestring,len);				
//				
//				}

//				
//			}
			cJSON_Delete(json);
		}
		free(receive);
	}
}

void wifiSmartLinkStarted(void)
{
	g_main_ssidToDo = 1;
	pairStart();
}

void mainThread(void)
{
//	static uint16_t count = 0;
	if (g_main_status == STATUS_POWEROFF)
	{
		return;
	}
	if (!lcd_refreshed)
	{
	return;
	}

//	th_update();
	cJSON* json = cJSON_CreateObject();
	if (json)
	{
		#if MAC2UID
		cJSON_AddStringToObject(json,"devId",getSetting(g_main_uploadId));
		#else
		cJSON_AddStringToObject(json,"devId",getSetting(g_main_uid));
		#endif
		cJSON_AddStringToObject(json,"ver",DEVICE_VERSION_VER); //special ver string upload to server
		cJSON_AddNumberToObject(json,"slope",(uint32_t)PM_modifier*100);
//		if (g_main_power)
		{
			cJSON_AddNumberToObject(json,"pm2d5",pm_pc_value.PM2_5Value/100.0);
			cJSON_AddNumberToObject(json,"pm1d0",pm_pc_value.PM1Value/100.0);
			cJSON_AddNumberToObject(json,"pm10",pm_pc_value.PM10Value/100.0);
			#if HCHO_ENABLE
			cJSON_AddNumberToObject(json,"ch2o",g_main_ch2o*10);
			#endif
			#if CO2_ENABLE
			cJSON_AddNumberToObject(json,"co2",g_main_co2);
			#endif
//			cJSON_AddNumberToObject(json,"temp",g_main_temp/100.0);
//			cJSON_AddNumberToObject(json,"hum",g_main_humidity/100.0);
		//	cJSON_AddNumberToObject(json,"check",atoi(g_main_uid + strlen(g_main_uid)-2)+g_main_PM2_5Value/100.0);
		}
		char* ssid = esp8266_getSSID();
		if (ssid)
		{
			cJSON_AddStringToObject(json,"ssid",ssid);
		}
		if (g_main_pairToDo)
		{
			uint32_t pairTick = tick_ms() - g_main_pairTick;
			if (pairTick > 120000)
			{
				g_main_pairToDo = 0;
			}
			else
			{
				cJSON_AddNumberToObject(json,"pair",pairTick);
			}
		}
		cJSON_AddNumberToObject(json,"systemTick",tick_ms());
		char * str = cJSON_PrintUnformatted(json);
		cJSON_Delete(json);
		if (ssid)
		{
			free(ssid);
		}
		if (str)
		{
			uint16_t length = strlen(str);
			char* buffer = malloc(length+2);
			if (buffer)
			{
				sprintf(buffer,"%s\n",str);
				char* address = getSetting(g_main_udpAddress);//?g_main_udpAddress:g_main_udpAddressDefault;
				uint16_t port = getSetting(g_main_udpPort);//g_main_udpPort!=0?g_main_udpPort:g_main_udpPortDefault;
				esp8266_udp(buffer,address,port,0);
				free(buffer);
			}
			free(str);
		}
	}
	lcd_refreshed = 0;
}

/**
  * @brief  get outdoor AQI data ,must run after valid wifi connection
	* @param  
  * @retval None
  */
void updateOutdoorAQI(void)
{
	if (g_main_status == STATUS_POWEROFF)
	{
		return;
	}
	cJSON* json = cJSON_CreateObject();
	if (json)
	{
		cJSON* token = cJSON_CreateObject();
		cJSON_AddItemToObject(json,"token",token);
		cJSON_AddStringToObject(token,"devId",getSetting(g_main_uploadId));
		uint8_t idBuffer[16];
		MD5_CTX md5;
		MD5Init(&md5);         		
		MD5Update(&md5,(unsigned char *)(g_main_tokenString),strlen(g_main_tokenString));		
		MD5Final(&md5,idBuffer);  
		
		char tokenString[32] = {0};
		if (tokenString)
		{
			for (uint8_t i = 0; i < 16;i ++)
			{
				sprintf(tokenString + strlen(tokenString),"%02X",idBuffer[i]);
			}
			cJSON_AddStringToObject(token,"value",tokenString);
			free(tokenString);
			
		}
		
		
		
		char * str = cJSON_PrintUnformatted(json);
		//char* buffer = "5f6e8f455f6f27d4";
		if (str)
		{
			char* address = getSetting(g_main_udpAddress);//?g_main_udpAddress:g_main_udpAddressDefault;
			uint16_t port = getSetting(g_main_udpPort);//g_main_udpPort!=0?g_main_udpPort:g_main_udpPortDefault;
			esp8266_udp(str,address,port,1);
			free(str);
		}
	cJSON_Delete(json);
	}


}

/* USER CODE END 0 */

int main(void)
{

  /* USER CODE BEGIN 1 */
//	EnterBootLoader();

  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();
	
  /* Initialize all configured peripherals */
//  MX_GPIO_Init();
//  MX_DMA_Init();
//  MX_ADC_Init();
//  MX_I2C1_Init();
//  MX_I2C2_Init();
//  MX_IWDG_Init();
//  MX_SPI2_Init();
//  MX_TIM1_Init();
//  MX_USART1_UART_Init();
//  MX_USART2_UART_Init();

  /* USER CODE BEGIN 2 */


	if (__HAL_PWR_GET_FLAG(PWR_FLAG_SB) != RESET)
  {
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB);
  }
	g_isIniting = 1;
	hw_init();
	fpe_init(62,63);
	uint32_t timeOut = tick_ms()+120000;
	while(1)
	{
		if (timeOut < tick_ms())
		{			
			break;
		}
		for (uint8_t i = 0; i < 5 ; i ++)
		{
			HAL_Delay(100);
			battery_update();			
		}
		if (g_main_is_charging)
		{
			lcd_showBattery(g_main_battery,g_main_is_charging,1);
		}		
		if (g_main_battery >= 12 )
		{
			break;
		}
		HAL_Delay(500);
	}
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_6,GPIO_PIN_SET); // co2 power on
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_9,GPIO_PIN_SET); // ch2o power on
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_2,GPIO_PIN_SET); // th power on
	UARTswitchChannel(0);
	//memset(g_uart2Buffer,UINT8_MAX-1,sizeof(g_uart2Buffer));
//	HAL_UART_Receive_DMA(&huart1, (uint8_t *)g_uart2Buffer, UART2_BUFFER_SIZE);
		
	loadFlashData();
	uint8_t receiveBytes[5]= {0};
	//if (HAL_I2C_Master_Receive(&hi2c2,ST_I2C_ADDRESS,receiveBytes,sizeof(receiveBytes),15) == HAL_OK)
	{
	hts221_init();
	}	
	tick_init(&htim1);
	thread_init(0,tick_ms());
//	g_main_console = console_init(&huart1,g_main_consoleBuffer,sizeof(g_main_consoleBuffer));
//	console_addListener(g_main_console,"cal",newCalibration,0);

//	lcd_loadData();	
// button
	button_init(BUTTON_DOME,isButtonPressed);
//	button_init(BUTTON_TOUCH,isButtonPressed);
	button_addListener(BUTTON_DOME,BUTTON_STATUS_HOLD,onPowerButtonPressed,0);
	button_addListener(BUTTON_DOME,BUTTON_STATUS_CLICKED_NO_NEXT,onModeButtonPressed,0);
	button_addListener(BUTTON_DOME,BUTTON_STATUS_DOUBLE_CLICKED,onMode2ButtonPressed,0);
//	button_addListener(BUTTON_TOUCH,BUTTON_STATUS_CLICKED_NO_NEXT,onTouchButtonPressed,0);
//	button_addListener(BUTTON_TOUCH,BUTTON_STATUS_HOLD,onTouchButtonPressed,0);	
	
	
	
	//* thread
	
	thread_quickAdd(0,button_run,10,0,0);
	thread_quickAdd(0,battery_update,250,1,0);
	#if 1
	thread_quickAdd(0,ch2o_update,1000,1,0);
	
	#endif
	thread_quickAdd(0,dust_update,1000,1,0);
//	thread_quickAdd(0,th_fix,90000,1,0);
//	thread_quickAdd(0,th_update,1000,1,0);
	
//	thread_quickAdd(0,lcd_update,500,1,0);
	thread_quickAdd(0,history_record,500,1,0);
	#if WIFI_ENABLE
	thread_quickAdd(0,wifi_response,100,1,0);
  thread_quickAdd(0,mainThread,1000,1,0);
	thread_quickAdd(0,esp8266_run,10,1,0);
//	thread_quickAdd(0,updateOutdoorAQI,5500,1,0);
	thread_quickAdd(0,updateWifiStatus,1000,1,0);
//	thread_quickAdd(0,drawCircle,25,1,0);
	#endif
//	thread_quickAdd(0,esp8266_run,10,1,0);
//	thread_quickAdd(0,wifi_deal,g_main_wifi_interval,1,0);
//	thread_quickAdd(0,updateWifiStatus,1000,1,0);
//	#endif
	//* tick
	thread_quickAdd(0,delayCall_run,10,0,0);
	tick_add(delayCall_tick,10);
//	tick_add(console_tick,100);
	tick_add(button_tick,10);
	tick_add(co2_tick,1);
	
	tick_start();

//	console_printf(g_main_console,"chip is on, OB->RDP = 0x");
//	console_printHex(g_main_console,OB->RDP,16);
//	console_printf(g_main_console,"\r\n");
//	HAL_Delay(100);
	
//	dust_update();

	//HAL_UART_Receive_DMA(&huart1, (uint8_t *)tbuffer, 8);
	UARTswitchChannel(1);//UART1 ->ESP8266
#if WIFI_ENABLE
	{
		esp8266_init(&huart1);
		esp8266_setSmartCallback(wifiSmartLinkStarted);
		#if MAC2UID
		char* mac = esp8266_getMAC();
		if (mac)
		{
		g_main_uid = (char*) malloc(strlen(mac)+1);		
		strcpy(g_main_uid,mac);
		*(g_main_uid + strlen(mac) ) = '\0';
		free(mac);
		}
//		g_main_uid = esp8266_getMAC();
//		char* mac = "5c:cf:7f:03:0e:6b";
		uint8_t idBuffer[16];
	//	char* uidString;
		MD5_CTX md5;
		MD5Init(&md5);         		
		MD5Update(&md5,(unsigned char *)(g_main_uid),strlen(g_main_uid));		
		MD5Final(&md5,idBuffer);  
		g_main_uploadId = malloc(32 + strlen(DEVICE_VERSION)+1);
		sprintf(g_main_uploadId,"%s-",DEVICE_VERSION); 
		for (uint8_t i = 0; i < 16;i ++)
		{
			sprintf(g_main_uploadId + strlen(g_main_uploadId),"%02X",idBuffer[i]);
		}
		#endif
	}
#endif

//	HAL_UART_Receive_DMA(&huart1, (uint8_t *)tbuffer, 8);

	pm2d5_init();
//	getID_PWD();
	__HAL_UART_CLEAR_IT(&huart2, UART_CLEAR_OREF);
	HAL_Delay(100);
	sensorPowerOn();
	UART_config_B2("at+outtype=4\r\n");
	HAL_Delay(100);
	UART_config_B2("at+sensitivity=4\r\n");
	   
	
//#if COMPRESS_BMP
//		__IO char buffer[1100] ={0};
//		__IO uint16_t size = 0;
//		UARTswitchChannel(0);
//		lcd_Compress(buffer,&size)
//		HAL_UART_Transmit(&huart1,buffer,size,300);
//#endif 
		g_main_activeMs = tick_ms() + SYSTEM_ATUOPOFF_MS;
		g_main_10s_countDown = 10;
		g_main_120s_countDown = 60;
		g_main_is_warmUp = 1;

		lcd_update();

	/* USER CODE END 2 */

  /* USER CODE BEGIN 3 */
  /* Infinite loop */
  while (1)
  {
		thread_run(0,tick_ms(),0,0);
  }
  /* USER CODE END 3 */

}

/** System Clock Configuration
*/
void SystemClock_Config(void)
{
#if 0
	RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSI14
                              |RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSI14State = RCC_HSI14_ON;
  RCC_OscInitStruct.HSICalibrationValue = 16;
  RCC_OscInitStruct.HSI14CalibrationValue = 16;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL12;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
   // Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
   // Error_Handler();
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_RTC;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK1;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
   // Error_Handler();
  }

  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
	
#else	
  __HAL_RCC_HSI_CALIBRATIONVALUE_ADJUST(16);
	__HAL_RCC_HSI_CALIBRATIONVALUE_ADJUST(16);
	__HAL_RCC_HSI14ADC_DISABLE();
  __HAL_RCC_HSI14_ENABLE();
	HAL_Delay(8);
  __HAL_RCC_HSI14_CALIBRATIONVALUE_ADJUST(16);
	/* Disable the main PLL. */
	__HAL_RCC_PLL_DISABLE();
	/* Wait till PLL is ready */  
	HAL_Delay(15);

	/* Configure the main PLL clock source, predivider and multiplication factor. */
	__HAL_RCC_PLL_CONFIG(RCC_PLLSOURCE_HSI,
											 RCC_PREDIV_DIV1,
											 RCC_PLL_MUL12);
	
	/* Enable the main PLL. */
	__HAL_RCC_PLL_ENABLE();
	
	HAL_Delay(10);
	MODIFY_REG(RCC->CFGR, RCC_CFGR_HPRE, RCC_SYSCLK_DIV1);
	HAL_Delay(11);
  MODIFY_REG(RCC->CFGR, RCC_CFGR_SW, RCC_SYSCLKSOURCE_PLLCLK);
	uint32_t circle = 10000;
	while(circle--);
	HAL_InitTick (TICK_INT_PRIORITY);
	
  __PWR_CLK_ENABLE();
  __HAL_RCC_USART1_CONFIG(RCC_USART1CLKSOURCE_PCLK1);
	
  __SYSCFG_CLK_ENABLE();;
	  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
#endif
}

/* ADC init function */
void MX_ADC_Init(void)
{

  ADC_ChannelConfTypeDef sConfig;

    /**Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion) 
    */
  hadc.Instance = ADC1;
  hadc.Init.ClockPrescaler = ADC_CLOCK_ASYNC;
  hadc.Init.Resolution = ADC_RESOLUTION12b;
  hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc.Init.ScanConvMode = ADC_SCAN_DIRECTION_FORWARD;
  hadc.Init.EOCSelection = EOC_SINGLE_CONV;
  hadc.Init.LowPowerAutoWait = DISABLE;
  hadc.Init.LowPowerAutoPowerOff = DISABLE;
  hadc.Init.ContinuousConvMode = ENABLE;
  hadc.Init.DiscontinuousConvMode = DISABLE;
  hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc.Init.DMAContinuousRequests = ENABLE;
  hadc.Init.Overrun = OVR_DATA_PRESERVED;
  HAL_ADC_Init(&hadc);

    /**Configure for the selected ADC regular channel to be converted. 
    */
  sConfig.Channel = ADC_CHANNEL_5;
 // sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_71CYCLES_5;
  HAL_ADC_ConfigChannel(&hadc, &sConfig);

}

///* I2C1 init function */
//void MX_I2C1_Init(void)
//{

//  hi2c1.Instance = I2C1;
//  hi2c1.Init.Timing = 0x2000090E;
//  hi2c1.Init.OwnAddress1 = 0;
//  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
//  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLED;
//  hi2c1.Init.OwnAddress2 = 0;
//  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
//  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLED;
//  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLED;
//  HAL_I2C_Init(&hi2c1);

//    /**Configure Analogue filter 
//    */
//  HAL_I2CEx_AnalogFilter_Config(&hi2c1, I2C_ANALOGFILTER_ENABLED);

//}

///* I2C2 init function */
void MX_I2C2_Init(void)
{

  hi2c2.Instance = I2C2;
  hi2c2.Init.Timing = 0x2000090E;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLED;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLED;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLED;
  HAL_I2C_Init(&hi2c2);

    /**Configure Analogue filter 
    */
  HAL_I2CEx_AnalogFilter_Config(&hi2c2, I2C_ANALOGFILTER_ENABLED);

}

/* IWDG init function */
//void MX_IWDG_Init(void)
//{

//  hiwdg.Instance = IWDG;
//  hiwdg.Init.Prescaler = IWDG_PRESCALER_4;
//  hiwdg.Init.Window = 4095;
//  hiwdg.Init.Reload = 4095;
//  HAL_IWDG_Init(&hiwdg);

//}

/* SPI2 init function */
//void MX_SPI2_Init(void)
//{

//  hspi2.Instance = SPI2;
//  hspi2.Init.Mode = SPI_MODE_MASTER;
//  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
//  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
//  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
//  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
//  hspi2.Init.NSS = SPI_NSS_SOFT;
//  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
//  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
//  hspi2.Init.TIMode = SPI_TIMODE_DISABLED;
//  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLED;
//  hspi2.Init.NSSPMode = SPI_NSS_PULSE_DISABLED;
//  HAL_SPI_Init(&hspi2);

//}

/* TIM1 init function */
void MX_TIM1_Init(void)
{
#if 1
  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;

  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 47;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 99;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  HAL_TIM_Base_Init(&htim1);

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig);

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig);
#else
	__TIM1_CLK_ENABLE();
	TIM1->PSC = 0x0000002f;
	TIM1->ARR = 0x00000063;
		
	HAL_NVIC_SetPriority(TIM1_BRK_UP_TRG_COM_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(TIM1_BRK_UP_TRG_COM_IRQn);
	#endif
}

/* USART1 init function */
void MX_USART1_UART_Init(void)
{
#if 1
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONEBIT_SAMPLING_DISABLED ;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  HAL_UART_Init(&huart1);
	#else
	__USART1_CLK_ENABLE();
	DMA1_Channel3->CCR    = 0x000030A0;
	USART1->CR1 	= 0x0000000D;
	USART1->BRR 	= 48000000/115200;
	USART1->CR1 |= 0x00000001;
	#endif
}

/* USART2 init function */
void MX_USART2_UART_Init(void)
{
#if 1
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONEBIT_SAMPLING_DISABLED ;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  HAL_UART_Init(&huart2);
#else
	__USART2_CLK_ENABLE();
	DMA1_Channel5->CCR    = 0x000030A0;
	USART2->CR1 	= 0x0000000D;
	USART2->BRR 	= 48000000/115200;
	USART2->CR1 |= 0x00000001;
	#endif	
}
void MX_RTC_Init(void)
{



    /**Initialize RTC and set the Time and Date 
    */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  HAL_RTC_Init(&hrtc);
	
//	
//  sTime.Hours = 0x0;
//  sTime.Minutes = 0x0;
//  sTime.Seconds = 0x0;
//  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
//  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
//  HAL_RTC_SetTime(&hrtc, &sTime, FORMAT_BCD);

//  sDate.WeekDay = RTC_WEEKDAY_MONDAY;
//  sDate.Month = RTC_MONTH_JANUARY;
//  sDate.Date = 0x1;
//  sDate.Year = 0x0;

//  HAL_RTC_SetDate(&hrtc, &sDate, FORMAT_BCD);
}
/** 
  * Enable DMA controller clock
  */
void MX_DMA_Init(void) 
{
  /* DMA controller clock enable */
  __DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

}

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __GPIOC_CLK_ENABLE();
  __GPIOF_CLK_ENABLE();
  __GPIOA_CLK_ENABLE();
  __GPIOB_CLK_ENABLE();
  __GPIOD_CLK_ENABLE();

  /*Configure GPIO pins : PC13 PC14 PC15 PC0 
                           PC1 PC2 PC3 PC4 
                           PC5 PC6 PC7 PC8 
                           PC9 PC10 PC11 PC12 */
	GPIO_InitStruct.Pin = GPIO_PIN_All;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PF0 PF1 PF4 PF5 
                           PF6 PF7 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_5 
                          |GPIO_PIN_6|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pins : PA0 PA1 PA4 PA7 
                           PA8 PA11 PA12  */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_6|GPIO_PIN_7 
                          |GPIO_PIN_8|GPIO_PIN_11|GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins :  PB5 PB6 
                           PB7 8 9 12 13*/
  GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7
												|GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_12|GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PD2 */
  GPIO_InitStruct.Pin = GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : PA15 BUTTON*/
//  GPIO_InitStruct.Pin = GPIO_PIN_15;
//  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
//  GPIO_InitStruct.Pull = GPIO_PULLUP;
//  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
//	
//		GPIO_InitStruct.Pin = GPIO_PIN_3;
//		GPIO_InitStruct.Pull = GPIO_NOPULL;
//		GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
//		GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
//		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct
		
		GPIO_InitStruct.Pin = GPIO_PIN_15 ;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
		GPIO_InitStruct.Mode = GPIO_MODE_EVT_FALLING;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		GPIO_InitStruct.Pin = GPIO_PIN_3;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
		GPIO_InitStruct.Mode = GPIO_MODE_EVT_RISING;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	/* Enable and set Button EXTI Interrupt to the lowest priority */
		HAL_NVIC_SetPriority(EXTI4_15_IRQn, 0x02, 0x00);
		HAL_NVIC_SetPriority(EXTI2_3_IRQn, 0x02, 0x00);
		HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);
		HAL_NVIC_EnableIRQ(EXTI2_3_IRQn);		
	  /*Configure GPIO pin : PB4  charge EN (low active*/
  GPIO_InitStruct.Pin = GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	
}

/* USER CODE BEGIN 4 */

//void EnterBootLoader()
//{
//	uint32_t addressSystemMerory = 0x1fffec00;
//	void((*BootLoaderEntry)()) = (void((*)()))(*((__IO uint32_t *)(addressSystemMerory+4)));
//	__set_MSP(*(__IO uint32_t*)addressSystemMerory);
//	(*BootLoaderEntry)();
//}

//void setAllLevelColor(void)
//{
//setLevelColor(GAS_PM25,g_main_PM2_5Value,uint16_t *color,uint8_t *level)

//}
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* h)
{
	HAL_ADC_Stop(&hadc);
	g_adcConvertDone = 1;
}
/* USER CODE END 4 */

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
/* User can add his own implementation to report the file name and line number,
ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
