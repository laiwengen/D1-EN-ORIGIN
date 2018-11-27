#include "LCD.h"
#include "stdlib.h"

#include "math.h"
#include "QRencode.h"
#include "stdio.h"
#include "lcd_img.h"
#include "string.h"
#include "mainConfig.h"

lcd_block_t* g_lcd_start = NULL;
int8_t g_main_10s_countDown;
int16_t g_main_120s_countDown;
uint16_t xOffset=0,yOffset=0;
void lcd_drawGradualBMP(const uint8_t * org,Rect_t * position,uint16_t Startcolor,uint16_t Endcolor,int8_t scale);
extern	uint8_t RUN_MODE;
static inline void hw_wdata(uint16_t value)
{
  GPIOF->BSRRH = GPIO_PIN_1;
  GPIOF->BSRRL = GPIO_PIN_0;
	GPIOC->ODR = value;
  GPIOA->BSRRH = GPIO_PIN_0;
  GPIOA->BSRRL = GPIO_PIN_0;
  GPIOF->BSRRL = GPIO_PIN_1;
}
static inline void hw_wcmd(uint16_t value)
{
  GPIOF->BSRRH = GPIO_PIN_1;
  GPIOF->BSRRH = GPIO_PIN_0;
	GPIOC->ODR = value;
  GPIOA->BSRRH = GPIO_PIN_0;
  GPIOA->BSRRL = GPIO_PIN_0;
  GPIOF->BSRRL = GPIO_PIN_1;
}
//static inline uint16_t hw_rdata(void)
//{
//	uint16_t value;
//	GPIOC->MODER = 0;
//  GPIOF->BSRRH = GPIO_PIN_1;
//  GPIOF->BSRRL = GPIO_PIN_0;
//  GPIOA->BSRRH = GPIO_PIN_1;
//  GPIOA->BSRRL = GPIO_PIN_1;
//	value = GPIOC->IDR;
//  GPIOF->BSRRL = GPIO_PIN_1;
//	GPIOC->MODER = 0x55555555;
//	return value;
//}



static void hw_deinit(void)
{
	
  GPIO_InitTypeDef GPIO_InitStruct;
	
	// pc0-pc15
  GPIO_InitStruct.Pin = GPIO_PIN_All;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
	
	// pf0 pf1 pf4 pf5 pf6
	
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
	
		  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_5,GPIO_PIN_RESET);//power
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_8,GPIO_PIN_RESET);//power
//	GPIOF->BRR= (uint32_t)GPIO_PIN_4;//BSRR reset fail..
	
}

