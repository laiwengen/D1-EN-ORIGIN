#include "Dust.h"
#include "math.h"
#include "stdlib.h"
#include "string.h"

#define KVALUE 1e-4
#define MVALUE 10
#define CVALUE 4
#define RVALUE 10

#define max(a,b) a>b?a:b
#define min(a,b) a<b?a:b

typedef struct dust_fluctuat_t
{
	uint16_t increaseCnt;
	uint16_t decreaseCnt;
} dust_fluctuat_t;

typedef struct dust_uint_t
{
	uint16_t minWidth;
	uint16_t maxWidth;
	uint32_t count;
	dust_fluctuat_t countF;
	uint32_t countA;
	uint64_t mass;
	dust_fluctuat_t massF;
	uint64_t massA;
	uint16_t ms;
	struct dust_uint_t* next;
} dust_uint_t;


uint8_t volatile *g_dust_buffer1;
uint8_t volatile *g_dust_buffer2;
volatile uint16_t g_dust_triggerLevel = 20;
volatile uint16_t g_dust_bufferSize;
ADC_HandleTypeDef * g_dust_hadc;
dust_uint_t* g_dust_firstUint = NULL;

static void hw_setLaser(uint8_t on)
{
	//PA12
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,on?GPIO_PIN_SET:GPIO_PIN_RESET);
}

static void hw_adcResolution(ADC_HandleTypeDef* h, uint32_t resolution)
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
#define ADC_DUST_CHANNEL 4
static void hw_startDMA(void)
{
	//HAL_ADC_PollForConversion
	// PA0 channel 0;
	HAL_ADC_PollForConversion(g_dust_hadc,10);
	if (!(g_dust_hadc->Instance->CHSELR & (1<<ADC_DUST_CHANNEL)))
	{
		HAL_ADC_Stop(g_dust_hadc);
		g_dust_hadc->Instance->CHSELR = 1<<ADC_DUST_CHANNEL;
		hw_adcResolution(g_dust_hadc,ADC_RESOLUTION8b);
	}
	HAL_ADC_Start_DMA(g_dust_hadc,(uint32_t*)g_dust_buffer1, g_dust_bufferSize);
}

static void CalculateBySamplesAC(uint8_t volatile * buffer, uint16_t size)
{
	uint16_t i;
	int16_t currentdata = 0,last1 = 0, last2 = 0;
	int16_t kangle=0;
	int16_t width=0,height=0;
	enum
	{
		WAVE_LOW=0,
		WAVE_HIGH,
		WAVE_RISE,
		WAVE_FALL,
	} statu = WAVE_LOW;
	
	for(i=0; i<size; i++)
	{
		last2 = last1;
		last1 = currentdata;
		currentdata = buffer[i];
		// Step 1: Check Rising or Falling by the rules.
		if (statu == WAVE_LOW)
		{
			if ((currentdata>g_dust_triggerLevel) && (currentdata>last1)) //over trigger and k>0
			{
				statu = WAVE_RISE;
			}
		}
		else if (statu == WAVE_HIGH)
		{
			if (currentdata<g_dust_triggerLevel) //under trigger
			{
				statu = WAVE_FALL;
			}
			else if (currentdata - last2 < -kangle) //fall faster than the half of it rised.
			{
				statu = WAVE_FALL;
			}
			else if (currentdata-last2 < -(UINT8_MAX>>2)) //fall faster than 1/4 of the adc range.
			{
				statu = WAVE_FALL;
			}
			else if ((last1<=currentdata) && (last1<=last2) && (currentdata - g_dust_triggerLevel)<(height-(height>>3))) //fall then rise at no more higher than 7/8 of the height. 
			{
				statu = WAVE_FALL;
			}
			else if ((currentdata - last1 > 2*kangle))  // rise 2 times faster than it did.
			{
				statu = WAVE_RISE;
			}
		}
		
		//Step 2: Do something depends on the statu. 
		//If rising, reset the width and height, remember the k of the rise. Mark WAVE_HIGH.
		if (statu == WAVE_RISE)
		{
			width = 1;
			height = currentdata - g_dust_triggerLevel;
			kangle = (int16_t)currentdata-(int16_t)last1;
			statu = WAVE_HIGH;
		}
		//If keep High, increase the width, and find the new height.
		else if (statu == WAVE_HIGH)
		{
			if (height < currentdata - g_dust_triggerLevel)
			{
				height = currentdata - g_dust_triggerLevel;
			}
			width ++;
		}
		//If falling, calculate the particle value and add to the summation. Mark WAVE_LOW
		else if (statu == WAVE_FALL)
		{
			dust_uint_t* du = g_dust_firstUint;
			while(du)
			{
				if (width > du->minWidth && width<=du->maxWidth)
				{
					du->count ++;
					du->mass += width*height;
				}
				du = du->next;
			}
			statu = WAVE_LOW;
		}
	}
}