static void hw_init(void)
{
	
	
//  GPIO_InitTypeDef GPIO_InitStruct;
//	
//	// pc0-pc15
//  GPIO_InitStruct.Pin = GPIO_PIN_All;
//  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
//  GPIO_InitStruct.Pull = GPIO_NOPULL;
//  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
//  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
//	
//	// pf0 pf1 pf4 pf5 pf6
//  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_6;
//  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
//  GPIO_InitStruct.Pull = GPIO_NOPULL;
//  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
//  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
//	
//	  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
//  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
//  GPIO_InitStruct.Pull = GPIO_NOPULL;
//  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
//  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	HAL_GPIO_WritePin(GPIOF,GPIO_PIN_0,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOF,GPIO_PIN_1,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_0,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_1,GPIO_PIN_SET);
//	HAL_GPIO_WritePin(GPIOF,GPIO_PIN_4,GPIO_PIN_SET);//power
	HAL_GPIO_WritePin(GPIOF,GPIO_PIN_6,GPIO_PIN_SET); //reset
	HAL_Delay(100);
	
	HAL_GPIO_WritePin(GPIOF,GPIO_PIN_6,GPIO_PIN_RESET); //reset
	HAL_Delay(100);
	HAL_GPIO_WritePin(GPIOF,GPIO_PIN_6,GPIO_PIN_SET); //reset
	HAL_Delay(500);
#if 0	
	hw_wcmd(0x00);  //nop
	
	hw_wcmd(0xCF);  //Power control B
	hw_wdata(0x00); 
	hw_wdata(0xd9); 
	hw_wdata(0X30);
	
	hw_wcmd(0xED);  //Power on sequence control
	hw_wdata(0x64); 
	hw_wdata(0x03); 
	hw_wdata(0X12); 
	hw_wdata(0X81);
	
	hw_wcmd(0xE8);  //Driver timing control A
	hw_wdata(0x85); 
	hw_wdata(0x00); 
	hw_wdata(0x78);
	
	hw_wcmd(0xCB);  //Power control A 
	hw_wdata(0x39); 
	hw_wdata(0x2C); 
	hw_wdata(0x00); 
	hw_wdata(0x34); 
	hw_wdata(0x02);
	
	hw_wcmd(0xF7);  //Pump ratio control
	hw_wdata(0x20);
	
	hw_wcmd(0xEA);  //Driver timing control B
	hw_wdata(0x00); 
	hw_wdata(0x00);
	
	hw_wcmd(0xC0);    //Power control 
	hw_wdata(0x1B);   //VRH[5:0] 
	
	hw_wcmd(0xC1);    //Power control 
	hw_wdata(0x13);   //SAP[2:0];BT[3:0] 
	
	hw_wcmd(0xC5);    //VCM control 
	hw_wdata(0x3f); 	 //3F
	hw_wdata(0x3c); 	 //3C
	
	hw_wcmd(0xC7);    //VCM control2 
	hw_wdata(0X96); 
	
		hw_wcmd(0x36);    //Memory Access Control
	hw_wdata(0xC8);	
	
	hw_wcmd(0x3A);   
	hw_wdata(0x55); 	
	
	hw_wcmd(0xB1);   //Frame Rate Control
	hw_wdata(0x00);   
	hw_wdata(0x10); 
	
	hw_wcmd(0xB6);    // Display Function Control 
	hw_wdata(0x0A); 
	hw_wdata(0xA2); 
	
	hw_wcmd(0xF2);    // 3Gamma Function Disable 
	hw_wdata(0x00); 
	
	hw_wcmd(0x26);    //Gamma curve selected 
	hw_wdata(0x01); 
	
	hw_wcmd(0xE0);    //Set Gamma 
	hw_wdata(0x0F); 
	hw_wdata(0x19); 
	hw_wdata(0x18); 
	hw_wdata(0x0A); 
	hw_wdata(0x0C); 
	hw_wdata(0x07); 
	hw_wdata(0x4D); 
	hw_wdata(0X84); 
	hw_wdata(0x41); 
	hw_wdata(0x09); 
	hw_wdata(0x15); 
	hw_wdata(0x07); 
	hw_wdata(0x12); 
	hw_wdata(0x06); 
	hw_wdata(0x00); 	
	
	hw_wcmd(0XE1);    //Set Gamma 
	hw_wdata(0x00);
	hw_wdata(0x1A);
	hw_wdata(0x1B);
	hw_wdata(0x03);
	hw_wdata(0x0F);
	hw_wdata(0x05);
	hw_wdata(0x33);
	hw_wdata(0x15);
	hw_wdata(0x47);
	hw_wdata(0x02);
	hw_wdata(0x0b);
	hw_wdata(0x0A);
	hw_wdata(0x35);
	hw_wdata(0x38);
	hw_wdata(0x0F);
	

	hw_wcmd(0x2A); 
	hw_wdata(0x00);
	hw_wdata(0x00);
	hw_wdata(0x00);
	hw_wdata(0xef);	
	
	hw_wcmd(0x2B); 
	hw_wdata(0x00);
	hw_wdata(0x00);
	hw_wdata(0x01);
	hw_wdata(0x3F);
		


	hw_wcmd(0x11); //Exit Sleep
//	Delayms(120);
	HAL_Delay(120);
	hw_wcmd(0x29); //display on
	hw_wcmd(0x2c);
#else //ST7789V

hw_wcmd(0x00B2);  
hw_wdata(0x000C); 
hw_wdata(0x000C); 
hw_wdata(0x0000); 
hw_wdata(0x0033);
hw_wdata(0x0033);

hw_wcmd(0x00B7);  
hw_wdata(0x0035); 

hw_wcmd(0x00BB);  
hw_wdata(0x0015);    //1f

hw_wcmd(0x00C0);  
hw_wdata(0x002C); 

hw_wcmd(0x00C2);  
hw_wdata(0x0001); 
 
hw_wcmd(0x00C3);  
hw_wdata(0x0017); 

hw_wcmd(0x00C4);  
hw_wdata(0x0030);		  //20

hw_wcmd(0x00C6);  
hw_wdata(0x0006);			//0f

hw_wcmd(0x00CA);  
hw_wdata(0x000F);

hw_wcmd(0x00C8);  
hw_wdata(0x0008);


hw_wcmd(0x0055);  
hw_wdata(0x0090);		//90


hw_wcmd(0x00D0);  
hw_wdata(0x00A4); 
hw_wdata(0x00A1); 
 
hw_wcmd(0x00E0);    //Set Gamma 
hw_wdata(0x00d0); 
hw_wdata(0x0000); 
hw_wdata(0x0014); 
hw_wdata(0x0015); 
hw_wdata(0x0013); 
hw_wdata(0x002c); 
hw_wdata(0x0042); 
hw_wdata(0x0043); 
hw_wdata(0x004e); 
hw_wdata(0x0009); 
hw_wdata(0x0016); 
hw_wdata(0x0014); 
hw_wdata(0x0018);
hw_wdata(0x0021); 
 
 
hw_wcmd(0x00E1);    //Set Gamma 
hw_wdata(0x00d0); 
hw_wdata(0x0000); 
hw_wdata(0x0014); 
hw_wdata(0x0015); 
hw_wdata(0x0013); 
hw_wdata(0x000b); 
hw_wdata(0x0043); 
hw_wdata(0x0055); 
hw_wdata(0x0053); 
hw_wdata(0x000c); 
hw_wdata(0x0017); 
hw_wdata(0x0014); 
hw_wdata(0x0023); 
hw_wdata(0x0020); 

hw_wcmd(0x0036);  
hw_wdata(0x0008); 
hw_wcmd(0x003A); 
hw_wdata(0x0055); 

hw_wcmd(0x002A); 
hw_wdata(0x0000);
hw_wdata(0x0000);
hw_wdata(0x0000);
hw_wdata(0x00ef);	

hw_wcmd(0x002B); 
hw_wdata(0x0000);
hw_wdata(0x0000);
hw_wdata(0x0001);
hw_wdata(0x003f);
 
hw_wcmd(0x0011); 
HAL_Delay(120); 

hw_wcmd(0x0029); //display on
hw_wcmd(0x002c);
#endif
}

//static void hw_setScroll(uint16_t top, uint16_t bottom, uint16_t scroll)
//{
//	hw_wcmd(0x33);
//	hw_wdata(top>>8);
//	hw_wdata(top&0xff);
//	hw_wdata(scroll>>8);
//	hw_wdata(scroll&0xff);
//	hw_wdata(bottom>>8);
//	hw_wdata(bottom&0xff);
//	
//	hw_wcmd(0x37);
//	hw_wdata(50>>8);
//	hw_wdata(50&0xff);
//}

static void hw_setDir(uint16_t degree)
{
	uint8_t mymxmv = 0;
	switch (degree)
	{
		case 0:
			mymxmv = 0x00;
			break;
		case 90:
			mymxmv = 0x05;
			break;
		case 180:
			mymxmv = 0x06;
			break;
		case 270:
			mymxmv = 0x03;
			break;
		default:
			break;
	}
//	hw_wcmd(0x36);    //Memory Access Control
//	hw_wdata((mymxmv<<5)|0x8);	    //  000
}

static void hw_setOrigin(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
	//used
	hw_wcmd(0x2a);
	hw_wdata(x>>8);
	hw_wdata(x&0xff);
	hw_wdata((x+w-1)>>8);
	hw_wdata((x+w-1)&0xff);

	hw_wcmd(0x2b);   
	hw_wdata(y>>8);
	hw_wdata(y&0xff);
	hw_wdata((y+h-1)>>8);
	hw_wdata((y+h-1)&0xff);
		
	hw_wcmd(0x3A);   
	hw_wdata(0x55);
	hw_wcmd(0x2c);
}