static uint64_t FluctuateCorrent(dust_fluctuat_t* df, uint64_t average, uint64_t value)
{
	int64_t realDelta = (int64_t)value - (int64_t)average;
	uint16_t count = 0;
	int64_t calculateDelta = 0;
	int64_t calculateValue = 0;
	if (realDelta > 0)
	{ 
		if (df->increaseCnt!= UINT32_MAX)
		{
			df->increaseCnt ++;
		}
		if (df->decreaseCnt < RVALUE)  //#define RVALUE 4
		{
			df->decreaseCnt = 0;
		}
		else
		{
			df->decreaseCnt -= RVALUE;
		}
		if (df->increaseCnt > CVALUE)   //#define CVALUE 4
		{
			count = df->increaseCnt - CVALUE;
		}
	}
	else
	{
		if (df->decreaseCnt!= UINT32_MAX)
		{
			df->decreaseCnt ++;
		}
		if (df->increaseCnt < RVALUE)
		{
			df->increaseCnt = 0;
		}
		else
		{
			df->increaseCnt -= RVALUE;
		}
		if (df->decreaseCnt > CVALUE)
		{
			count = df->decreaseCnt - CVALUE;
		}
	}
	calculateDelta = abs(realDelta*KVALUE*(1.0+MVALUE*count)); 
	calculateDelta = min(calculateDelta,abs(realDelta/2));  
	if (realDelta>0)
	{
		calculateValue = average + calculateDelta;
	}
	else
	{
		calculateValue = average - calculateDelta;
	}
	return (uint64_t)max(calculateValue-1,0);
}


void dust_init(ADC_HandleTypeDef* hadc, uint8_t* buffer1,uint8_t* buffer2, uint16_t size)
{
	g_dust_buffer1 = buffer1;
	g_dust_buffer2 = buffer2;
	g_dust_bufferSize = size;
	g_dust_hadc = hadc;
	HAL_GPIO_WritePin(GPIOF,GPIO_PIN_5,GPIO_PIN_SET);

}

void dust_deinit(void)
{
	HAL_GPIO_WritePin(GPIOF,GPIO_PIN_5,GPIO_PIN_RESET);


}


void dust_newSize(uint16_t minWidth, uint16_t maxWidth)
{
	dust_uint_t * du = malloc(sizeof(dust_uint_t));
	memset(du,0,sizeof(dust_uint_t));
	du->minWidth = minWidth;
	du->maxWidth = maxWidth;
	du->next = g_dust_firstUint;
	g_dust_firstUint = du;
}

void dust_run(void)
{
	//Step 1: Start AC sampling to the bufferForDMA.
	//wait for the last ANY converting 
	//turn to ac channel
	//turn on the laser
	#if !DEBUG_LASER_OFF
	hw_setLaser(1);
	#endif
	//setup the status
	//stat ad convert with dma
	hw_startDMA();
	
	//Step 2: Calculate paricle by the LAST CIRCLE AC samples, which be storen in the bufferForCalculate.
	//calculate with the last ac data. A complicate algorithm
	CalculateBySamplesAC(g_dust_buffer2,g_dust_bufferSize);  //* bufferForCalculate = g_SampleBufferAC2;
	
	//Step 3: Add the resault to the summation.
//	particleSum += (uint64_t)particle;
//	particleCnt ++;
//	
	dust_uint_t* du = g_dust_firstUint;
	while(du)
	{
		du->ms++;
		if(du->ms > 100)
		{
			du->countA = FluctuateCorrent(&du->countF,du->countA,du->count);
			du->count = 0;
			du->massA = FluctuateCorrent(&du->massF,du->massA,du->mass);
			du->mass = 0;
			du->ms = 0;
		}
		du = du->next;
	}
	//Step 4: Swap the buffer pointer for next cricle.
	{
		uint8_t volatile * ptrtmp;
		ptrtmp = g_dust_buffer2;
		g_dust_buffer2 = g_dust_buffer1;
		g_dust_buffer1 = ptrtmp;
	}
}

uint32_t dust_getData(uint16_t minWidth, uint16_t maxWidth, uint8_t type)
{
	dust_uint_t* du = g_dust_firstUint;
	while(du)
	{
		if (minWidth == du->minWidth && maxWidth == du->maxWidth)
		{
			if (type == 0)
			{
				return du->countA;
			}
			else
			{
				return du->massA;
			}
		}
		du = du->next;
	}
	return 0;
}