uint16_t g_lcd_xOrigin = 0, g_lcd_yOrigin = 0, g_lcd_rotation = UINT16_MAX;
uint16_t g_lcd_printColor = 0, g_lcd_backColor = 0;
static void hw_fill(uint16_t width, uint16_t height, uint16_t data)
{
	for (uint16_t i = 0 ; i < width; i++)
	{
		for(uint16_t j = 0 ; j < height; j++)
		{
			hw_wdata(data);
		}
	}
}
//static void hw_drawOneBitUint(lcd_unit_t* lu)
//{
//	for(uint16_t i = 0; i<lu->height; i++)
//	{
//		for(uint16_t j = 0; j<lu->realWidth; j++)
//		{
//			uint16_t data;
//			if (j>=lu->width)
//			{
//				data = g_lcd_backColor;
//			}
//			else
//			{
//				uint16_t dataPerLine = (lu->width + 0x07)>>3;
//				uint16_t dataIndex = (i*dataPerLine) + (j>>3);
//				uint16_t dataMask = 0x80>>(j&0x07);
//				if ((*(lu->data + dataIndex))&dataMask)
//				{
//					data = g_lcd_printColor;
//				}
//				else
//				{
//					data = g_lcd_backColor;
//				}
//			}
//			hw_wdata(data);
//		}
//	}
//}

//static void hw_drawRealColorUint(lcd_unit_t* lu)
//{
//	for(uint16_t i = 0; i<lu->height; i++)
//	{
//		for(uint16_t j = 0; j<lu->width; j++)
//		{
//			uint16_t data;
//			uint16_t dataIndex = (i * lu->width + j)<<1;
//			data = *(((uint16_t*)(lu->data)) + dataIndex);
//			hw_wdata(data);
//		}
//	}
//}


void lcd_init(void)
{
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_5,GPIO_PIN_SET);//power
	hw_init();
	lcd_erase(240,320);
	lcd_setRotation(0);
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_8,GPIO_PIN_SET);//power

	
}

void lcd_deinit(void)
{
	hw_deinit();
}

//static lcd_block_t* getBlock(uint16_t groupId)
//{
//	lcd_block_t* b = g_lcd_start;
//	while(b)
//	{
//		if(b->groupId == groupId)
//		{
//			return b;
//		}
//		b=b->next;
//	}
//	return NULL;
//}

//static lcd_block_t* newBlock(uint16_t groupId, int16_t type)
//{
//	lcd_block_t* lb = (lcd_block_t*)malloc(sizeof(lcd_block_t));
//	lb->first = NULL;
//	lb->groupId = groupId;
//	lb->type = type;
//	lb->next = g_lcd_start;
//	g_lcd_start = lb;
//	return lb;
//}

//static lcd_unit_t* getUnit(lcd_block_t* lb, uint16_t key)
//{
//	lcd_unit_t* u = lb->first;
//	while(u)
//	{
//		if(u->key == key)
//		{
//			return u;
//		}
//		u=u->next;
//	}
//	return NULL;
//}

//void lcd_regester(uint16_t groupId, int16_t type, lcd_unit_t* uint)
//{
//	lcd_block_t* lb = getBlock(groupId);
//	if (!lb)
//	{
//		lb = newBlock(groupId,type);
//	}
//	if ( lb->type != type)
//	{
//		return;
//	}
//	uint->next = lb->first;
//	lb->first = uint;
//}

void lcd_setOrigin(uint16_t x, uint16_t y)
{
	g_lcd_xOrigin = x;
	g_lcd_yOrigin = y;
}

uint8_t lcd_setRotation(uint16_t degree)
{
	if (degree != g_lcd_rotation)
	{
		g_lcd_rotation = degree;
		hw_setDir(degree);
		return 1;
	}
	return 0;
}

//void lcd_setColor(uint16_t* pPrintColor, uint16_t* pBackColor)
//{
//	if (pPrintColor)
//	{
//		g_lcd_printColor = *pPrintColor;
//	}
//	if (pBackColor)
//	{
//		g_lcd_backColor = *pBackColor;
//	}
//}

//void lcd_fill(uint16_t width, uint16_t height)
//{
//	hw_setOrigin(g_lcd_xOrigin,g_lcd_yOrigin,width,height);
//	hw_fill(width,height,g_lcd_printColor);
//}

void lcd_erase(uint16_t width, uint16_t height)
{
	hw_setOrigin(g_lcd_xOrigin,g_lcd_yOrigin,width,height);
	hw_fill(width,height,g_lcd_backColor);
}
void lcd_drawRectangle(Rect_t *pos,uint16_t color)
{
	
	hw_setOrigin(pos->x,pos->y,pos->w,pos->h);
	hw_fill(pos->w,pos->h,color);
	
}



