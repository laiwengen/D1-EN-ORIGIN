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
	HAL_Delay(200);
	
#ifndef LCD_DRIVER_ST7789	
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

hw_wcmd(0x0011); 
HAL_Delay(120); 
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
hw_wdata(0x0020);		  //20

hw_wcmd(0x00C6);  
hw_wdata(0x0006);			//0f

//hw_wcmd(0x00CA);  
//hw_wdata(0x000F);

//hw_wcmd(0x00C8);  
//hw_wdata(0x0008);


//hw_wcmd(0x0055);  
//hw_wdata(0x0090);		//90


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
hw_wdata(0x0000); 
hw_wcmd(0x003A); 
hw_wdata(0x0005); 

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
	hw_wcmd(0x36);    //Memory Access Control
#ifdef LCD_DRIVER_ST7789
	hw_wdata((mymxmv<<5)|0x0);	    //  000
#else
	hw_wdata((mymxmv<<5)|0x08);	    //  000
	#endif
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

//void lcd_print(uint16_t groupId, char* str, uint8_t anchor)
//{
//	if(str == NULL)
//	{
//		return;
//	}
//	lcd_block_t* lb = getBlock(groupId);
//	if (!lb)
//	{
//		return;
//	}
//	if (str == NULL)
//	{
//		return;
//	}
//	if ((anchor&0xf0) == 0x20 || (anchor&0xf0) == 0x10 )
//	{
//		uint16_t xOffset = 0;
//		char*strt = str;
//		while(*strt != '\0')
//		{
//			uint16_t key = *strt;
//			lcd_unit_t* lu = getUnit(lb,key);
//			if(lu)
//			{
//				xOffset += lu->realWidth;
//			}
//			strt ++;
//		}
//		if ((anchor&0xf0) == 0x20)
//		{
//			xOffset>>=1;
//		}
//		g_lcd_xOrigin -= xOffset;
//	}
//	
//	while(*str != '\0')
//	{
//		uint16_t key = *str;
//		lcd_unit_t* lu = getUnit(lb,key);
//		if(lu)
//		{
//			if(g_lcd_xOrigin<(UINT16_MAX>>1))
//			{
//				hw_setOrigin(g_lcd_xOrigin,g_lcd_yOrigin,lu->realWidth,lu->height);
//				if (lb->type == 1)
//				{
//					hw_drawOneBitUint(lu);
//				}
//				else if(lb->type == 16)
//				{
//					hw_drawRealColorUint(lu);
//				}
//			}
//			g_lcd_xOrigin += lu->realWidth;
//		}
//		str ++;
//	}
//}

//uint16_t lcd_printDirect(uint16_t groupId, char* str, uint8_t anchor)
//{
//	lcd_block_t* lb = getBlock(groupId);
//	uint16_t width = 0;
//	if (lb == NULL)
//	{
//		return 0;
//	}
//	if (str == NULL)
//	{
//		return 0;
//	}
//	lcd_unit_t* lu = lb->first;
//	if (lu == NULL)
//	{
//		return 0;
//	}
//	lcd_unit_t* lut = malloc(sizeof(lcd_unit_t));
//	if (lut == NULL)
//	{
//		return 0;
//	}
//	{
//		char*strt = str;
//		while(*strt != '\0')
//		{
//			uint16_t key = *strt;
//			lut->data = lu->data+((lu->width+7>>3)*lu->height*(key-lu->key));
//			lut->width = lu->width;
//			lut->height = lu->height;
//			lut->realWidth = lcd_getRealWidth(lut)+1;
//			width += lut->realWidth;
//			strt ++;
//		}
//		if ((anchor&0xf0) == 0x20)
//		{
//			g_lcd_xOrigin -= width>>1;
//		}
//		else if ((anchor&0xf0) == 0x10 )
//		{
//			g_lcd_xOrigin -= width;
//		}
//	}
//	while(*str != '\0')
//	{
//		uint16_t key = *str;
//		lut->data = lu->data+((lu->width+7>>3)*lu->height*(key-lu->key));
//		lut->width = lu->width;
//		lut->height = lu->height;
//		lut->realWidth = lcd_getRealWidth(lut)+1;
//		
//		if(g_lcd_xOrigin<(UINT16_MAX>>1))
//		{
//			hw_setOrigin(g_lcd_xOrigin,g_lcd_yOrigin,lut->realWidth,lut->height);
//			hw_drawOneBitUint(lut);
//		}
//		g_lcd_xOrigin += lut->realWidth;
//		str ++;
//	}
//	free(lut);
//	return width;
//}

//void lcd_setPixelColor(uint16_t x, uint16_t y, uint16_t color, uint16_t alpha)
//{
//	hw_setOrigin(x,y,1,1);
//	if (alpha<0x100)
//	{
//		uint16_t oldColor = hw_rdata();
//		color = (color*(uint32_t)alpha + oldColor*(uint32_t)(0x100-alpha))>>8;
//	}
//	hw_wdata(color);
//}

void lcd_test(uint16_t data)
{
//	hw_setOrigin(0,0,100,50);
//	for(uint16_t i = 0 ; i < 5000;i++)
//	{
//		hw_wdata(0xf800);
//	}
}

/* draw a single color bmp *///2015年10月14日09:41:28
Rect_t pos_pm25 =
{
	.x = 33,
	.y = 102,
	.w = 24,
	.h = 45,
};
//static void lcd_drawREFline(void)
//{
//		Rect_t pos_line = 
//{
//	.x = 120+xOffset,
//	.y = 0+yOffset,
//	.w = 1,
//	.h = 240,
//};
//lcd_drawRectangle(&pos_line,0xffff);
//		Rect_t pos_line2 = 
//{
//	.x = 0+xOffset,
//	.y = 	120+yOffset,
//	.w = 240,
//	.h = 1,
//};
//lcd_drawRectangle(&pos_line2,0xffff);
//}
static void setLevelColor(uint8_t dataType,uint32_t sourceData,uint16_t *color,uint8_t *level)
{
	uint32_t data = sourceData;
	switch (dataType)
	{
		
		case GAS_PM10:	
			{
			//	data = sourceData/100;
			if (data<=500)
			{
				*level = 0;
				*color = rgb2c(0,228,0);
			}
			else if (data<=1500)
			{
				*level = 1;
				*color = rgb2c(255,255,0);
			}
			else if (data<=2500)
			{
				*level = 2;
				*color = rgb2c(255,64,0);
			}
			else if (data<=3500)
			{
				*level = 3;
				*color = rgb2c(255,0,0);
			}
			else if (data<=4200)
			{
				*level = 4;
				*color = rgb2c(153,0,76);
			}
			else
			{
				*level = 5;
				*color = rgb2c(126,0,35);
			}
		break;
		}		
//		case GAS_PM1:
//		{
//		//	data = sourceData/100;
//			if (data<=200)
//			{
//				*level = 0;
//				*color = rgb2c(0,228,0);
//			}
//			else if (data<=450)
//			{
//				*level = 1;
//				*color = rgb2c(255,255,0);
//			}
//			else if (data<=700)
//			{
//				*level = 2;
//				*color = rgb2c(255,64,0);
//			}
//			else if (data<=900)
//			{
//				*level = 3;
//				*color = rgb2c(255,0,0);
//			}
//			else if (data<=1500)
//			{
//				*level = 4;
//				*color = rgb2c(153,0,76);
//			}
//			else
//			{
//				*level = 5;
//				*color = rgb2c(126,0,35);
//			}
//		break;
//		}			
		case GAS_PM25:
		{
		//	data = sourceData/100;
			if (data<=3500)
			{
				*level = 0;
				*color = rgb2c(0,228,0);
			}
			else if (data<=7500)
			{
				*level = 1;
				*color = rgb2c(255,255,0);
			}
			else if (data<=11500)
			{
				*level = 2;
				*color = rgb2c(255,64,0);
			}
			else if (data<=15000)
			{
				*level = 3;
				*color = rgb2c(255,0,0);
			}
			else if (data<=25000)
			{
				*level = 4;
				*color = rgb2c(153,0,76);
			}
			else
			{
				*level = 5;
				*color = rgb2c(126,0,35);
			}
		break;
		}		
		
		case GAS_CO2:
		{
		//	data = sourceData/100;
			if (data<=1080)
			{
				*level = 0;
				*color = rgb2c(0,255,0);
			}
			else if (data<=1800)
			{
				*level = 1;
				*color = rgb2c(64,255,0);
			}
			else if (data<=2700)
			{
				*level = 2;
				*color = rgb2c(255,255,0);
			}	
			else 
			{
				*level = 3;
				*color = rgb2c(255,0,0);
			}			
		break;
		}
		
		case GAS_CH2O:
		{
			data = sourceData*10;
			if (data<= 30)
			{
				*level = 0;
				*color = rgb2c (0,255,0);		
			}
			else if (data <= 80)
			{
				*level = 0;
				*color = rgb2c (64,255,0);	
			}
			else if (data <= 300)
			{
				*level = 2;
				*color = rgb2c(255,255,0);
			}
			else if (data <= 500)
			{
				*level = 3;
				*color = rgb2c (255,126,0);
			}
			else if (data <= 750)
			{
				*level = 4;
				*color = rgb2c(255,0,0);			
			}
			else 
			{
				*level = 5;
				*color = rgb2c(153,0,76);			
			}
		break;	
		}
		case TEMPRATURE:
		{
			data = sourceData;
		//	{rgb2c(0,120,255),rgb2c(52,170,220),rgb2c(0,255,0),rgb2c(255,255,0),rgb2c(255,0,0)};
			//uint16_t colorBar[] = {rgb2c(225,0,186),rgb2c(71,144,255),rgb2c(0,228,0),rgb2c(255,64,0),rgb2c(255,0,0)};	
			if (data<= 500)
			{
				*level = 0;
				*color = rgb2c(225,0,186);		
			}
			else if (data <= 1000)
			{
				*level = 1;
				*color = rgb2c(71,144,255);
			}
			else if (data <= 1600)
			{
				*level = 2;
				*color = rgb2c(0,0,228);
			}
			else if (data <= 2400)
			{
				*level = 3;
				*color = rgb2c(0,228,0);
			}
			else if (data <= 3000)
			{
				*level = 4;
				*color = rgb2c(255,255,0);
			}
			else
			{
				*level = 5;
				*color = rgb2c(255,0,0);
			}
		
		break;
		}
		case HUMIDITY:
		{
			data = sourceData;
		//	{rgb2c(0,120,255),rgb2c(52,170,220),rgb2c(0,255,0),rgb2c(255,255,0),rgb2c(255,0,0)};
		//uint16_t colorBar[] = {rgb2c(255,0,0),rgb2c(153,0,76),rgb2c(0,228,0),rgb2c(71,144,255)};	
			if (data <= 4000)
			{
				*level = 0;
				*color = rgb2c(255,64,0);
			}
			else if (data <= 7000)
			{
				*level = 1;
				*color = rgb2c(0,228,0);
			}
			else
			{
				*level = 2;
				*color = rgb2c(71,144,255);					
			}		
		break;
		}
	}	
}
//static void lcd_showLine(Rect_t *pos)
//{
//	hw_setOrigin(pos->x + xOffset+pos->w,pos->y + yOffset + pos->h/2,30,1);
//	for (uint8_t i=0;i<10;i++)
//	{
//		hw_wdata(0xffff);
//	}
//		for (uint8_t i=0;i<10;i++)
//	{
//		hw_wdata(0);
//	}
//		for (uint8_t i=0;i<10;i++)
//	{
//		hw_wdata(0xffff);
//	}

//}
void lcd_drawSpecialBMP(const uint8_t * org,Rect_t * position,uint16_t Fcolor,uint16_t Bcolor,uint16_t percent)
{	
	int8_t scale = 0;
	uint16_t x = position->x, y = position->y,w = (position->w),h = (position->h) ;
	__IO uint16_t bitperline = ((((w -1)>>3)+1)<<3)>>scale;
//	__IO uint16_t x_bytes;
//	x_bytes = ceil(width*1.0/8);
//	bitperline = (ceil(w*1.0/8))*(8>>scale);
	if (scale>1)
	{
	bitperline = 32;
	}
	hw_setOrigin(x,y,w,h);
	for (__IO uint32_t i=0;i<(h);i++)
	{
		
	//	uint32_t row_pixel_count = 0;		
		for (__IO uint8_t j=0;j<(w);j++)
	{
	//	for (uint8_t k = 0;k<8;k++)
		{
//			__IO uint8_t mask = 0x80>>k;
			uint16_t  mapindex = (((i>>scale)*bitperline)+(j>>scale))>>3;
			__IO uint8_t mask = 0x80>>((j>>scale)&0x07);
			__IO uint16_t nchar = mapindex;
			nchar = *(org+nchar);
			if (nchar&mask)
			{
				hw_wdata(0);		
			}
			else
			{
				if (j<(w*percent/100))
				{
				hw_wdata(Fcolor);
				}
				else
				{
				hw_wdata(Bcolor);
				}
				
				
			}
//			row_pixel_count++;
//			if (row_pixel_count>=width)
//			{
//				break;
//			}
		}	
	}	
	}

}
void lcd_drawBMP(const uint8_t * org,Rect_t * position,uint16_t Fcolor,uint16_t Bcolor,int8_t scale)
{	
	uint16_t x = position->x, y = position->y,w = (position->w),h = (position->h) ;
	__IO uint16_t bitperline = ((((w -1)>>(3+scale))+1)<<(3+scale))>>scale;
//	__IO uint16_t x_bytes;
//	x_bytes = ceil(width*1.0/8);
	//bitperline = (ceil(w*1.0/(4<<(scale+1))))*(8);
//	if (scale>1)
//	{
//	bitperline = 32;
//	}
	hw_setOrigin(x,y,w,h);
	for (__IO uint32_t i=0;i<(h);i++)
	{
		
	//	uint32_t row_pixel_count = 0;		
		for (__IO uint8_t j=0;j<(w);j++)
		{
		//	for (uint8_t k = 0;k<8;k++)
			{
		//			__IO uint8_t mask = 0x80>>k;
				uint16_t  mapindex = (((i>>scale)*bitperline)+(j>>scale))>>3;
				__IO uint8_t mask = 0x80>>((j>>scale)&0x07);
				__IO uint16_t nchar = mapindex;
				nchar = *(org+nchar);
				if (nchar&mask)
				{
					hw_wdata(Fcolor);		
				}
				else
				{
					hw_wdata(Bcolor);					
				}
		//			row_pixel_count++;
		//			if (row_pixel_count>=width)
		//			{
		//				break;
		//			}
			}	
		}	
	}

}
void lcd_drawCompressBMP(const uint8_t * org,Rect_t * position,uint16_t Fcolor,uint16_t Bcolor,int8_t scale,uint8_t percent)
{	
	uint16_t x = position->x, y = position->y,w = (position->w),h = (position->h) ;
	__IO uint16_t bitperline = ((((w -1)>>3)+1)<<3)>>scale;
//	__IO uint16_t x_bytes;
//	x_bytes = ceil(width*1.0/8);
//	bitperline = (ceil(w*1.0/16))*(16>>scale);
//	if (scale>1)
//	{
//	bitperline = 32;
//	}
	hw_setOrigin(x,y,w,h);
	__IO uint32_t  readAddress = 0;
	__IO uint32_t totalCount = 0;
	for (__IO uint32_t i=0;i<(h);i++)
	{		
		uint16_t rowCount = 0;
	//	uint32_t row_pixel_count = 0;		
		for (__IO uint8_t j=0;j<(w);j++)
	{
	//	for (uint8_t k = 0;k<8;k++)
		{
//			__IO uint8_t mask = 0x80>>k;
			
			__IO uint8_t mask = 0x80>>((totalCount>>scale)&0x07);
			__IO uint16_t nchar = 0;
			static uint16_t ffCount = 0;
			nchar = *(org+readAddress);
			static uint32_t normalStart = 0;			
			
			if (nchar == 0xff)
			{
				if (ffCount == 0)
				{
					ffCount = *(org+readAddress+1)<<3;
				}
				
				//__IO uint16_t looptimes = ffCount;
				
				//for (uint16_t i=j;i<looptimes;i++)
					if (percent==0xff)
					{
						if (nchar&mask)
						{
							hw_wdata(Bcolor);		
						}
						else
						{
							hw_wdata(Fcolor);
							
						}
					}
					else
					{
						
						if (nchar&mask)
						{
							hw_wdata(0);		
						}
						else
						{
							hw_wdata(Fcolor);
							
						}
					}
//						else
//						{
//							if (rowCount<(w*percent/100))
//							{
//							hw_wdata(Bcolor);
//							}
//							else
//							{
//							hw_wdata(Fcolor);
//							}
//							
//							
//						}
//					
//					}
					ffCount --;
					totalCount++;
					rowCount++;
//					j++;
//					if (i>=w)
//					{
//					break;
//					}
				
				if (ffCount==0)
				{
					readAddress += 2;
					normalStart = 0;
				}	
			}
			else
			{
				
//				if (nchar&mask)
//				{
//					hw_wdata(Bcolor);		
//				}
//				else
//				{
//					hw_wdata(Fcolor);
//					
//				}
				if (percent==0xff)
					{
						if (nchar&mask)
						{
							hw_wdata(Bcolor);		
						}
						else
						{
							hw_wdata(Fcolor);
							
						}
					}
					else
					{
						
						if (nchar&mask)
						{
							hw_wdata(0);		
						}
						else
						{
							if (rowCount<(w*percent/100))
							{
							hw_wdata(Fcolor);
							}
							else
							{
							hw_wdata(Bcolor);
							}
							
							
						}
					
					}
				totalCount++;
				normalStart++;
				rowCount++;	
				if (normalStart >=8)
				{
				readAddress += normalStart>>3;
					normalStart = 0;
				}
				
			}

//			row_pixel_count++;
//			if (row_pixel_count>=width)
//			{
//				break;
//			}
		}	
	}	
	}

}

void swap(uint16_t* value1,uint16_t* value2)
{
	uint16_t temp;
	temp = *value1;
	*value1 = *value2;
	*value2 = temp;
	
}
void lcd_drawLine(Rect_t * startPoint, Rect_t * endPoint,uint8_t width,uint16_t color)
{
	uint16_t x0 = startPoint->x, y0 = startPoint->y , x1 = endPoint->x ,y1 = endPoint->y;
	uint8_t steep = (ABS(y1 - y0) > ABS(x1 - x0));
	if (steep)
	{
		swap(&x0, &y0);
		swap(&x1, &y1);
	}
	if (x0 > x1)
	{
		swap(&x0, &x1);
		swap(&y0, &y1);
	}
	int deltax = x1 -x0;
	int deltay = ABS(y1 - y0);
	int error = deltax / 2;
	int ystep = 0;
	int x;
	int y = y0;
	if (y0 > y1)
	{
		ystep = -1;
	}
	else
	{
		ystep = 1;
	}
	Rect_t point;
	point.h = width;
	point.w = width;
//	point.y = origin->y;

	for (x = x0 ;x < x1 ; (x)++) 
	{
		if (steep)
		{			
			point.x = y;
			point.y = x;		
		}
		else
		{
			point.x = x;
			point.y = y;
		}
		lcd_drawRectangle(&point,color);
		error -= deltay;
		if (error < 0)
		{
			y += ystep;
			error +=deltax;
		}		
	}
}
//show x.xx xx.x xxx number (input max 99999->999,1234->12.3,345->3.45),always have "width" digital display
void lcd_drawNumber(Rect_t * position,uint16_t color,uint16_t backColor,uint32_t number,uint8_t width,uint8_t type,uint8_t dot_position)
{
//	uint8_t firstzero = 1;
	uint8_t nbit[5]={0};
//	uint8_t dot_position=0xff;
//	uint8_t dotShowed = 0;
	const static uint16_t div_nums[] = {10000,1000,100,10,1};
  __IO int16_t currentx = 1;
	 Rect_t nclearArea;
	Rect_t dot_pos;
	 uint8_t i, numshownd = 0;
	 uint8_t validDigital = 0;
//	static uint8_t lastDigitals = 0;
	if (type==0)		
	{
		dot_pos.w = 16;	
//		position->w = 55;
//		position->h = 90;
	}
	else if (type==1)		
	{
		#if PM25_SINGLE_MODE
		dot_pos.w = 24;
#else
			dot_pos.w = 16;	
		#endif
	//	position->w = 24;
	//	position->h = 49;		
	}
	else if (type==2)		
	{
		dot_pos.w = 8;	
//		position->w = 22;
//		position->h = 43;		
	}	
	else if (type==3)		
	{
		dot_pos.w = 8;	
//		position->w = 10;
//		position->h = 19;		
	}	
	#if PM25_SINGLE_MODE
	else if (type==4)		
	{
		dot_pos.w = 16;	
//		position->w = 10;
//		position->h = 19;		
	}
	#endif
	Rect_t tops = {.x = (position->x)+xOffset, .y = (position->y)+yOffset, .w = position->w, .h = position->h};	
	for (i=0; i<sizeof(nbit)/sizeof(uint8_t); i++)
	{
		nbit[i] = (number/div_nums[i])%10;		
	}
	validDigital = 0;//储存位数
	uint32_t temp=1;
	do
	{
		++validDigital;
		temp *= 10;
	}while(temp<=number);
	if (validDigital>4)
	{
	//	dot_position = 0xff;
	}	
	else
	{
		//dot_position = 2;
	}
	if (validDigital>4)
	{
	currentx+= dot_pos.w>>1;
	}
		
	
	nclearArea.h = position->h;
	nclearArea.w = currentx;	
	nclearArea.x = tops.x+1;
	nclearArea.y = tops.y;
	if (type == 2)
	{
		lcd_drawRectangle(&nclearArea,backColor);
	}
	else
	{
		lcd_drawRectangle(&nclearArea,backColor);		
	}		

	for (i = MIN(5-width,width+dot_position-validDigital);i<5; i++)
	{
		tops.x = position->x + currentx + xOffset;
//		if (i >= dot_position)
//		{
//			firstzero = 0;
//		}
//		if (!firstzero||(nbit[i])!=0)
		if (type==0)
		{
		lcd_drawBMP(gImage_hugeNumber[nbit[i]], &tops,color,backColor,0);
		}
		else if (type==2)
		{
		lcd_drawBMP(gImage_smallNumber[nbit[i]], &tops,color, backColor,0);
		}
		else if (type==1)
		{
		lcd_drawBMP(gImage_bigNumber[nbit[i]], &tops,color,backColor,0);
		}
		else if (type==3)
		{
		lcd_drawBMP(gImage_littleNumber[nbit[i]], &tops,color,backColor,0);
		}
	#if PM25_SINGLE_MODE
		else if (type==4)
		{
		lcd_drawBMP(gImage_middleNumber[nbit[i]], &tops,color,backColor,0);
		}
	#endif		
//		else if (type==2)
//		{
//		lcd_drawBMP(gImage_midNumber[nbit[i]], &tops,color,0x0000,0);
//		}
//		else if (type==3)
//		{
//		lcd_drawBMP(gImage_smallNumber[nbit[i]], &tops,color,0x0000,0);
//		}		
			numshownd ++;
//			firstzero = 0;
			currentx += tops.w;
		
		if (numshownd == width)
		{
			break;
		}
		if (i == dot_position )
		{				
			dot_pos.h = tops.h;
			dot_pos.y=tops.y;
			dot_pos.x = tops.x + tops.w;	
			if (type==0)		
			{					
				lcd_drawBMP(gImage_hugeDot,&dot_pos,color,backColor,0);
			}
			else if (type==1)		
			{				
				lcd_drawBMP(gImage_bigdot,&dot_pos,color,backColor,0);
			}
			else if (type==2)		
			{			
				lcd_drawBMP(gImage_smallDot,&dot_pos,color, backColor,0);
			}
			else if (type==3)		
			{			
				lcd_drawBMP(gImage_littleDot,&dot_pos,color,backColor,0);
			}
	#if PM25_SINGLE_MODE
			else if (type==4)
			{
				lcd_drawBMP(gImage_middledot,&dot_pos,color,backColor,0);
			}				
	#endif
			currentx += dot_pos.w;
			
//			dotShowed = 1;
		}
	}
	if (validDigital>4)
	{
		nclearArea.w = dot_pos.w>>1;

		nclearArea.x = position->x + currentx+xOffset;

		if (type == 2)
		{
			lcd_drawRectangle(&nclearArea,backColor);
		}
		else
		{
			lcd_drawRectangle(&nclearArea,backColor);		
		}
	}
//	lastDigitals = validDigital;	

}


Rect_t pos_temp =
{
	.x = 72,
	.y = 59,
	.w = 10,
	.h = 19,
};
Rect_t pos_hum =
{
	.x = 155,
	.y = 59,
	.w = 10,
	.h = 19,
};
//static void lcd_showTemprature(uint32_t temp,uint16_t color)
//{
//	lcd_drawRHNumber( &pos_temp,color,temp,1,2,1);	
//}
//static void lcd_showHumidity(uint32_t hum,uint16_t color)
//{
//	lcd_drawRHNumber( &pos_hum,color,hum,1,2,1);	
//}

static void lcd_showEngGrade(uint8_t grade,uint16_t startX, uint16_t startY,uint16_t backColor,uint8_t constColor)
{
	uint16_t color[] = {rgb2c(0,228,0),rgb2c(255,255,0),rgb2c(255,126,0),rgb2c(255,0,0),rgb2c(153,0,76),rgb2c(126,0,35)};
}
static void lcd_showGrade(uint8_t grade,uint16_t startX, uint16_t startY,uint16_t backColor,uint8_t constColor)
{
	uint16_t color[] = {rgb2c(0,228,0),rgb2c(255,255,0),rgb2c(255,126,0),rgb2c(255,0,0),rgb2c(153,0,76),rgb2c(126,0,35)};
	
	
	Rect_t pos_exllent = {.x = 167+xOffset,.y = 115+yOffset,.w = 15,.h = 14,};	
//	pos_exllent.x = startX-8;
	pos_exllent.y = startY+1;	

	Rect_t pos_clear 	=  {.x = 145+xOffset,.y = 115+yOffset,.w = 112,.h = 14,};
	pos_clear.x = startX - 60;
	pos_clear.y = startY;	
	
	if (constColor)
	{
		for (uint8_t i = 0 ; i < sizeof(color)/sizeof(uint16_t) ; i++)
		{
			color[i] = 0xffff;		
			
		}
	}
	else
	{
		backColor = g_lcd_backColor;
	}
//	if (needRedraw)
	{
	lcd_drawRectangle(&pos_clear,backColor);
	}
	switch (grade)
	{
		case 0:
			pos_exllent.x = startX - 8;
//			pos_exllent.y = startY;	
			pos_exllent.w = 32;
			pos_exllent.h = 13;
			lcd_drawBMP(gImage_enGrade0, &pos_exllent,color[grade],backColor,0);		
		break;
		case 1:	
			pos_exllent.x = startX - 24;
//			pos_exllent.y = startY;	
			pos_exllent.w = 64;
			pos_exllent.h = 13;		
			lcd_drawBMP(gImage_enGrade1, &pos_exllent,color[grade],backColor,0);	
			break;
		
		case 2:
		case 3:
			pos_exllent.x = startX - 32;
	//		pos_exllent.y = startY;	
			pos_exllent.w = 72;
			pos_exllent.h = 13;
			lcd_drawBMP(gImage_enGrade2, &pos_exllent,color[grade],backColor,0);	
			break;		
		case 4:			
			pos_exllent.x = startX - 60;
		//	pos_exllent.y = startY;	
			pos_exllent.w = 112;
			pos_exllent.h = 13;		
		  lcd_drawBMP(gImage_enGrade3, &pos_exllent,color[grade],backColor,0);	
			break;
		case 5:
			pos_exllent.x = startX - 32;
	//		pos_exllent.y = startY;	
			pos_exllent.w = 72;
			pos_exllent.h = 13;	
			lcd_drawBMP(gImage_enGrade4, &pos_exllent,color[grade],backColor,0);	
			break;
		default:
			break;
	}

	
}

void lcd_showAQIgrade(uint8_t grade,uint8_t needRedraw,uint16_t color,uint16_t mode)
{
//	const uint16_t color[] = {rgb2c(0,228,0),rgb2c(255,255,0),rgb2c(255,64,0),rgb2c(255,0,0),rgb2c(153,0,76),rgb2c(126,0,35)};
	
	Rect_t pos_grade = {.x =60,.y = 280,.w = 24,.h = 24,};	
	Rect_t pos_backRetangle =  {.x = 0,.y = 270,.w = 240,.h = 43,};
//	Rect_t pos_grade = {.x =70+xOffset,.y = 280+yOffset,.w = 24,.h = 24,};	
//	Rect_t pos_grade ={.x = 90+xOffset,.y = 205+yOffset,.w = 15,.h = 14,};
//	Rect_t pos_twoWord = {.x = 102+xOffset,.y = 205+yOffset,.w = 15,.h = 14,};
//	Rect_t pos_clear 	=  {.x = 86+xOffset,.y = 202+yOffset,.w = 155-86,.h = 20,};
//	Rect_t pos_backRetangle =  {.x = 0+xOffset,.y = 270+yOffset,.w = 240,.h = 50,};
//	Rect_t pos_roundR =  {.x =155+xOffset,.y = 202+yOffset,.w = 7,.h = 20,};
	if (needRedraw)
	{
	//		lcd_drawRectangle(&pos_backRetangle,color);
//		lcd_drawR
//		lcd_drawBMP(gImage_roundLeft, &pos_roundL,0,color,0);	
//		lcd_drawBMP(gImage_roundRight, &pos_roundR,0,color,0);	
	lcd_drawRectangle(&pos_backRetangle,color);
	}
	switch (grade)
	{
		case 0:
			pos_grade.x += 30;
			pos_grade.w = 56;
			pos_grade.h = 18;
			lcd_drawBMP(gImage_good, &pos_grade,0xffff,color,0);	
			break;
		case 1:	
			pos_grade.w = 112;
			pos_grade.h = 18;
			lcd_drawBMP(gImage_moderate, &pos_grade,0xffff,color,0);	
			break;
		case 2:
		case 3:
			pos_grade.w = 126;
			pos_grade.h = 18;
			lcd_drawBMP(gImage_unhealthy, &pos_grade,0xffff,color,0);	
			break;
		case 4:			
		case 5:
			pos_grade.w = 126;
			pos_grade.h = 18;
			lcd_drawBMP(gImage_hazardous, &pos_grade,0xffff,color,0);	
			break;
		default:
			break;
	}
	

//		{
//			Rect_t pos_good = {.x = 167+xOffset,.y = 115+yOffset,.w = 15,.h = 14,};
//			const uint8_t* index[] = {gImage_grade[0]};
//			lcd_drawBMP(index[0], &pos_exllent,0,rgb2c(0,255,0),0);			
//		}
	
}
#define j2h(x) (3.1415926*(x)/180.0)

void lcd_drawCircleLine(uint32_t angle,Rect_t* center,uint16_t raidus,uint16_t length,uint16_t color)
{
	Rect_t startPoint,endPoint;
//	uint32_t angle;
	//angle is form 360 - 45  to 180 +45

//	angle = pm25data*270/99999/10*10;
//	if (angle > 180)
//	{
//		startPoint.x = (center->x) + (raidus * cos(j2h(angle)));
//		startPoint.y = (center->y) + (raidus * sin(j2h(angle)));
//		endPoint.x = (center->x) + ((raidus - length)* cos(j2h(angle)));
//		endPoint.y = (center->y) + ((raidus - length)* sin(j2h(angle)));	
//	}
//	else
	{
		startPoint.x = (center->x) - (raidus * cos(j2h(angle)));
		startPoint.y = (center->y) - (raidus * sin(j2h(angle)));
		endPoint.x = (center->x) - ((raidus - length)* cos(j2h(angle)));
		endPoint.y = (center->y) - ((raidus - length)* sin(j2h(angle)));	
	}
	lcd_drawLine(&startPoint,&endPoint,2,color);
}
void lcd_showBattery(uint8_t level,uint8_t is_charging,uint8_t isPoweroff)
{
	const uint8_t *batterys[] = {gImage_battery0,gImage_battery1,gImage_battery2,gImage_battery3,gImage_battery4};
	Rect_t pos_battery = 
	{
		.x = 201+xOffset,
		.y = 19+yOffset,
		.w = 22,
		.h = 9,
	};
	Rect_t pos_largeBattery =
	{
		.x = 80+xOffset,
		.y = 140+yOffset,
		.w = 88,
		.h = 36,
	};
	static uint8_t showlevel = 100;
	if (is_charging == 1)
	{
	//	uint8_t currentlevel = level;
		showlevel +=1;
		if (showlevel>4)
		{
			showlevel = level/25;
		}
		if (isPoweroff)
		{
//			const uint8_t *lbatterys[] = {gImage_lbattery0,gImage_lbattery1,gImage_lbattery2,gImage_lbattery3,gImage_lbattery4};
			uint16_t color = rgb2c(200,200,200);
			Rect_t pos_low =
			{
				.x = 82,
				.y = 185,
				.w = 88,
				.h = 13,
			};

	//		lcd_drawCompressBMP(gImage_logo_compressed, &pos_logo,color,rgb2c(32,32,32),0,showlevel);
				
//			lcd_drawBMP(lbatterys[showlevel],&pos_logo,0,0xffff,1);
			if (level<12)
			{
				lcd_drawBMP(gImage_batteryLow,&pos_low,0xffff,0,0);
			}
			else
			{
				lcd_drawRectangle(&pos_low,0);
			}
			lcd_drawBMP(batterys[showlevel],&pos_largeBattery,g_lcd_backColor,0xffff,2);
		}	
		else
		{	
			lcd_drawBMP(batterys[showlevel],&pos_battery,g_lcd_backColor,0xffff,0);
		//lcd_drawSpecialBMP(gImage_chargingLogo,&pos_battery,rgb2c(0,64,255),0xffff,showlevel);
		}
		
		
	}
	else if (is_charging == 0)
	{
		showlevel = level/25;
		lcd_drawBMP(batterys[showlevel],&pos_battery,g_lcd_backColor,0xffff,0);

	}
	else
	{
	//	Rect_t pos_battery_notify =
	//	{
	//		.x = 192+xOffset,
	//		.y = 12+yOffset,
	//		.w = 30,
	//		.h = 16,
	//	};

		{
		lcd_drawRectangle(&pos_battery,g_lcd_backColor);

		lcd_drawBMP(gImage_charge,&pos_battery,g_lcd_backColor,0xffff,0);
		}
	}

}
//void lcd_showWifiIco(uint8_t status)
//{
//		Rect_t pos_wifi = 
//{
//	.x = 104+xOffset,
//	.y = 23+yOffset,
//	.w = 11,
//	.h = 11,
//};
//if (status == 3)
//{
//lcd_drawBMP(gImage_wifi,&pos_wifi,0,0xffff,0);

//}
//else
//{
//lcd_drawRectangle(&pos_wifi,0);
//}

//}
void lcd_showBackGround(void)
{



}


void lcd_showQR(char *data,uint8_t needRefresh)
{
	static uint8_t generated=0;
	#if 0
	Rect_t QR_tag =
	{
		.x = 64+xOffset,
		.y = 80+yOffset,
		.w = 116,
		.h = 116,
	};

	Rect_t wifi_outline =
	{
		.x = QR_tag.x - 4,
		.y = QR_tag.y - 4,
		.w = QR_tag.w + 8,
		.h = QR_tag.h + 8,
	};
	#else
//	Rect_t QR_tag =
//	{
//		.x = 4,
//		.y = 4,
//		.w = 74,
//		.h = 74,
//	};
//	Rect_t wifi_outline =
//	{
//		.x = 0,
//		.y = 0,
//		.w = 90,
//		.h = 90,
//	};
	Rect_t QR_tag =
	{
		.x = 46,
		.y = 64,
		.w = 74*2,
		.h = 74*2,
	};
	Rect_t wifi_outline =
	{
		.x = QR_tag.x - 4,
		.y = QR_tag.y - 4,
		.w = QR_tag.w + 8,
		.h = QR_tag.h + 8,
	};
	#endif
	char testString[108];
//		char testString[55];
//	char testString[68];
	static uint8_t *QRbuffer;
	if (!generated )
	{
		#if MAC2UID
		sprintf(testString,"http://app.hw99lt.com/app/login?deviceId=%s",data);
		#else
		sprintf(testString,data);
		#endif
//		sprintf(testString,"111");
		QRbuffer = QR_stringToBuffer(testString);
//		for (uint8_t i = 0; i<246; i++)
		for (uint8_t i = 0; i<185; i++)
//		for (uint8_t i = 0; i<116; i++)
		{
			*(QRbuffer+i) = ~*(QRbuffer+i);
		}	
	
	}
	if (needRefresh || !generated)
	{
	lcd_drawRectangle(&wifi_outline,0xffff);
	lcd_drawBMP(QRbuffer, &QR_tag,0xffff,g_lcd_backColor,2);	
	}
//	lcd_drawGradualBMP(QRbuffer, &QR_tag,rgb2c(0,0,255),0,2);
	generated = 1;		
//	Rect_t pos_test =
//	{
//		.x = 110+xOffset,
//		.y = 85+yOffset,
//		.w = 30,
//		.h = 30,
//	};
//		lcd_drawRectangle(&pos_test,0xf000);
//	lcd_drawBMP(gImage_battery0,&pos_battery_notify,0,0xffff,0);
	//	testString[16]=0;
	// 	if (two==0)
	// 	{
	//}
	//if (two==1)
	//{
	//two=0;
	//}
	//  eeprom_write_byte(HISTORY_EEPROM_WRITE_INDEX_ADDRESS,QRbuffer);	
}
void lcd_showTest(uint8_t number,uint16_t color)
{
	if (number>99)
	{
		number = 99;
	}
	Rect_t pos_testnumber =
	{
	.x = 10+xOffset,
	.y = 160,
	.w = 6,
	.h = 9
	};
	lcd_drawBMP(img_wifi_num[number%10],&pos_testnumber,0XFFFF,0,0);
	pos_testnumber.x += 10;
	lcd_drawBMP(img_wifi_num[number/10],&pos_testnumber,0XFFFF,0,0);
}
static uint16_t lcd_drawMutiWords(Rect_t *firstWord,const uint8_t ** org,uint8_t size)
{
	
	for (uint8_t i = 0; i < size ; i++)
	{
		lcd_drawBMP(*(org+i),firstWord,0XFFFF,0,0);
		firstWord->x += firstWord->w;
	
	}
	return firstWord->x;

}
void lcd_showInfoHeader(uint8_t wifistatus,uint16_t data_battery,uint8_t is_charging,RTC_TimeTypeDef* time, RTC_DateTypeDef* date)
{

		Rect_t pos_line = 
	{
		.x = 0,
		.y = 38,
		.w = 240,
		.h = 1,
	};
		Rect_t pos_titleclear = 
	{
		.x = 0,
		.y = 0,
		.w = 175,
		.h = 37,
	};

	Rect_t pos_title = 
	{
		.x = 12,
		.y = 10,
		.w = 106,
		.h = 22,
	};

			Rect_t pos_wifiIco = 
	{
		.x = 168,
		.y = 18,
		.w = 15,
		.h = 11,
	};
	lcd_showBattery(data_battery,is_charging,0);
	static uint8_t lastWifiStatus = 0;
	lcd_drawRectangle(&pos_line,0xffff);
	if (lastWifiStatus!=wifistatus)
	{
		lcd_drawRectangle(&pos_wifiIco,g_lcd_backColor);
//		lcd_drawRectangle(&pos_clear,g_lcd_backColor);
		lcd_drawRectangle(&pos_titleclear,g_lcd_backColor);
	}
	lastWifiStatus = wifistatus;
	if (wifistatus == 6)
	{
		lcd_drawBMP(gImage_wifiico,&pos_wifiIco,g_lcd_backColor,0xffff,0);
//		lcd_showDateTime(time,date);
	}
	else
	{
		#if NO_LOGO_UI
		lcd_drawRectangle(&pos_titleclear,g_lcd_backColor);
		#else
		lcd_drawBMP(gImage_bluesky,&pos_title,g_lcd_backColor,rgb2c(0,64,255),0);	
		#endif
	}
}	
void lcd_showWifiStatus(uint8_t wifistatus,uint8_t statusChanged,uint32_t smartingTick)
{
		Rect_t pos_clear =
	{
	.x = 10,
	.y = 270,
	.w = 200,
	.h = 49
	};
	Rect_t pos_singleWord =
	{
	.x = 84+xOffset,
	.y = 278+yOffset,
	.w = 13,
	.h = 13
	};
		Rect_t pos_leftnumber =
	{
	.x = 112+xOffset,
	.y = pos_singleWord.y+18+yOffset,
	.w = 6,
	.h = 8
	};
	Rect_t pos_rightnumber =
	{
	.x = 120+xOffset,
	.y = pos_leftnumber.y,
	.w = 6,
	.h = 8
	};


	Rect_t pos_line = 
	{
		.x = 0,
		.y = 269,
		.w = 240,
		.h = 1,
	};
	
//		Rect_t pos_center_title = 
//	{
//		.x = 70,
//		.y = 115,
//		.w = 106,
//		.h = 22,
//	};

//	lcd_showQR(NULL,statusChanged);	
	
	//lcd_drawBMP(gImage_wifititle,&pos_wifiTitle,0,0xffff,0);

	lcd_drawRectangle(&pos_line,0xffff);
	
	static uint8_t lastWifiStatus = 0;

	if (lastWifiStatus!=wifistatus)
	{
//		lcd_drawRectangle(&pos_wifiIco,g_lcd_backColor);
		lcd_drawRectangle(&pos_clear,g_lcd_backColor);
	//	lcd_drawRectangle(&pos_titleclear,g_lcd_backColor);
	}
	lastWifiStatus = wifistatus;
//	lcd_drawBMP(gImage_dotline,&pos_dotline,0,0xffff,0);
//	pos_dotline.x = 192;
//	lcd_drawBMP(gImage_dotline2,&pos_dotline,0,0xffff,0);

//	lcd_drawBMP(gImage_bluesky,&pos_title,0,rgb2c(0,64,255),0);

//	if (statusChanged)
//	{
//	lcd_drawBMP(gImage_bluesky,&pos_center_title,0,rgb2c(0,64,255),0);
//	}
	switch (wifistatus)
	{
		case 1: 
		{
			uint8_t s = smartingTick/1000/10;
			uint8_t g = smartingTick/1000%10;
			pos_singleWord.x = 64;
			pos_singleWord.w = 112;
			lcd_drawBMP(gImage_smart,&pos_singleWord,0XFFFF,0,0);

			lcd_drawBMP(gImage_littlenumber[s],&pos_leftnumber,0XFFFF,0,0);
			lcd_drawBMP(gImage_littlenumber[g],&pos_rightnumber,0XFFFF,0,0);
		break;
		}
		
//			lcd_drawBMP(gImage_smarting,&pos_connecting,0,rgb2c(0,0,255),0);
//		
//			lcd_drawBMP(img_phone,&pos_left_phone,rgb2c(0,0,255),0,0);
////			lcd_drawBMP(img_connectTips,&pos_connect_tip1,rgb2c(0,0,255),0,0);
////			lcd_drawBMP(img_connectTips2,&pos_connect_tip2,rgb2c(0,0,255),0,0);
//		break;	
		
		case 4: 
			break;
		
		
		//	lcd_drawBMP(img_phone,&pos_left_phone,rgb2c(0,0,255),0,0);
//			lcd_drawBMP(gImage_cloudDown,&pos_cloudDown,0XFFFF,0,0);
		
		case 5:
		{

			pos_singleWord.y += 8;
			pos_singleWord.x = 60;
			pos_singleWord.w = 120;
			lcd_drawBMP(gImage_noWifi,&pos_singleWord,0XFFFF,0,0);

			
			break;
		}
		case 2:	
		case 3:
		{
			pos_singleWord.y += 8;
			pos_singleWord.x = 48;
			pos_singleWord.w = 136;
			lcd_drawBMP(gImage_connect,&pos_singleWord,0XFFFF,0,0);
			break;			
		}
		case 6:
		{
			pos_singleWord.y += 8;
			pos_singleWord.x = 54;
			pos_singleWord.w = 128;
			lcd_drawBMP(gImage_connected,&pos_singleWord,0XFFFF,0,0);
//			lcd_drawBMP(gImage_wifiico,&pos_wifiIco,g_lcd_backColor,0xffff,0);
			break;
		}
		
		case 0:
		//	lcd_drawBMP(img_phone,&pos_left_phone,rgb2c(0,0,255),0,0);
//			lcd_drawBMP(gImage_isSmarting,&pos_smarting,0XFFFF,0,0);
		default:

			break;
	}
//	#if MAC2UID
			pos_singleWord.x = 32;
			pos_singleWord.y = 235;
				pos_singleWord.w = 176;
		lcd_drawBMP(gImage_scan,&pos_singleWord,0XFFFF,0,0);
//			const uint8_t* imageList[] = {gImage_wifiword[15],gImage_wifiword[16],gImage_wifiword[17],gImage_wifiword[18],gImage_wifiword[19],gImage_wifiword[20],gImage_wifiword[21],gImage_wifiword[22]};
//			lcd_drawMutiWords(&pos_singleWord,imageList,8);
//	#else
//			pos_singleWord.x = 64+xOffset;
//			pos_singleWord.y = 235+yOffset;
//			const uint8_t* imageList[] = {gImage_wifiword[15],gImage_wifiword[16],gImage_wifiword[17],gImage_wifiword[18],gImage_wifiword[19],gImage_wifiword[20],gImage_wifiword[21],gImage_wifiword[22],gImage_wifiword[23]};
//			lcd_drawMutiWords(&pos_singleWord,imageList,9);
//	#endif
	
	


}
static void lcd_showVersion(void)
{
		Rect_t pos_leftnumber =
	{
	.x = 180,
	.y = 296,
	.w = 6,
	.h = 9
	};
	uint32_t fcolor = rgb2c(64,64,64);
	lcd_drawBMP(gImage_version_char[0],&pos_leftnumber,fcolor,0,0);
	pos_leftnumber.x += 7;
#if HCHO_ENABLE
	{
		lcd_drawBMP(gImage_version_char[1],&pos_leftnumber,fcolor,0,0);	
		pos_leftnumber.x += 7;
	}
#endif
#if CO2_ENABLE
	{
		lcd_drawBMP(gImage_version_char[2],&pos_leftnumber,fcolor,0,0);	
		pos_leftnumber.x += 7;
	}
#endif	
	
	for (uint8_t i=0;i<strlen(FIRMWARE_VERSION);i++)
	{
		//lcd_drawBMP(gImage_wifi,&pos_company,0,0XFFFF,0);
		uint8_t number = FIRMWARE_VERSION[i]- '0';
		if (FIRMWARE_VERSION[i] == '.')
		{
				Rect_t pos_dot=
			{
				.x = pos_leftnumber.x ,
				.y = pos_leftnumber.y + pos_leftnumber.h -1,
				.w = 1,
				.h = 1,
			};
			lcd_drawRectangle(&pos_dot,fcolor);
			pos_leftnumber.x += 4;
			continue;
			
		}
		lcd_drawBMP(img_wifi_num[number],&pos_leftnumber,fcolor,0,0);
		pos_leftnumber.x += 7;
		//lcd_drawBMP(img_wifi_num[g],&pos_rightnumber,0XFFFF,0,0);
	
	}
	if (M1_MODE)
	{
		lcd_drawBMP(gImage_version_char[3],&pos_leftnumber,fcolor,0,0);	
	}

}
void lcd_showlogo(uint16_t color,uint8_t mode)
{
	if (mode == 0)
	{
		Rect_t pos_logo =
		{
			.x = 22+xOffset,
			.y = 76+yOffset,
			.w = 200,
			.h = 44,
		};

			Rect_t pos_company =
		{
			.x = 22+xOffset,
			.y = 272+yOffset,
			.w = 198,
			.h = 10,
		};
		
		Rect_t pos_TYPE =
		{
			.x = 60+xOffset,
			.y = 152+yOffset,
			.w = 125,
			.h = 23,
		};		
		
		{
			//	lcd_drawBMP(gImage_companyWord,&pos_company,0XFFFF,g_lcd_backColor,0);
		}
		lcd_showVersion();
		
		#if M1_MODE
		lcd_drawBMP(gImage_D1,&pos_TYPE,g_lcd_backColor,rgb2c(80,255,255),0);
		#else
		lcd_drawBMP(gImage_D1,&pos_TYPE,g_lcd_backColor,rgb2c(255,255,255),0);
		#endif
		#if !NO_LOGO_UI
		//lcd_drawBMP(gImage_logo, &pos_logo,0,color,0);

		lcd_drawCompressBMP(gImage_logo_compressed, &pos_logo,color,g_lcd_backColor,0,0xff);
		#endif
//		#if NO_LOGO_UI
//		Rect_t pos_TYPE =
//		{
//			.x = 60+xOffset,
//			.y = 152+yOffset,
//			.w = 125,
//			.h = 23,
//		};
//		lcd_drawBMP(gImage_D1,&pos_TYPE,g_lcd_backColor,rgb2c(255,255,255),0);
//		#endif
	}
	else
	{
			Rect_t pos_largeBattery =
		{
			.x = 80,
			.y = 140,
			.w = 88,
			.h = 36,
		};

		lcd_drawBMP(gImage_battery0,&pos_largeBattery,g_lcd_backColor,0xffff,2);
		
//		lcd_drawBMP(gImage_lbatteryCharge,&pos_battery_notify,0,0xffff,0);
	}


}
#if 0
void lcd_drawRHNumber(Rect_t * position,uint16_t color,uint16_t Bcolor,uint32_t number,uint8_t dot_position,uint8_t width,uint8_t isSmall)
{
//	uint8_t firstzero = 1;
	uint8_t nbit[5]={0};
//	uint8_t dotShowed = 0;
	const static uint16_t div_nums[] = {10000,1000,100,10,1};
	//const uint8_t* pImage[135]={gImage_midNumber,gImage_smallNumber};
	//const uint8_t* dotImage[]={gImage_dot,gImage_smallDot};
  __IO uint16_t currentx = 1;
	 Rect_t nclearArea;
	 uint8_t i, numshownd = 0;
	 __IO uint8_t validDigital = 0;
	
//	static uint8_t lastDigitals = 0;
	Rect_t tops = {.x = (position->x)+xOffset, .y = (position->y)+yOffset, .w = position->w, .h = position->h};
	
	for (i=0; i<sizeof(nbit)/sizeof(uint8_t); i++)
	{
		nbit[i] = (number/div_nums[i])%10;		
	}
//	width = fmin(width, sizeof(nbit)/sizeof(uint8_t));
	validDigital = 0;//储存位数
	uint32_t temp=1;
	do
	{
		++validDigital;
		temp *= 10;
	}while(temp<=number);
	
	if (dot_position<(width-1))
	{
		validDigital = MAX(2,validDigital);
	}
	else if (validDigital ==0)
	{
		validDigital = 1;
	}
	if (dot_position == 0)
	{
		validDigital = width;
	}
		
	if (validDigital<width)
	{
		currentx = (tops.w * width  - (validDigital)*tops.w)>>1;
	}
	
//	if (lastDigitals!=validDigital)
	if (isSmall)
	{
		nclearArea.h = 12;	
	}	
	else
	{
		nclearArea.h = 49;
	}
	nclearArea.w = currentx-1;
	
	nclearArea.x = tops.x+1;
	nclearArea.y = tops.y;
	lcd_drawRectangle(&nclearArea,Bcolor);		
		
	
	
	for (i = 5 - validDigital;i<5; i++)
	{
		tops.x = position->x + currentx + xOffset;
//		if (i >= dot_position)
//		{
//			firstzero = 0;
//		}
//		if (!firstzero||(nbit[i])!=0)
		{
			//		oled_getdataFromPgm(tmp,notes[nbit[i]]);
				if (isSmall)
			{
				lcd_drawBMP(gImage_titleNumber[nbit[i]], &tops,color,Bcolor,0);
			}
				else
			{
			//	lcd_drawBMP(gImage_midNumber[nbit[i]], &tops,color,Bcolor,0);
			}
			
			numshownd ++;
//			firstzero = 0;
			currentx += tops.w;
		}
		if (numshownd == validDigital)
		{
			break;
		}
		if (i == dot_position &&  dot_position != (width-1))
		{
			Rect_t dot_pos;// = {.w = 4,.h = 4,.y=position->y+24};
							
							
			if (isSmall)
			{
				dot_pos.h = 19;
				dot_pos.w = 8;	
				dot_pos.y=tops.y;
				dot_pos.x = tops.x + tops.w;	
				lcd_drawBMP(gImage_smallDot,&dot_pos,color,Bcolor,0);
			}
			else
			{
//				dot_pos.w = 12;	
//				dot_pos.h = 49;
//				dot_pos.y=tops.y;
//				dot_pos.x = tops.x + tops.w;	
//				lcd_drawBMP(gImage_bigdot,&dot_pos,Bcolor,color,0);
			}

			currentx += dot_pos.w;
			
//			dotShowed = 1;
		}
	}
//	if (lastDigitals!=validDigital)
	{
		nclearArea.w = tops.w * (width - numshownd) - ((tops.w * width  - (validDigital)*tops.w)>>1)+1;
//		nclearArea.h = 45;
		nclearArea.x = position->x + currentx+xOffset;
		//nclearArea.y = position->y+yOffset;
		lcd_drawRectangle(&nclearArea,Bcolor);
	}
//	lastDigitals = validDigital;	
}


#endif
void lcd_drawRHNumber(Rect_t * position,uint16_t color,uint16_t Bcolor,uint32_t number,uint8_t dot_position,uint8_t width,uint8_t isSmall,uint8_t leftJust)
{
//	uint8_t firstzero = 1;
	uint8_t nbit[5]={0};
//	uint8_t dotShowed = 0;
	const static uint16_t div_nums[] = {10000,1000,100,10,1};
	//const uint8_t* pImage[135]={gImage_midNumber,gImage_smallNumber};
	//const uint8_t* dotImage[]={gImage_dot,gImage_smallDot};
  __IO uint16_t currentx = 1;
	 Rect_t nclearArea;
	 uint8_t i, numshownd = 0;
	 __IO uint8_t validDigital = 0;
	
//	static uint8_t lastDigitals = 0;
	Rect_t tops = {.x = (position->x)+xOffset, .y = (position->y)+yOffset, .w = position->w, .h = position->h};
	
	for (i=0; i<sizeof(nbit)/sizeof(uint8_t); i++)
	{
		nbit[i] = (number/div_nums[i])%10;		
	}
//	width = fmin(width, sizeof(nbit)/sizeof(uint8_t));
	validDigital = 0;//储存位数
	uint32_t temp=1;
	do
	{
		++validDigital;
		temp *= 10;
	}while(temp<=number);
	if (dot_position<(width-1))
	{
	validDigital = MAX(2,validDigital);
	}
	else if (validDigital ==0)
	{
	validDigital = 1;
	}
	if (dot_position == 0)
	{
	validDigital = width;
	}
	if (leftJust)
	{
		validDigital = width;
	}	
	if (validDigital<width)
	{
		currentx = (tops.w * width  - (validDigital)*tops.w)>>1;
	}
	
//	if (lastDigitals!=validDigital)
	if (isSmall)
	{
	nclearArea.h = 19;	
	}	
	else
	{
	nclearArea.h = 12;
	}
	nclearArea.w = currentx-1;
	
	nclearArea.x = tops.x+1;
	nclearArea.y = tops.y;
	lcd_drawRectangle(&nclearArea,Bcolor);		
		
	
	
	for (i = 5 - validDigital;i<5; i++)
	{
		tops.x = position->x + currentx + xOffset;
//		if (i >= dot_position)
//		{
//			firstzero = 0;
//		}
//		if (!firstzero||(nbit[i])!=0)
		{
			//		oled_getdataFromPgm(tmp,notes[nbit[i]]);
				if (isSmall)
			{
				lcd_drawBMP(gImage_smallNumber[nbit[i]], &tops,color,Bcolor,0);
			}
				else
			{
				lcd_drawBMP(gImage_titleNumber[nbit[i]], &tops,color,Bcolor,0);
			}
			
			numshownd ++;
//			firstzero = 0;
			currentx += tops.w;
		}
		if (numshownd == validDigital)
		{
			break;
		}
		if (i == dot_position &&  dot_position != (width-1))
		{
			Rect_t dot_pos;// = {.w = 4,.h = 4,.y=position->y+24};
							
							
			if (isSmall)
			{
				dot_pos.h = 19;
				dot_pos.w = 8;	
				dot_pos.y=tops.y;
				dot_pos.x = tops.x + tops.w;	
				lcd_drawBMP(gImage_smallDot,&dot_pos,color,Bcolor,0);
			}
//			else
//			{
//				dot_pos.w = 12;	
//				dot_pos.h = 49;
//				dot_pos.y=tops.y;
//				dot_pos.x = tops.x + tops.w;	
//				lcd_drawBMP(gImage_bigdot,&dot_pos,Bcolor,color,0);
//			}

			currentx += dot_pos.w;
			
//			dotShowed = 1;
		}
	}
//	if (lastDigitals!=validDigital)
	{
		nclearArea.w = tops.w * (width - numshownd) - ((tops.w * width  - (validDigital)*tops.w)>>1)+1;
//		nclearArea.h = 45;
		nclearArea.x = position->x + currentx+xOffset;
		//nclearArea.y = position->y+yOffset;
		lcd_drawRectangle(&nclearArea,Bcolor);
	}
//	lastDigitals = validDigital;	
}
//static void lcd_showTemprature(uint32_t temp,uint16_t color)
//{
//	Rect_t pos_temp =
//{
//	.x = 72,
//	.y = 59,
//	.w = 10,
//	.h = 19,
//};
//	lcd_drawRHNumber( &pos_temp,color,temp,1,2,1);	
//}
//static void lcd_showHumidity(uint32_t hum,uint16_t color)
//{
//	Rect_t pos_hum =
//{
//	.x = 155,
//	.y = 59,
//	.w = 10,
//	.h = 19,
//};
//	lcd_drawRHNumber( &pos_hum,color,hum,1,2,1);	
//}
//void lcd_showTempHum(uint32_t temp,uint32_t hum)
//{
//	lcd_showTemprature(temp,0xffff);
//	lcd_showHumidity(hum,0xffff);

//}
void lcd_showDataList(uint32_t* data,uint8_t needRedraw)
{


		Rect_t pos_allclear =
	{
		.x = 34+xOffset,
		.y = 62+yOffset,
		.w = 180,
		.h = 151,
	};
		Rect_t pos_pm25 =
	{
		.x = 105,
		.y = 62,
		.w = 16,
		.h = 19,
	};

		Rect_t pos_pm25_unit = 
	{
		.x = 185,
		.y = pos_pm25.y + 2,
		.w = 44,
		.h = 17,
	};
		Rect_t pos_pc_unit = 
	{
		.x = 184,
		.y = 60,
		.w = 44,
		.h = 15,
	};
	Rect_t pos_pm25title = 
	{
		.x = 11,
		.y = pos_pm25.y + 2,
		.w = 63,
		.h = 15,
	};
	Rect_t pos_pc_title = 
	{
		.x = 11,
		.y = 60,
		.w = 63,
		.h = 15,
	};
		Rect_t pos_row = 
	{
		.x = 0,
		.y = 45,
		.w = 240,
		.h = 1,
	};

	Rect_t pos_backBar = 
	{
		.x = pos_pm25title.x - 4,
		.y = pos_pm25title.y - 5,
		.w = 232-8,
		.h = 26,
	
	};
	Rect_t pos_colum = 
	{
		.x = pos_backBar.x - 1,
		.y = pos_backBar.y,
		.w = 1,
		.h = (pos_backBar.h + 1)*9 - 1,
	};
	
	const uint8_t* titleImage[] = {gImage_pm1title,gImage_pm2_5title,gImage_pm5title,gImage_pm10title,gImage_pc0_3,gImage_pc1_0,gImage_pc2_5,gImage_pc5_0,gImage_pc10};
	uint16_t backColor = rgb2c(0,24,48);
	uint16_t backColor2 = rgb2c(64,64,64);
	if (needRedraw)
	{
		lcd_drawRectangle(&pos_allclear,0);
//		lcd_drawRectangle(&pos_row,0xffff);
		lcd_drawRectangle(&pos_colum,backColor2);
		pos_colum.x += pos_backBar.w + 1;
		lcd_drawRectangle(&pos_colum,backColor2);
		pos_row.w = pos_backBar.w;
		pos_row.x = pos_backBar.x;
		pos_row.y = pos_backBar.y-1 ;
		lcd_drawRectangle(&pos_row,backColor2);
		pos_row.y ++;
		for (uint8_t j = 0; j < 9; j ++)
		{
			lcd_drawRectangle(&pos_backBar,backColor);			
			pos_backBar.y += pos_backBar.h ;
			pos_row.y = pos_backBar.y ;
			lcd_drawRectangle(&pos_row,backColor2);
			pos_backBar.y ++;
		}
		uint8_t i = 0;
		for (; i <4; i ++)
		{
			lcd_drawBMP(titleImage[i],&pos_pm25title,0xffff,backColor,0);
			lcd_drawBMP(gImage_ugm3,&pos_pm25_unit,backColor,0xffff,0);
			pos_pm25_unit.y += pos_pm25_unit.h + 11;
			pos_pm25title.y += pos_pm25title.h + 12;
			
		}
		pos_pc_title.y = pos_pm25title.y ;
		pos_pc_unit.y = pos_pm25_unit.y;
		for (;i < 4 + 5;i ++)
		{
			lcd_drawBMP(titleImage[i],&pos_pc_title,0xffff,backColor,0);
			pos_pc_title.y += pos_pc_title.h + 12;
			lcd_drawBMP(gImage_pc_unit,&pos_pc_unit,0xffff,backColor,0);
			pos_pc_unit.y += pos_pc_unit.h + 11;
		}
	}
	


	uint8_t i = 0;
	for (; i < 4; i ++)
	{
		lcd_drawNumber(&pos_pm25,0xffff,rgb2c(0,24,48),data[i],3,2,2);
		pos_pm25.y += pos_pm25.h + 8;
	}
	pos_pm25.x -= 10;
	for (; i < 9; i ++)
	{
		lcd_drawRHNumber(&pos_pm25,0xffff,backColor,data[i],5,5,1,0);
		pos_pm25.y += pos_pm25.h + 8;	
	}


}
//#define NUMBEROFCOLOR 6
uint8_t lcd_showColorGrade(uint32_t value,uint16_t startX,uint16_t startY,uint16_t width ,uint16_t* colorList,uint32_t* gradeList,uint8_t size)
{
	uint16_t backColor = g_lcd_backColor;
	uint8_t numberOfColor = size;
	Rect_t pos_triangle = 
	{
//		.x = 35,
//		.y = 150,
		.w = 9,
		.h = 6,
	};		
	pos_triangle.x = startX - (pos_triangle.w >> 1);
	pos_triangle.y = startY - (pos_triangle.h) - 1 ;
	Rect_t pos_colorBar = 
	{
//		.x = 40,
//		.y = 157,
		.h = 8,
	};	
	if (width < 80)
	{
		pos_colorBar.h = 6;
	}
	pos_colorBar.w = width/size;
	pos_colorBar.x = startX;
	pos_colorBar.y = startY;	

	Rect_t pos_clear ;
	pos_clear.w = (pos_colorBar.w) * numberOfColor + (pos_triangle.w) + 1;
	pos_clear.x = pos_triangle.x;
	pos_clear.y = pos_triangle.y;
	pos_clear.h = pos_triangle.h;
	
	Rect_t pos_outLine,pos_outLine2;
	uint16_t gapColor = rgb2c(164,164,164);
	pos_outLine.w = (pos_colorBar.w) * numberOfColor + 1;
	pos_outLine.x = (pos_colorBar.x) - 1;
	pos_outLine.y = (pos_colorBar.y) - 1 ;
	pos_outLine.h = 1;
	pos_outLine2.w = 1;
	pos_outLine2.x = (pos_colorBar.x) -1 ;
	pos_outLine2.y = (pos_colorBar.y) - 1 ;
	pos_outLine2.h = pos_colorBar.h + 2;	
	lcd_drawRectangle(&pos_outLine,gapColor);
	pos_outLine.y += pos_colorBar.h + 1;
	lcd_drawRectangle(&pos_outLine,gapColor);
	

	
//	pos_colorBar.h = barHeight/6;
	for (uint8_t i=0;i<numberOfColor;i++)
	{
		lcd_drawRectangle(&pos_colorBar,colorList[i]);
		pos_colorBar.x += pos_colorBar.w;
		lcd_drawRectangle(&pos_outLine2,gapColor);
		pos_outLine2.x = pos_colorBar.x ;
	//	lcd_drawRectangle(&pos_outLine2,0xffff);		
	}
	lcd_drawRectangle(&pos_outLine2,gapColor);

	uint8_t i = 1;
	for (; i < numberOfColor ; i ++)
	{
		if (value < gradeList[i] )
		{
			break;
		}
	}
//		static uint32_t lastTriangle = 40;
//		pos_triangleLast.x = lastTriangle;
		lcd_drawRectangle(&pos_clear,backColor);
		pos_triangle.x += (i-1) * (pos_colorBar.w) + (value - gradeList[i-1]) * pos_colorBar.w / (gradeList[i] - gradeList[i-1]);
//		lastTriangle = pos_triangle.x;
		lcd_drawBMP(gImage_triangle,&pos_triangle,backColor,0xffff,0);
	return (i-1);
}
void lcd_showOutDoor(uint32_t PM2_5Value,uint32_t outDoor_pm25,uint8_t needRedraw)
{
	//@TBD
	Rect_t pos_block1 = 
	{
		.x = 14,
		.y = 46,
		.w = 212,
		.h = 126,
	};	
	
		Rect_t pos_block2 = 
	{
		.x = 14,
		.y = 182,
		.w = 212,
		.h = 126,
	};	
		Rect_t pos_pm25title = 
	{
		.x = 18 + 68,
		.y = 54,
		.w = 45,
		.h = 12,
	};
		Rect_t pos_local = 
	{
		.x = 18,
		.y = 52,
		.w = 64,
		.h = 13,
	};
	Rect_t pos_outdoor = 
	{
		.x = 18,
		.y = 188 ,
		.w = 56,
		.h = 13,
	};
		Rect_t pos_pm25 =
	{
		.x = 45,
		.y = 83,
		.w = 40,
		.h = 49,
	};
		Rect_t pos_out_pm25 =
	{
		.x = 45,
		.y = 218,
		.w = 40,
		.h = 49,
	};

		Rect_t pos_pm25_unit = 
	{
		.x = 188,
		.y = 117,
		.w = 34,
		.h = 12,
	};
		Rect_t pos_line2 = 
	{
		.x = 0,
		.y = 175,
		.w = 240,
		.h = 1,
	};		
//	uint16_t block1Color = rgb2c(168,0,100);
//	uint16_t block2Color = rgb2c(0xff,0x1f,0);
	uint16_t block1Color = rgb2c(0,0,0);
	uint16_t block2Color = rgb2c(0,0,0);
	uint8_t level;
	uint16_t color;
	if (needRedraw)
	{
		lcd_drawRectangle(&pos_block1,block1Color);
		lcd_drawRectangle(&pos_block2,block2Color);		
		lcd_drawBMP(gImage_realtime,&pos_local,0xffff,block1Color,0);
//		lcd_drawBMP(gImage_pm2_5title2,&pos_pm25title,block1Color,0xffff,0);
		pos_pm25title.y += pos_block1.h + 8;
//		lcd_drawBMP(gImage_pm2_5title2,&pos_pm25title,block2Color,0xffff,0);
		lcd_drawBMP(gImage_outdoor,&pos_outdoor,0xffff,block2Color,0);		
	}
	if (block1Color == block2Color && block2Color == 0)
	{
			lcd_drawRectangle(&pos_line2,0xffff);
	}
	uint16_t colorBar[] = {rgb2c(0,228,0),rgb2c(255,255,0),rgb2c(255,64,0),rgb2c(255,0,0),rgb2c(153,0,76),rgb2c(126,0,35)};	
	uint32_t grade[] = {0,3500,7500,11500,15000,25000,99999};	
	lcd_showColorGrade(PM2_5Value,40,153,156,colorBar,grade,6);
	lcd_showColorGrade(outDoor_pm25,40,289,156,colorBar,grade,6);
#if !PM25_SINGLE_MODE
	lcd_drawNumber(&pos_pm25,0xffff,block1Color,PM2_5Value,3,1,2);
#else
	lcd_drawNumber(&pos_pm25,0xffff,block1Color,PM2_5Value,3,4,2);
#endif	
	setLevelColor(GAS_PM25,PM2_5Value,&color,&level);	
	lcd_showGrade(level,180,pos_pm25.y - 31,block1Color,1);
	pos_pm25_unit.y = pos_pm25.y + 34;
	lcd_drawBMP(gImage_smallUg,&pos_pm25_unit,block1Color,0xffff,0);
	if (outDoor_pm25)
	{
	#if !PM25_SINGLE_MODE
	lcd_drawNumber(&pos_out_pm25,0xffff,block2Color,outDoor_pm25,3,1,2);	
	#else
		lcd_drawNumber(&pos_out_pm25,0xffff,block2Color,outDoor_pm25,3,4,2);
	#endif	
	}
	else 
	{
			Rect_t pos_line = 
		{	
			.x = 87 ,
			.y = 240 ,
			.w = 20,
			.h = 2,
		};
		lcd_drawRectangle(&pos_line,0xffff);
		pos_line.x += 40;
		lcd_drawRectangle(&pos_line,0xffff);
		 //disable number distrub
	
	}	
	setLevelColor(GAS_PM25,outDoor_pm25,&color,&level);	
	lcd_showGrade(level,180,pos_out_pm25.y - 31,block2Color,1);
	pos_pm25_unit.y = pos_out_pm25.y + 34;
	lcd_drawBMP(gImage_smallUg,&pos_pm25_unit,block2Color,0xffff,0);
}

void lcd_drawBigNumber(Rect_t * position,uint16_t color,uint32_t number,uint8_t width,uint8_t type,uint8_t dot_position)
{
//	uint8_t firstzero = 1;
	uint8_t nbit[5]={0};
//	uint8_t dot_position=0xff;
//	uint8_t dotShowed = 0;
	const static uint16_t div_nums[] = {10000,1000,100,10,1};
  __IO int16_t currentx = 1;
	 Rect_t nclearArea;
	Rect_t dot_pos;
	 uint8_t i, numshownd = 0;
	 uint8_t validDigital = 0;
//	static uint8_t lastDigitals = 0;
	if (type==0)		
	{
		dot_pos.w = 24;	
//		position->w = 55;
//		position->h = 90;
	}
	else if (type==1)		
	{
		dot_pos.w = 8;	
		position->w = 15;
		position->h = 15;		
	}
	else if (type==2)		
	{
		dot_pos.w = 8;	
//		position->w = 22;
//		position->h = 43;		
	}	
	else if (type==3)		
	{
		dot_pos.w = 4;	
		position->w = 8;
		position->h = 9;		
	}	
	Rect_t tops = {.x = (position->x)+xOffset, .y = (position->y)+yOffset, .w = position->w, .h = position->h};	
	for (i=0; i<sizeof(nbit)/sizeof(uint8_t); i++)
	{
		nbit[i] = (number/div_nums[i])%10;		
	}
	validDigital = 0;//储存位数
	uint32_t temp=1;
	do
	{
		++validDigital;
		temp *= 10;
	}while(temp<=number);
	if (validDigital>4)
	{
	//	dot_position = 0xff;
	}	
	else
	{
		//dot_position = 2;
	}
	if (validDigital>4)
	{
	currentx+= dot_pos.w>>1;
	}
		
	
	nclearArea.h = position->h;
	nclearArea.w = currentx;	
	nclearArea.x = tops.x+1;
	nclearArea.y = tops.y;
	lcd_drawRectangle(&nclearArea,rgb2c(0,0,0));		

	for (i = fmin(5-width,width+dot_position-validDigital);i<5; i++)
	{
		tops.x = position->x + currentx + xOffset;
//		if (i >= dot_position)
//		{
//			firstzero = 0;
//		}
//		if (!firstzero||(nbit[i])!=0)
		if (type==0)
		{
		lcd_drawBMP(gImage_hugeNumber2[nbit[i]], &tops,color,0x0000,0);
		}
		else if (type==1)
		{
//		lcd_drawBMP(gImage_midNumber[nbit[i]], &tops,0x0000,color,0);
		}
//		else if (type==2)
//		{
//		lcd_drawBMP(gImage_midNumber[nbit[i]], &tops,color,0x0000,0);
//		}
		else if (type==3)
		{
		lcd_drawBMP(img_wifi_num[nbit[i]], &tops,color,0x0000,0);
		}		
			numshownd ++;
//			firstzero = 0;
			currentx += tops.w;
		
		if (numshownd == width)
		{
			break;
		}
		if (i == dot_position )
		{				
			dot_pos.h = tops.h;
			dot_pos.y=tops.y;
			dot_pos.x = tops.x + tops.w;	
			if (type==0)		
			{					
				lcd_drawBMP(gImage_Hdot,&dot_pos,0x0000,color,0);
			}
			else if (type==1)		
			{				
				lcd_drawBMP(gImage_middot,&dot_pos,0x0000,color,0);
			}
			else if (type==2)		
			{			
				lcd_drawBMP(gImage_middot,&dot_pos,0x0000,color,0);
			}
			else if (type==3)		
			{			
				lcd_drawBMP(gImage_littleDot,&dot_pos,0x0000,color,0);
			}						
			currentx += dot_pos.w;
			
//			dotShowed = 1;
		}
	}
	if (validDigital>4)
	{
		nclearArea.w = dot_pos.w>>1;

		nclearArea.x = position->x + currentx+xOffset;

		lcd_drawRectangle(&nclearArea,rgb2c(0,0,0));
	}
//	lastDigitals = validDigital;	

}

void lcd_showSingle(uint8_t wifistatus,uint32_t data_pm,uint8_t gas_type,uint16_t data_battery,uint8_t is_charging,uint8_t needRedraw)
{
		uint8_t level = 0;
		uint16_t color = 0;
		Rect_t pos_wifiIco = 
	{
		.x = 160,
		.y = 22,
		.w = 15,
		.h = 11,
	};
		Rect_t pos_allclear =
	{
		.x = 34+xOffset,
		.y = 62+yOffset,
		.w = 180,
		.h = 151,
	};
		Rect_t pos_pm25 =
	{
//		.x = 24,
//		.y = 105,
//		.w = 55,
//		.h = 90,
		.x = 24,
		.y = 125,
		.w = 40,
		.h = 71,
	};

		Rect_t pos_pm25_unit = 
	{
		.x = 100+xOffset,
		.y = 230+yOffset,
		.w = 44,
		.h = 17,
	};
	Rect_t pos_pm25title = 
	{
		.x = 18,
		.y = 54,
		.w = 63,
		.h = 15,
	};

	Rect_t pos_pm1title = 
	{
		.x = 70+xOffset,
		.y = 69+yOffset,
		.w = 40,
		.h = 9,
	};


		Rect_t pos_line = 
	{
		.x = 0+xOffset,
		.y = 45+yOffset,
		.w = 240,
		.h = 1,
	};
			Rect_t pos_title = 
	{
		.x = 18,
		.y = 15,
		.w = 106,
		.h = 22,
	};

	if (needRedraw)
	{
		lcd_drawRectangle(&pos_allclear,0);
	}
	static uint8_t lastStatus = 0;
	if (lastStatus!=wifistatus)
	{
		
		lcd_drawRectangle(&pos_wifiIco,0);

	//	lcd_drawRectangle(&pos_connecting,0);

	}

	if (wifistatus == 6)
	{
	lcd_drawBMP(gImage_wifiico,&pos_wifiIco,0,0xffff,0);
	}
		
//	lcd_drawBMP(gImage_bluesky,&pos_title,0,rgb2c(0,64,255),0);	
	lcd_drawRectangle(&pos_line,0xffff);
	pos_line.y = 269;
	lcd_drawRectangle(&pos_line,0xffff);
	lcd_showBattery(data_battery,is_charging,0);
//	lcd_drawBMP(gImage_pm25title,&pos_pm25title,0,0xffff,0);
	switch (gas_type)
	{
//		case GAS_PM1:
//		{
//			lcd_drawBMP(gImage_pm1title,&pos_pm1title,0,0xffff,0);
//			lcd_drawBMP(gImage_ugm3,&pos_pm25_unit,0,0xffff,0);

//			setLevelColor(GAS_PM1,data_pm/10,&color,&level);	
//			lcd_drawNumber(&pos_pm25,color,data_pm,3,0,2);
//			break;
//		}
		case GAS_PM10:
		{
			lcd_drawBMP(gImage_pm10title,&pos_pm1title,0,0xffff,0);
			lcd_drawBMP(gImage_ugm3,&pos_pm25_unit,0,0xffff,0);

			setLevelColor(GAS_PM10,data_pm/10,&color,&level);	
			lcd_drawNumber(&pos_pm25,color,0,data_pm,3,0,2);
			break;
		}
		case GAS_PM25:
		{
			lcd_drawBMP(gImage_pm2_5title,&pos_pm25title,0xffff,0,0);
			lcd_drawBMP(gImage_ugm3,&pos_pm25_unit,0,0xffff,0);
			
			{
			setLevelColor(GAS_PM25,data_pm/10,&color,&level);	
			lcd_drawNumber(&pos_pm25,0xffff,0,data_pm,3,0,2);
			}
			break;
		}
		#if 1
		case GAS_CH2O:
		{
			Rect_t pos_hchotitle = 
			{
				.x = 18,
				.y = 64,
				.w = 48,
				.h = 18,
			};
			Rect_t pos_hcho =
			{
				.x = 10,
				.y = 105,
				.w = 55,
				.h = 90,
			};

			Rect_t pos_hcho_unit =
			{
				.x = 100,
				.y = 231,
				.w = 47,
				.h = 15,
			};
			
			
			lcd_drawBMP(gImage_hchoTitle,&pos_hchotitle,0,0xffff,0);
			lcd_drawBMP(gImage_mgm32,&pos_hcho_unit,0,0xffff,0);
			
			
			if (g_main_10s_countDown)//g_main_10s_countDown
			{
				Rect_t pos_line = 
				{	
					.x = 79 ,
					.y = pos_hcho.y + (pos_hcho.h>>1),
					.w = 30,
					.h = 2,
				};
				lcd_drawRectangle(&pos_line,0xffff);
				pos_line.x += 60;
				lcd_drawRectangle(&pos_line,0xffff);
				 //disable GAS_CH2O distrub
				setLevelColor(GAS_CH2O,0,&color,&level);
			}
			else
			{				
				setLevelColor(GAS_CH2O,data_pm,&color,&level);
//				lcd_drawNumber(&pos_pm25,0xffff,0,data_pm,3,0,2);
				lcd_drawBigNumber(&pos_pm25,0xffff,data_pm,4,0,1);
			}
			//lcd_drawNumber(&pos_pm25,color,data_pm,3,0,2);
			break;
		}
		#endif
		#if 0
		case TEMPRATURE:
		{
			lcd_drawBMP(gImage_wen,&pos_RHtitle,0,0xffff,0);
			pos_RHtitle.x += 21;
			lcd_drawBMP(gImage_du,&pos_RHtitle,0,0xffff,0);

			setLevelColor(TEMPRATURE,data_pm,&color,&level);	
			lcd_drawNumber(&pos_pm25,color,data_pm,3,0,2);
			
			
				Rect_t pos_temp_unit = 
			{
				.x = 140+xOffset,
				.y = 68+yOffset,
				.w = 14,
				.h = 9,
			};				
			lcd_drawBMP(gImage_sheshidu,&pos_temp_unit,0,0xffff,0);
		

		break;
		}
		case HUMIDITY:
		{
			lcd_drawBMP(gImage_shi,&pos_RHtitle,0,0xffff,0);
			pos_RHtitle.x += 21;
			lcd_drawBMP(gImage_du,&pos_RHtitle,0,0xffff,0);			
			setLevelColor(HUMIDITY,data_pm,&color,&level);
			lcd_drawNumber(&pos_pm25,color,data_pm,3,0,2);					
			Rect_t pos_rh_unit = 
			{
				.x = 140+xOffset,
				.y = 67+yOffset,
				.w = 10,
				.h = 10,
			};
			
			lcd_drawBMP(gImage_percent2,&pos_rh_unit,0,0xffff,0);		
		break;
		}	
		#endif
		default: break;
	
	}
	
		static uint8_t testLevel = 0;
		testLevel++;
		if(testLevel>5)
		{
			testLevel=0;
		}
		level = testLevel;

		static uint8_t lastLevel = 0;
		if (lastLevel != level)
		{
			needRedraw = 1;
		}
		lastLevel = level;
		
		lcd_showAQIgrade(level,needRedraw,color,gas_type);


}

void lcd_showHistoryGrade(uint16_t startX,uint16_t startY,uint8_t grade,uint8_t needRedraw,uint16_t color,uint16_t mode)
{
//	const uint16_t color[] = {rgb2c(0,228,0),rgb2c(255,255,0),rgb2c(255,64,0),rgb2c(255,0,0),rgb2c(153,0,76),rgb2c(126,0,35)};
//	uint16_t basseY = 195;
	Rect_t pos_grade = {.x =114,.y = 180,.w = 15,.h = 14,};	
//	Rect_t pos_grade ={.x = 90+xOffset,.y = 205+yOffset,.w = 15,.h = 14,};
//	Rect_t pos_twoWord = {.x = 102+185,.y = 205+yOffset,.w = 15,.h = 14,};
	Rect_t pos_clear 	=  {.x = 86,.y = 180-3,.w = 155-86,.h = 20,};
	Rect_t pos_roundL =  {.x = 79,.y = 180-3,.w = 7,.h = 20,};
	Rect_t pos_roundR =  {.x =155,.y = 180-3,.w = 7,.h = 20,};
	pos_grade.x = startX;
	pos_grade.y = startY;
	pos_clear.x = startX - 28;
	pos_clear.y = pos_roundL.y = pos_roundR.y = startY -3;
	pos_roundL.x = startX - 35;
	pos_roundR.x = startX + 41;
//	if (needRedraw)
	{
		lcd_drawBMP(gImage_roundLeft, &pos_roundL,0,color,0);	
		lcd_drawBMP(gImage_roundRight, &pos_roundR,0,color,0);	
		lcd_drawRectangle(&pos_clear,color);
	}
	
	switch (grade)
	{
		case 0:
		case 1:	
		{			
		//	pos_grade.x = 114+xOffset;
			const uint8_t* index[] = {gImage_grade[grade]};
			lcd_drawBMP(index[0], &pos_grade,0xffff,color,0);	
				break;
		}
		case 2:
		case 3:
		case 4:			
		{	
			pos_grade.x -= 24;
			const uint8_t* index[] = {gImage_grade[grade],gImage_grade[5],gImage_grade[7],gImage_grade[8]};
			for (uint8_t i=0;i<4;i++)
			{
				lcd_drawBMP(index[i], &pos_grade,0xffff,color,0);	
				pos_grade.x += 16 ;
			}
			break;
		}
		case 5:
		{	
			pos_grade.x -= 24;	
			const uint8_t* index[] = {gImage_grade[6],gImage_grade[4],gImage_grade[7],gImage_grade[8]};
			for (uint8_t i=0;i<4;i++)
			{
				lcd_drawBMP(index[i], &pos_grade,0xffff,color,0);	
				pos_grade.x += 16 ;
			}
			break;
		}
		default:
			break;
	}
	
	

//		{
//			Rect_t pos_good = {.x = 167+xOffset,.y = 115+yOffset,.w = 15,.h = 14,};
//			const uint8_t* index[] = {gImage_grade[0]};
//			lcd_drawBMP(index[0], &pos_exllent,0,rgb2c(0,255,0),0);			
//		}
	
}
void lcd_showDateTime(RTC_TimeTypeDef* time, RTC_DateTypeDef* date)
{

	uint16_t startX = 14;

	
	Rect_t pos_number;
	pos_number.x = startX;
	pos_number.y  = 16;
	pos_number.h = 12;
	pos_number.w = 8;
//	pos_base.x = startX + (pos_number.w<<1);
	
	Rect_t pos_line;
	pos_line.w = 6;
	pos_line.h = 1;
	pos_line.y = pos_number.y + (pos_number.h >> 1);
	
	Rect_t pos_colun = {
	.x = 0,
		.y = pos_number.y,
		.w = 8,
		.h = 12
	};
//	pos_base.x = startX;
//	for (uint8_t i = 0 ; i < 5 ;i++)
//	{		
//		lcd_drawBMP(gImage_calendar[i],&pos_base,0xffff,g_lcd_backColor,0);
//		pos_base.x += (pos_number.w<<1) + pos_base.w + 1;	
//	}	

//	date->Year = 17;
//	date->Month = 12;
//	date->Date  =31;
//	void lcd_drawRHNumber(Rect_t * position,uint16_t color,uint16_t Bcolor,uint32_t number,uint8_t dot_position,uint8_t width,uint8_t isSmall)
	lcd_drawRHNumber(&pos_number,0xffff,g_lcd_backColor,(date->Year + 2000),4,4,0,0);
	pos_number.x += (pos_number.w<<2) ;
	pos_line.x = pos_number.x + 2;
	lcd_drawRectangle(&pos_line,0xffff);
	pos_number.x += pos_line.w + 2;
	
	lcd_drawRHNumber(&pos_number,0xffff,g_lcd_backColor,date->Month,2,2,0,1);
	pos_number.x += (pos_number.w<<1);
	pos_line.x = pos_number.x + 2;
	lcd_drawRectangle(&pos_line,0xffff);
	pos_number.x += pos_line.w + 2;	
	lcd_drawRHNumber(&pos_number,0xffff,g_lcd_backColor,date->Date,2,2,0,1);
	pos_number.x += (pos_number.w<<1) + pos_number.w;
	
	lcd_drawRHNumber(&pos_number,0xffff,g_lcd_backColor,time->Hours,2,2,0,1);
	pos_number.x += (pos_number.w<<1) + 2;
	pos_colun.x = pos_number.x;	
	lcd_drawBMP(gImage_colun,&pos_colun,0xffff,g_lcd_backColor,0);
	pos_number.x += pos_colun.w;
	
	lcd_drawRHNumber(&pos_number,0xffff,g_lcd_backColor,time->Minutes,2,2,0,1);
}
#if (HCHO_ENABLE && CO2_ENABLE)
void lcd_showAll(uint32_t* data_pm,uint8_t needRedraw)
{
		uint8_t level = 0;
		uint16_t color = 0;
		Rect_t pos_pm25 =
	{
		.x = 45,
		.y = 89,
		.w = 40,
		.h = 49,
	};

		Rect_t pos_pm25title = 
	{
		.x = 18,
		.y = 54,
		.w = 45,
		.h = 12,
	};
		Rect_t pos_pm25_unit = 
	{
		.x = 188,
		.y = 130,
		.w = 44,
		.h = 17,
	};


		Rect_t pos_block1 = 
	{
		.x = 14,
		.y = 46,
		.w = 212,
		.h = 160,
	};	

		Rect_t pos_block2 = 
	{
		.x = 14,
		.y = 218,
		.w = 100,
		.h = 90,
	};	
		Rect_t pos_block3 = 
	{
		.x = 125,
		.y = 218,
		.w = 100,
		.h = 90,
	};	


		Rect_t pos_line3 = 
	{
		.x = 120,
		.y = 210,
		.w = 1,
		.h = 320-210,
	};	
		Rect_t pos_line2 = 
	{
		.x = 0,
		.y = 210,
		.w = 240,
		.h = 1,
	};		
//	uint16_t block1Color = rgb2c(168,0,100);
//	uint16_t block2Color = rgb2c(0xdf,0x3f,0x1f);
	uint16_t block1Color = 0;
	uint16_t block2Color = 0;
//	lcd_drawBMP(gImage_bluesky,&pos_title,0,rgb2c(0,64,255),0);	

	if (block1Color == block2Color && block2Color == 0)
	{
			lcd_drawRectangle(&pos_line2,0xffff);
			
			lcd_drawRectangle(&pos_line3,0xffff);
	
	}

	
	if (needRedraw)
	{
		lcd_drawRectangle(&pos_block1,block1Color);
		lcd_drawRectangle(&pos_block2,block2Color);
		lcd_drawRectangle(&pos_block3,block2Color);		
	}
	uint16_t colorBar[] = {rgb2c(0,228,0),rgb2c(255,255,0),rgb2c(255,64,0),rgb2c(255,0,0),rgb2c(153,0,76),rgb2c(126,0,35)};	
	uint32_t grade[] = {0,3500,7500,11500,15000,25000,99999};	
	level = lcd_showColorGrade(data_pm[0],40,157,156,colorBar,grade,6);



	lcd_drawNumber(&pos_pm25,0xffff,block1Color,data_pm[0],3,1,2);
//	setLevelColor(GAS_PM25,data_pm[0],&color,&level);	
	lcd_drawBMP(gImage_pm2_5title2,&pos_pm25title,block1Color,0xffff,0);
	lcd_drawBMP(gImage_ugm3,&pos_pm25_unit,block1Color,0xffff,0);
	lcd_showGrade(level,180,54,g_lcd_backColor,1);
	//lcd_showHistoryGrade(118,180,level,needRedraw,color,0);
	Rect_t pos_pm10title = 
	{
		.x = 23,
		.y = 184,
		.w = 44,
		.h = 12,
	};
	Rect_t pos_pm10 =
	{
		.x = 71,
		.y = 184,
		.w = 8,
		.h = 12,
	};
	Rect_t pos_pm1d0title =
	{
		.x = 132,
		.y = 184,
		.w = 49,
		.h = 12,
	};
	Rect_t pos_pm1d0 = 
	{
		.x = 184,
		.y = 184,
		.w = 8,
		.h = 12,		
	};
	lcd_drawNumber(&pos_pm10,0xffff,block1Color,data_pm[3],3,3,2);
	lcd_drawNumber(&pos_pm1d0,0xffff,block1Color,data_pm[4],3,3,2);
	lcd_drawBMP(gImage_pm10_2,&pos_pm10title,block1Color,0xffff,0);
	lcd_drawBMP(gImage_pm1d0,&pos_pm1d0title,block1Color,0xffff,0);
	Rect_t pos_hchotitle = 
	{
		.x = 126,
		.y = 218,
		.w = 32,
		.h = 12,
//		.w = 28,
//		.h = 14,
	};
	Rect_t pos_hcho =
	{
		.x = 135,
		.y = 243,
		.w = 22,
		.h = 28,
	};

	Rect_t pos_hcho_unit =
	{
		.x = 190,
		.y = 275,
		.w = 40,
		.h = 14,
	};
	Rect_t pos_wait = 
	{
		.x = 165,
		.y = 218,
		.w = 72,
		.h = 13,		
	};

	uint16_t colorBar2[] = {rgb2c(64,255,0),rgb2c(64,255,0),rgb2c(255,255,0),rgb2c(255,126,0),rgb2c(255,0,0), rgb2c(153,0,76)};
	uint32_t grade2[] = {0,3,8,30,50,75,150};	
//	setLevelColor(GAS_CH2O,data_pm[1],&color,&level);	
	
	if (g_main_10s_countDown == 0)
	{
		lcd_drawNumber(&pos_hcho,0xffff,block2Color,data_pm[1],3,0,2);
		level = lcd_showColorGrade(data_pm[1],135,300,90,colorBar2,grade2,6);
		lcd_showGrade(level,193,218,g_lcd_backColor,1);
	//	lcd_showHistoryGrade(178,290,level,needRedraw,color,0);
	}
	else
	{
		level = lcd_showColorGrade(0,135,300,90,colorBar2,grade2,6);
		lcd_drawNumber(&pos_hcho,0xffff,block2Color,1,3,0,2);
		lcd_drawBMP(gImage_preheat,&pos_wait,0xffff,0,0);
	}
	
	lcd_drawBMP(gImage_tvoc2,&pos_hchotitle,block2Color,0xffff,0);
	lcd_drawBMP(gImage_mgm3,&pos_hcho_unit,block2Color,0xffff,0);

	Rect_t pos_co2title = 
	{
		.x = 8,
		.y = 218,
		.w = 24,
		.h = 13,
	};
	Rect_t pos_co2 =
	{
		.x = 15,
		.y = 243,
		.w = 23,
		.h = 28,
	};
	Rect_t pos_PPM =
	{
		.x = 85,
		.y = 277,
		.w = 24,
		.h = 10,
	};

	pos_wait.x = 45;

//		data_pm[2] = 600;
	
		uint16_t colorBar3[] = {rgb2c(64,255,0),rgb2c(255,255,0),rgb2c(255,126,0),rgb2c(255,0,0)};	
	uint32_t grade3[] = {0,1080,1800,2700,5000};	
	if (g_main_120s_countDown == 0)
	{
		level = lcd_showColorGrade(data_pm[2],18,300,90,colorBar3,grade3,4);		
	//	lcd_showHistoryGrade(60,290,level,needRedraw,color,0);	
		lcd_drawNumber(&pos_co2,0xffff,block2Color,data_pm[2],4,0,4);
		lcd_showGrade(level,68,218,g_lcd_backColor,1);	
		
	}
	else
	{
		level = lcd_showColorGrade(0,18,300,90,colorBar3,grade3,4);
		lcd_drawNumber(&pos_co2,0xffff,block2Color,400,4,0,4);
		lcd_drawBMP(gImage_preheat,&pos_wait,0xffff,0,0);
	}
	

//	setLevelColor(GAS_CH2O,data_pm[1],&color,&level);	
	
	lcd_drawBMP(gImage_co2,&pos_co2title,block2Color,0xffff,0);
	lcd_drawBMP(gImage_ppm,&pos_PPM,block2Color,0xffff,0);

}

#elif (CO2_ENABLE)
void lcd_showAll(uint32_t* data_pm,uint8_t needRedraw)
{
		uint8_t level = 0;
		uint16_t color = 0;
		Rect_t pos_pm25 =
	{
		.x = 45,
		.y = 89,
		.w = 40,
		.h = 49,
	};

		Rect_t pos_pm25title = 
	{
		.x = 18,
		.y = 54,
		.w = 45,
		.h = 12,
	};
		Rect_t pos_pm25_unit = 
	{
		.x = 188,
		.y = 130,
		.w = 44,
		.h = 17,
	};


		Rect_t pos_block1 = 
	{
		.x = 14,
		.y = 46,
		.w = 212,
		.h = 160,
	};	

		Rect_t pos_block2 = 
	{
		.x = 14,
		.y = 218,
		.w = 212,
		.h = 90,
	};	
//		Rect_t pos_block3 = 
//	{
//		.x = 125,
//		.y = 218,
//		.w = 100,
//		.h = 90,
//	};	


//		Rect_t pos_line3 = 
//	{
//		.x = 120,
//		.y = 210,
//		.w = 1,
//		.h = 320-210,
//	};	
		Rect_t pos_line2 = 
	{
		.x = 0,
		.y = 204,
		.w = 240,
		.h = 1,
	};		
//	uint16_t block1Color = rgb2c(168,0,100);
//	uint16_t block2Color = rgb2c(0xdf,0x3f,0x1f);
	uint16_t block1Color = 0;
	uint16_t block2Color = 0;
//	lcd_drawBMP(gImage_bluesky,&pos_title,0,rgb2c(0,64,255),0);	

	if (block1Color == block2Color && block2Color == 0)
	{
			lcd_drawRectangle(&pos_line2,0xffff);
			
//			lcd_drawRectangle(&pos_line3,0xffff);
	
	}

	
	if (needRedraw)
	{
		lcd_drawRectangle(&pos_block1,block1Color);
		lcd_drawRectangle(&pos_block2,block2Color);
//		lcd_drawRectangle(&pos_block3,block2Color);		
	}
	uint16_t colorBar[] = {rgb2c(0,228,0),rgb2c(255,255,0),rgb2c(255,64,0),rgb2c(255,0,0),rgb2c(153,0,76),rgb2c(126,0,35)};	
	uint32_t grade[] = {0,3500,7500,11500,15000,25000,99999};	
	level = lcd_showColorGrade(data_pm[0],40,157,156,colorBar,grade,6);



	lcd_drawNumber(&pos_pm25,0xffff,block1Color,data_pm[0],3,1,2);
//	setLevelColor(GAS_PM25,data_pm[0],&color,&level);	
	lcd_drawBMP(gImage_pm2_5title2,&pos_pm25title,block1Color,0xffff,0);
	lcd_drawBMP(gImage_ugm3,&pos_pm25_unit,block1Color,0xffff,0);
	lcd_showGrade(level,180,54,g_lcd_backColor,1);
	//lcd_showHistoryGrade(118,180,level,needRedraw,color,0);
	Rect_t pos_pm10title = 
	{
		.x = 23,
		.y = 184,
		.w = 44,
		.h = 12,
	};
	Rect_t pos_pm10 =
	{
		.x = 71,
		.y = 184,
		.w = 8,
		.h = 12,
	};
	Rect_t pos_pm1d0title =
	{
		.x = 132,
		.y = 184,
		.w = 49,
		.h = 12,
	};
	Rect_t pos_pm1d0 = 
	{
		.x = 184,
		.y = 184,
		.w = 8,
		.h = 12,		
	};
	lcd_drawNumber(&pos_pm10,0xffff,block1Color,data_pm[3],3,3,2);
	lcd_drawNumber(&pos_pm1d0,0xffff,block1Color,data_pm[4],3,3,2);
	lcd_drawBMP(gImage_pm10_2,&pos_pm10title,block1Color,0xffff,0);
	lcd_drawBMP(gImage_pm1d0,&pos_pm1d0title,block1Color,0xffff,0);
	
	Rect_t pos_co2title = 
	{
		.x = 18,
		.y = 213,
		.w = 24,
		.h = 13,
	};
	Rect_t pos_co2 =
	{
		.x = 39,
		.y = 233,
		.w = 40,
		.h = 49,
	};
	Rect_t pos_PPM =
	{
		.x = 200,
		.y = 280,
		.w = 24,
		.h = 10,
	};

	Rect_t pos_wait = 
	{
		.x = 160,
		.y = 213,
		.w = 40,
		.h = 14,		
	};
	
	uint16_t colorBar3[] = {rgb2c(64,255,0),rgb2c(255,255,0),rgb2c(255,126,0),rgb2c(255,0,0)};	
	uint32_t grade3[] = {0,1080,1800,2700,5000};	
	if (g_main_120s_countDown == 0)
	{
		level = lcd_showColorGrade(data_pm[2],40,300,156,colorBar3,grade3,4);
		lcd_showGrade(level,180,213,g_lcd_backColor,1);	
		lcd_drawNumber(&pos_co2,0xffff,block2Color,data_pm[2],4,1,4);
	}
	else
	{
		level = lcd_showColorGrade(0,40,300,156,colorBar3,grade3,4);
		lcd_drawNumber(&pos_co2,0xffff,block2Color,400,4,1,4);
		lcd_drawBMP(gImage_wait,&pos_wait,0,0xffff,0);
	}
	

//	setLevelColor(GAS_CH2O,data_pm[1],&color,&level);	
	
	lcd_drawBMP(gImage_co2,&pos_co2title,block2Color,0xffff,0);
	lcd_drawBMP(gImage_ppm,&pos_PPM,block2Color,0xffff,0);

}
#elif (HCHO_ENABLE)

void lcd_showAll(uint32_t* data_pm,uint8_t needRedraw)
{
		uint8_t level = 0;
		uint16_t color = 0;
		Rect_t pos_pm25 =
	{
		.x = 45,
		.y = 89,
		.w = 40,
		.h = 49,
	};

		Rect_t pos_pm25title = 
	{
		.x = 18,
		.y = 54,
		.w = 45,
		.h = 12,
	};
		Rect_t pos_pm25_unit = 
	{
		.x = 188,
		.y = 130,
		.w = 44,
		.h = 17,
	};


		Rect_t pos_block1 = 
	{
		.x = 14,
		.y = 46,
		.w = 212,
		.h = 160,
	};	

		Rect_t pos_block2 = 
	{
		.x = 14,
		.y = 218,
		.w = 212,
		.h = 90,
	};	


		Rect_t pos_line2 = 
	{
		.x = 0,
		.y = 204,
		.w = 240,
		.h = 1,
	};		
//	uint16_t block1Color = rgb2c(168,0,100);
//	uint16_t block2Color = rgb2c(0xdf,0x3f,0x1f);
	uint16_t block1Color = 0;
	uint16_t block2Color = 0;
//	lcd_drawBMP(gImage_bluesky,&pos_title,0,rgb2c(0,64,255),0);	

	if (block1Color == block2Color && block2Color == 0)
	{
		lcd_drawRectangle(&pos_line2,0xffff);
	}

	
	if (needRedraw)
	{
		lcd_drawRectangle(&pos_block1,block1Color);
		lcd_drawRectangle(&pos_block2,block2Color);
	}
	uint16_t colorBar[] = {rgb2c(0,228,0),rgb2c(255,255,0),rgb2c(255,64,0),rgb2c(255,0,0),rgb2c(153,0,76),rgb2c(126,0,35)};	
	uint32_t grade[] = {0,3500,7500,11500,15000,25000,99999};	
	level = lcd_showColorGrade(data_pm[0],40,157,156,colorBar,grade,6);

	lcd_drawNumber(&pos_pm25,0xffff,block1Color,data_pm[0],3,1,2);
//	setLevelColor(GAS_PM25,data_pm[0],&color,&level);	
	lcd_drawBMP(gImage_pm2_5title2,&pos_pm25title,block1Color,0xffff,0);
	lcd_drawBMP(gImage_ugm3,&pos_pm25_unit,block1Color,0xffff,0);
	lcd_showGrade(level,180,54,g_lcd_backColor,1);
	//lcd_showHistoryGrade(118,180,level,needRedraw,color,0);
	Rect_t pos_pm10title = 
	{
		.x = 23,
		.y = 184,
		.w = 44,
		.h = 12,
	};
	Rect_t pos_pm10 =
	{
		.x = 71,
		.y = 184,
		.w = 8,
		.h = 12,
	};
	Rect_t pos_pm1d0title =
	{
		.x = 132,
		.y = 184,
		.w = 49,
		.h = 12,
	};
	Rect_t pos_pm1d0 = 
	{
		.x = 184,
		.y = 184,
		.w = 8,
		.h = 12,		
	};
	lcd_drawNumber(&pos_pm10,0xffff,block1Color,data_pm[3],3,3,2);
	lcd_drawNumber(&pos_pm1d0,0xffff,block1Color,data_pm[4],3,3,2);
	lcd_drawBMP(gImage_pm10_2,&pos_pm10title,block1Color,0xffff,0);
	lcd_drawBMP(gImage_pm1d0,&pos_pm1d0title,block1Color,0xffff,0);

	Rect_t pos_hchotitle = 
	{
		.x = 18,
		.y = 213,
		.w = 28,
		.h = 14,
	};
	Rect_t pos_hcho =
	{
		.x = 45,
		.y = 233,
		.w = 40,
		.h = 49,
	};

	Rect_t pos_hcho_unit =
	{
		.x = 190,
		.y = 280,
		.w = 40,
		.h = 14,
	};
	Rect_t pos_wait = 
	{
		.x = 180,
		.y = 213,
		.w = 40,
		.h = 14,		
	};

	uint16_t colorBar2[] = {rgb2c(64,255,0),rgb2c(64,255,0),rgb2c(255,255,0),rgb2c(255,126,0),rgb2c(255,0,0), rgb2c(153,0,76)};	
	uint32_t grade2[] = {1,3,8,30,50,75,150};	
//	setLevelColor(GAS_CH2O,data_pm[1],&color,&level);	
	
	if (g_main_10s_countDown == 0)
	{
		lcd_drawNumber(&pos_hcho,0xffff,block2Color,data_pm[1],3,1,2);
		level = lcd_showColorGrade(data_pm[1],40,300,156,colorBar2,grade2,6);
		lcd_showGrade(level,180,213,g_lcd_backColor,1);
	//	lcd_showHistoryGrade(178,290,level,needRedraw,color,0);
	}
	else
	{
		level = lcd_showColorGrade(1,40,300,156,colorBar2,grade2,6);
		lcd_drawNumber(&pos_hcho,0xffff,block2Color,1,3,1,2);
		lcd_drawBMP(gImage_wait,&pos_wait,0,0xffff,0);
	}
	
	lcd_drawBMP(gImage_shcho,&pos_hchotitle,block2Color,0xffff,0);
	lcd_drawBMP(gImage_mgm3,&pos_hcho_unit,block2Color,0xffff,0);
}

#elif (PM25_SINGLE_MODE)
void lcd_showAll(uint32_t* data_pm,uint8_t needRedraw)
{
		uint8_t level = 0;
		uint16_t color = 0;
	#if 1
	Rect_t pos_pm25 =
	{
		.x = 22,
		.y = 115,
		.w = 56,
		.h = 83,
	};

		Rect_t pos_pm25title = 
	{
		.x = 18,
		.y = 54,
		.w = 45,
		.h = 12,
	};
		Rect_t pos_pm25_unit = 
	{
		.x = 175,
		.y = 220,
		.w = 44,
		.h = 17,
	};
		Rect_t pos_block1 = 
	{
		.x = 14,
		.y = 46,
		.w = 212,
		.h = 260,
	};	
//	uint16_t block1Color = rgb2c(168,0,100);
//	uint16_t block2Color = rgb2c(0xdf,0x3f,0x1f);
	uint16_t block1Color = 0;
	uint16_t block2Color = 0;
//	lcd_drawBMP(gImage_bluesky,&pos_title,0,rgb2c(0,64,255),0);	
	if (needRedraw)
	{
		lcd_drawRectangle(&pos_block1,block1Color);
	}
	uint16_t colorBar[] = {rgb2c(0,228,0),rgb2c(255,255,0),rgb2c(255,64,0),rgb2c(255,0,0),rgb2c(153,0,76),rgb2c(126,0,35)};	
	uint32_t grade[] = {0,3500,7500,11500,15000,25000,99999};	
	level = lcd_showColorGrade(data_pm[0],40,257,156,colorBar,grade,6);

	lcd_drawNumber(&pos_pm25,0xffff,block1Color,data_pm[0],3,1,2);
//	setLevelColor(GAS_PM25,data_pm[0],&color,&level);	
	lcd_drawBMP(gImage_pm2_5title2,&pos_pm25title,block1Color,0xffff,0);
	lcd_drawBMP(gImage_ugm3,&pos_pm25_unit,block1Color,0xffff,0);
	lcd_showGrade(level,180,54,g_lcd_backColor,1);
	//lcd_showHistoryGrade(118,180,level,needRedraw,color,0);
	Rect_t pos_pm10title = 
	{
		.x = 23,
		.y = 284,
		.w = 44,
		.h = 12,
	};
	Rect_t pos_pm10 =
	{
		.x = 71,
		.y = 284,
		.w = 8,
		.h = 12,
	};
	Rect_t pos_pm1d0title =
	{
		.x = 132,
		.y = 284,
		.w = 49,
		.h = 12,
	};
	Rect_t pos_pm1d0 = 
	{
		.x = 184,
		.y = 284,
		.w = 8,
		.h = 12,		
	};
	lcd_drawNumber(&pos_pm10,0xffff,block1Color,data_pm[3],3,3,2);
	lcd_drawNumber(&pos_pm1d0,0xffff,block1Color,data_pm[4],3,3,2);
	lcd_drawBMP(gImage_pm10_2,&pos_pm10title,block1Color,0xffff,0);
	lcd_drawBMP(gImage_pm1d0,&pos_pm1d0title,block1Color,0xffff,0);
	#endif
#if 0
		Rect_t pos_pm25 =
	{
		.x = 24,
		.y = 85,
		.w = 55,
		.h = 90,
	};

		Rect_t pos_pm25title = 
	{
		.x = 18,
		.y = 54,
		.w = 45,
		.h = 12,
	};
		Rect_t pos_pm25_unit = 
	{
		.x = 188,
		.y = 190,
		.w = 44,
		.h = 17,
	};
		Rect_t pos_block1 = 
	{
		.x = 14,
		.y = 46,
		.w = 212,
		.h = 260,
	};	
//	uint16_t block1Color = rgb2c(168,0,100);
//	uint16_t block2Color = rgb2c(0xdf,0x3f,0x1f);
	uint16_t block1Color = 0;
	uint16_t block2Color = 0;
//	lcd_drawBMP(gImage_bluesky,&pos_title,0,rgb2c(0,64,255),0);	


	if (needRedraw)
	{
		lcd_drawRectangle(&pos_block1,block1Color);
	}
	uint16_t colorBar[] = {rgb2c(0,228,0),rgb2c(255,255,0),rgb2c(255,64,0),rgb2c(255,0,0),rgb2c(153,0,76),rgb2c(126,0,35)};	
	uint32_t grade[] = {0,3500,7500,11500,15000,25000,99999};	
	level = lcd_showColorGrade(data_pm[0],40,195,140,colorBar,grade,6);

	lcd_drawNumber(&pos_pm25,0xffff,block1Color,data_pm[0],3,1,2);
//	setLevelColor(GAS_PM25,data_pm[0],&color,&level);	
	lcd_drawBMP(gImage_pm2_5title2,&pos_pm25title,block1Color,0xffff,0);
	lcd_drawBMP(gImage_ugm3,&pos_pm25_unit,block1Color,0xffff,0);
	lcd_showGrade(level,180,54,g_lcd_backColor,1);
	//lcd_showHistoryGrade(118,180,level,needRedraw,color,0);
	Rect_t pos_pm10title = 
	{
		.x = 23,
		.y = 220,
		.w = 44,
		.h = 12,
	};
	Rect_t pos_pm10 =
	{
		.x = 71,
		.y = 220,
		.w = 8,
		.h = 12,
	};
	Rect_t pos_pm1d0title =
	{
		.x = 132,
		.y = 220,
		.w = 49,
		.h = 12,
	};
	Rect_t pos_pm1d0 = 
	{
		.x = 184,
		.y = 220,
		.w = 8,
		.h = 12,		
	};
	lcd_drawNumber(&pos_pm10,0xffff,block1Color,data_pm[3],3,3,2);
	lcd_drawNumber(&pos_pm1d0,0xffff,block1Color,data_pm[4],3,3,2);
	lcd_drawBMP(gImage_pm10_2,&pos_pm10title,block1Color,0xffff,0);
	lcd_drawBMP(gImage_pm1d0,&pos_pm1d0title,block1Color,0xffff,0);
	Rect_t pos_out_pm25 =
	{
		.x = 80,
		.y = 255,
		.w = 23,
		.h = 28,
	};
		Rect_t pos_outdoor = 
	{
		.x = 18,
		.y = 265 ,
		.w = 28,
		.h = 14,
	};
		Rect_t pos_line2 = 
	{
		.x = 0,
		.y = 243,
		.w = 240,
		.h = 1,
	};		
	lcd_drawRectangle(&pos_line2,0xffff);
	lcd_drawBMP(gImage_pm2_5title2,&pos_pm25title,block2Color,0xffff,0);
	lcd_drawBMP(gImage_outdoor,&pos_outdoor,block2Color,0xffff,0);	
	pos_pm25_unit.x = 188;

	lcd_drawNumber(&pos_out_pm25,0xffff,block2Color,data_pm[5],3,0,2);
//	setLevelColor(GAS_PM25,data_pm[5],&color,&level);	
	level = lcd_showColorGrade(data_pm[5],52,295,120,colorBar,grade,6);
	lcd_showGrade(level,190,pos_out_pm25.y + 5,block2Color,1);
	pos_pm25_unit.y = pos_out_pm25.y + 30;
	lcd_drawBMP(gImage_ugm3,&pos_pm25_unit,block2Color,0xffff,0);
#endif	
}
#endif
void lcd_circleMotion(uint16_t angle,uint16_t color)
{

		Rect_t pos_center = 
	{
		.x = 125,
		.y = 130,
		.w = 1,
		.h = 1,
	};
//	lcd_drawLine(&pos_start,&pos_end,2,0xffff);
//	static uint16_t lastColor = 0;
//	if (lastColor == 0x001f) 
//	{
//		lastColor = rgb2c(0,144,255);
//		
//	}
//	else
//	{
//		lastColor = 0x001f;
//	}

	for (int32_t i = 325; i <= 325+250  ; i += 10)
	{		
		if (i <= 315 + angle)
		{
			lcd_drawCircleLine(i ,&pos_center,70,10,color);
		}
		else
		{
			lcd_drawCircleLine(i ,&pos_center,70,10,0x001f );
		}
		//HAL_Delay(1);
		
		
	}		
}
#if 0
void lcd_showMark(uint16_t *array,uint8_t size,uint8_t dot_postion)
{
		Rect_t pos_colorBar = 
	{
		.x = 195+xOffset,
		.y = 70+yOffset,
		.w = 7,
		.h = 120/(size+1),
	};
	Rect_t pos_barMark = 
	{
		.x = 203+xOffset,
		.y = 82+yOffset,
		.w = 6,
		.h = 8,
	};
	Rect_t pos_barMarkBase = 
	{
		.x = 204+xOffset,
		.y = 70+yOffset,
		.w = 6,
		.h = 8,
	};
	
		for (uint8_t i=0;i<size;i++)
	{
		__IO uint8_t bw = *(array+i)/100;
		__IO uint8_t sw = *(array+i)%100/10;
		__IO uint8_t gw = *(array+i)%10;
		uint8_t firstshowed = 0;
				pos_barMark.x = pos_barMarkBase.x;
		pos_barMark.y = pos_barMarkBase.y + pos_colorBar.h - pos_barMark.h;
		if (bw!=0 || dot_postion != 0xff)
		{
			
			firstshowed = 1;
			lcd_drawBMP(gImage_littlenumber[bw],&pos_barMark,0xffff,0,0);
			pos_barMark.x += pos_barMark.w;
			if (dot_postion == 0)
			{
				Rect_t pos_dot=
				{
					.x = pos_barMark.x ,
					.y = pos_barMark.y + pos_barMark.h -1,
					.w = 1,
					.h = 1,
				};
				lcd_drawRectangle(&pos_dot,0xffff);
				pos_barMark.x += pos_barMark.w>>1;
			
			}
		}

		if (sw!=0 || firstshowed ||  dot_postion == 1)
		{
			firstshowed = 1;
			lcd_drawBMP(gImage_littlenumber[sw],&pos_barMark,0xffff,0,0);
			pos_barMark.x += pos_barMark.w;
			if (dot_postion == 1)
			{
				Rect_t pos_dot=
				{
					.x = pos_barMark.x,
					.y = pos_barMark.y + pos_barMark.h-1 ,
					.w = 1,
					.h = 1,
				};
				lcd_drawRectangle(&pos_dot,0xffff);
				pos_barMark.x += pos_barMark.w>>1;
			
			}
		}
		if (gw!=0 || firstshowed)
		{
				lcd_drawBMP(gImage_littlenumber[gw],&pos_barMark,0xffff,0,0);
		
		}
		pos_barMarkBase.y+= pos_colorBar.h;
	
	}
		
//		lcd_drawBMP(gImage_littlenumber[2],&pos_barMark,0xffff,0,0);
//		pos_barMark.x += pos_barMark.w;
//		lcd_drawBMP(gImage_littlenumber[5],&pos_barMark,0xffff,0,0);
//		pos_barMark.x += pos_barMark.w;
//		lcd_drawBMP(gImage_littlenumber[0],&pos_barMark,0xffff,0,0);
//		pos_barMarkBase.y+= pos_colorBar.h;
//		
//		pos_barMark.x = pos_barMarkBase.x;
//		pos_barMark.y = pos_barMarkBase.y;
//		lcd_drawBMP(gImage_littlenumber[1],&pos_barMark,0xffff,0,0);
//		pos_barMark.x += pos_barMark.w;
//		lcd_drawBMP(gImage_littlenumber[5],&pos_barMark,0xffff,0,0);
//		pos_barMark.x += pos_barMark.w;
//		lcd_drawBMP(gImage_littlenumber[0],&pos_barMark,0xffff,0,0);
//		pos_barMarkBase.y+= pos_colorBar.h;
//		
//		pos_barMark.x = pos_barMarkBase.x;
//		pos_barMark.y = pos_barMarkBase.y;
//		lcd_drawBMP(gImage_littlenumber[1],&pos_barMark,0xffff,0,0);
//		pos_barMark.x += pos_barMark.w;
//		lcd_drawBMP(gImage_littlenumber[1],&pos_barMark,0xffff,0,0);
//		pos_barMark.x += pos_barMark.w;		
//		lcd_drawBMP(gImage_littlenumber[5],&pos_barMark,0xffff,0,0);
//		pos_barMarkBase.y+= pos_colorBar.h;
//		
//		pos_barMark.x = pos_barMarkBase.x;
//		pos_barMark.y = pos_barMarkBase.y;
//		lcd_drawBMP(gImage_littlenumber[7],&pos_barMark,0xffff,0,0);	
//		pos_barMark.x += pos_barMark.w;
//		lcd_drawBMP(gImage_littlenumber[5],&pos_barMark,0xffff,0,0);
//		pos_barMarkBase.y+= pos_colorBar.h;
//		
//		pos_barMark.x = pos_barMarkBase.x;
//		pos_barMark.y = pos_barMarkBase.y;
//		lcd_drawBMP(gImage_littlenumber[3],&pos_barMark,0xffff,0,0);
//		pos_barMark.x += pos_barMark.w;
//		lcd_drawBMP(gImage_littlenumber[5],&pos_barMark,0xffff,0,0);
//		pos_barMarkBase.y+= pos_colorBar.h;

}
#endif
#if 0
void lcd_showhistory(uint32_t* data,uint16_t size ,uint8_t Mode,uint32_t mainNumber,uint8_t needRedraw)
{
//		Rect_t pos_ttt = 
//	{
//		.x = 158+xOffset,
//		.y = 85+yOffset,
//		.w = 4,
//		.h = 13,
//	};
//	Rect_t pos_rh = 
//	{
//		.x = 158+xOffset,
//		.y = 161+yOffset,
//		.w = 15,
//		.h = 13,
//	};

//	Rect_t pos_title =
//	{
//		.x = 174+xOffset,
//		.y = 142+yOffset,
//		.w = 36,
//		.h = 10,
//	};
	Rect_t pos_allclear =
	{
		.x = 29+xOffset,
		.y = 67+yOffset,
		.w = 195,
		.h = 174,
	};
	Rect_t pos_main_number =
	{
		.x = 55 ,
		.y = 151,
		.w = 18,
		.h = 36,
	};
	Rect_t pos_line = 
	{
		.x = 30+xOffset,
		.y = 132+yOffset,
		.w = 150,
		.h = 1,
	};
	Rect_t pos_triangle = 
	{
		.x = 174+xOffset,
		.y = 136+yOffset,
		.w = 10,
		.h = 9,
	};
	Rect_t pos_pm25_unit = 
	{
		.x = 100+xOffset,
		.y = 225+yOffset,
		.w = 49,
		.h = 12,
	};

	Rect_t pos_pm25title = 
	{
		.x = 96+xOffset,
		.y = 18+yOffset,
		.w = 46,
		.h = 9,
	};
		Rect_t pos_RHtitle = 
	{
		.x = 132+xOffset,
		.y = 157+yOffset,
		.w = 13,
		.h = 14,
	};
	Rect_t pos_colorBar = 
	{
		.x = 195+xOffset,
		.y = 70+yOffset,
		.w = 7,
		.h = 120,
	};

	Rect_t pos_pm1title = 
	{
		.x = 129+xOffset,
		.y = 162+yOffset,
		.w = 40,
		.h = 9,
	};
	
	uint16_t color = 0;
	uint8_t level = 0;
	uint16_t barHeight = 120;
	if (needRedraw)
	{
		lcd_drawRectangle(&pos_allclear,0);
	}
	
	{
		
		
	//	lcd_drawBMP(pImage[Mode],&pos_title,0,0xffff,0);
		lcd_drawBMP(gImage_arrowup,&pos_triangle,0,0xffff,0);
		lcd_drawRectangle(&pos_line,0);
		
		
		
		
		 uint32_t tempbuffer[size];
	
		 uint32_t max=0;
		if (Mode == GAS_PM1)
		{
			uint16_t colorBar[] = {rgb2c(0,228,0),rgb2c(255,255,0),rgb2c(255,64,0),rgb2c(255,0,0),rgb2c(153,0,76),rgb2c(126,0,35)};
			pos_colorBar.h = barHeight/6;
			for (uint8_t i=0;i<6;i++)
			{
				lcd_drawRectangle(&pos_colorBar,colorBar[5-i]);
				pos_colorBar.y += pos_colorBar.h;
				
			}
			uint16_t mark[5] = {150,90,70,45,25};
			lcd_showMark(mark,5,0xff);

			lcd_drawBMP(gImage_pm1title,&pos_pm1title,0,0xffff,0);
			lcd_drawBMP(gImage_ugm3,&pos_pm25_unit,0,rgb2c(0,64,255),0);
		//	pos_main_number.x = 76;
			max = 1500;
			setLevelColor(Mode,mainNumber/10,&color,&level);
			lcd_drawNumber(&pos_main_number,color,mainNumber,3,2,2);
		//	lcd_drawHistoryNumber(&pos_main_number,color,mainNumber,3,4,0);
			for (uint8_t i=0;i<size;i++)
			{			
			tempbuffer[i] = *(data+i);
			}
		}
		else 	if (Mode == GAS_PM10)
		{
			uint16_t colorBar[] = {rgb2c(0,228,0),rgb2c(255,255,0),rgb2c(255,64,0),rgb2c(255,0,0),rgb2c(153,0,76),rgb2c(126,0,35)};	
			pos_colorBar.h = barHeight/6;				
			for (uint8_t i=0;i<6;i++)
			{
			lcd_drawRectangle(&pos_colorBar,colorBar[5-i]);
				pos_colorBar.y += pos_colorBar.h;
				
			}
			uint16_t mark[5] = {420,350,250,150,50};
			lcd_showMark(mark,5,0xff);

			lcd_drawBMP(gImage_pm10title,&pos_pm1title,0,0xffff,0);
			lcd_drawBMP(gImage_ugm3,&pos_pm25_unit,0,rgb2c(0,64,255),0);
			max = 4200;
			setLevelColor(Mode,mainNumber/10,&color,&level);
			lcd_drawNumber(&pos_main_number,color,mainNumber,3,2,2);
			for (uint8_t i=0;i<size;i++)
			{			
			tempbuffer[i] = *(data+i);
			}
		}
		else if (Mode == GAS_PM25)
		{
			uint16_t colorBar[] = {rgb2c(0,228,0),rgb2c(255,255,0),rgb2c(255,64,0),rgb2c(255,0,0),rgb2c(153,0,76),rgb2c(126,0,35)};	
			pos_colorBar.h = barHeight/6;
			for (uint8_t i=0;i<6;i++)
			{
			lcd_drawRectangle(&pos_colorBar,colorBar[5-i]);
				pos_colorBar.y += pos_colorBar.h;
				
			}
			uint16_t mark[5] = {250,150,115,75,35};
			lcd_showMark(mark,5,0xff);			
			lcd_drawBMP(gImage_pm25title,&pos_pm25title,0,0xffff,0);
			lcd_drawBMP(gImage_ugm3,&pos_pm25_unit,0,rgb2c(0,64,255),0);
			max = 2500;
			setLevelColor(Mode,mainNumber/10,&color,&level);
			lcd_drawNumber(&pos_main_number,color,mainNumber,3,2,2);
			for (uint8_t i=0;i<size;i++)
			{			
			tempbuffer[i] = *(data+i);
			}			
		}
		else if (Mode == TEMPRATURE)
		{
				Rect_t pos_temp_unit = 
			{
				.x = 133+xOffset,
				.y = 179+yOffset,
				.w = 14,
				.h = 9,
			};
			max = 3000;
			uint16_t colorBar[] = {rgb2c(225,0,186),rgb2c(71,144,255),rgb2c(0,0,228),rgb2c(0,228,0),rgb2c(255,255,0),rgb2c(255,0,0)};	
			pos_colorBar.h = barHeight/6;	
			for (uint8_t i=0;i<6;i++)
			{
			lcd_drawRectangle(&pos_colorBar,colorBar[5-i]);
				pos_colorBar.y += pos_colorBar.h;
				
			}
			uint16_t mark[5] = {30,24,16,10,5};
			lcd_showMark(mark,5,0xff);			
			lcd_drawBMP(gImage_wen,&pos_RHtitle,0,0xffff,0);
			pos_RHtitle.x += 21;
			lcd_drawBMP(gImage_du,&pos_RHtitle,0,0xffff,0);
			//uint32_t showNumber = mainNumber;
			setLevelColor(Mode,mainNumber,&color,&level);
			//pos_main_number.x += pos_main_number.w;
			lcd_drawNumber(&pos_main_number,color,mainNumber,3,2,2);
			lcd_drawBMP(gImage_sheshidu,&pos_temp_unit,0,rgb2c(0,64,255),0);
			for (uint8_t i=0;i<size;i++)
			{			
			tempbuffer[i] = (*(data+i));
			//	__IO uint32_t testnumber = tempbuffer[i];
			}
		}
		else if (Mode == HUMIDITY)
		{
				Rect_t pos_rh_unit = 
			{
				.x = 132+xOffset,
				.y = 179+yOffset,
				.w = 10,
				.h = 10,
			};
			max = 6500;
			uint16_t colorBar[] = {rgb2c(255,64,0),rgb2c(0,228,0),rgb2c(71,144,255)};	
			pos_colorBar.h = barHeight/3;	
			for (uint8_t i=0;i<3;i++)
			{
			lcd_drawRectangle(&pos_colorBar,colorBar[2-i]);
			pos_colorBar.y += pos_colorBar.h;
				
			}
			uint16_t mark[2] = {70,40};
			lcd_showMark(mark,2,0xff);			
			lcd_drawBMP(gImage_shi,&pos_RHtitle,0,0xffff,0);
			pos_RHtitle.x += 21;
			lcd_drawBMP(gImage_du,&pos_RHtitle,0,0xffff,0);
			//uint32_t showNumber = (mainNumber&0xff);
			setLevelColor(Mode,mainNumber,&color,&level);
			//pos_main_number.x += pos_main_number.w;
			lcd_drawNumber(&pos_main_number,color,mainNumber,3,2,2);
			lcd_drawBMP(gImage_percent2,&pos_rh_unit,0,rgb2c(0,64,255),0);
			for (uint8_t i=0;i<size;i++)
			{			
			tempbuffer[i] = (*(data+i));
			}			
		}
#if 0		
		if (Mode == GAS_CH2O)
		{
			
			uint16_t colorBar[] = {rgb2c(0,228,0),rgb2c(255,255,0),rgb2c(255,64,0),rgb2c(255,0,0),rgb2c(153,0,76),rgb2c(126,0,35)};
			pos_colorBar.h = barHeight/5;
			for (uint8_t i=0;i<5;i++)
			{
				lcd_drawRectangle(&pos_colorBar,colorBar[4-i]);
				pos_colorBar.y += pos_colorBar.h;
				
			}
			uint16_t mark[4] = {75,50,30,8};
			lcd_showMark(mark,4,0);
			
			pos_RHtitle.x = 134+xOffset;
			pos_RHtitle.y = 157+yOffset;
			lcd_drawBMP(gImage_jia,&pos_RHtitle,0xffff,0,0);
			pos_RHtitle.x += 21;
			lcd_drawBMP(gImage_quan,&pos_RHtitle,0xffff,0,0);
			lcd_drawBMP(gImage_MGM3,&pos_pm25_unit,0,rgb2c(0,64,255),0);
			
			if (1)
			{
					Rect_t pos_line = 
				{	
					.x = 55 + xOffset,
					.y = 168 +yOffset,
					.w = 16,
					.h = 2,
				};
				lcd_drawRectangle(&pos_line,0xffff);
				pos_line.x += 32;
				lcd_drawRectangle(&pos_line,0xffff);
				setLevelColor(Mode,0,&color,&level);
			}	
			else
			{
				pos_main_number.x -= pos_main_number.w>>1;
				setLevelColor(Mode,mainNumber,&color,&level);
				lcd_drawNumber(&pos_main_number,color,mainNumber,4,2,1);
			}

			max = 300;
			

			for (uint8_t i=0;i<size;i++)
			{			
			tempbuffer[i] = *(data+i);
			}
		
		}
	#endif
		static uint8_t lastLevel = 0;
		if (lastLevel != level)
		{
			needRedraw = 1;
		}
		lastLevel = level;
		lcd_showAQIgrade(level,needRedraw,color,Mode);		
		

		

			Rect_t pos_area =
		{
			.x = 180+xOffset,
			.y = 130+yOffset,
			.w = 150,
			.h = 60,
		};
			Rect_t pos_col =
		{
			.x = 180+xOffset,
			.y = 130+yOffset,
			.w = 4,
			.h = pos_area.h,
		};
		Rect_t pos_clear =
		{
			.x = 198+xOffset,
			.y = 76+yOffset,
			.w = 4,
			.h = 20,
		};
		
		__IO uint32_t temp = 0;
		for (uint8_t i=0;i<size;i++)
		{
			if (max<(tempbuffer[i]))
			{
			max = tempbuffer[i];
			}
		}
		for (uint8_t i=0;i<size;i++)
		{
			//tempbuffer[i] = fmax(1,(*(data+i)*1.0)/max * pos_area.h) ;
			temp = tempbuffer[i]*1.0/max * pos_area.h
			temp = MAX(1,temp) ;
//		}			
//		for (uint8_t i=0;i<size;i++)
//		{
			pos_col.x -= pos_col.w + 1;		
			pos_col.y = pos_area.y- temp;		
			pos_col.h = temp;
			
			pos_clear.x = pos_col.x;
			pos_clear.y = pos_area.y - pos_area.h;
			pos_clear.h =  pos_area.h- temp;
			setLevelColor(Mode,tempbuffer[i],&color,&level);
			lcd_drawRectangle(&pos_col,color);
			lcd_drawRectangle(&pos_clear,0);
		}
		

			
		
	}
}	


#endif
#if COMPRESS_BMP
//to compress bmp with losts of 0xff
void lcd_bmpCompress(const unsigned char *str,char *dest,uint16_t size,uint16_t *afterSize)
{
	 uint16_t length = size;
	 uint16_t ffSum = 0;
	uint16_t destCount = 0;
 for (uint16_t i=0;i<length-1;)
	{
		 uint8_t  currentByte = *(str+i);
		
		if (currentByte ==0xff )
		{
			__IO uint8_t  nextByte = *(str+i+1);
			ffSum++;
			*(dest + destCount)= 0xff;
			destCount++;
			
			while (nextByte == 0xff)
			{			
				ffSum++;
				nextByte = *(str+i+ffSum);
			}
			*(dest +destCount)= ffSum;
			destCount++;
			i = i+ffSum;
			ffSum = 0;
		}
		else
		{
			i++;
		*(dest + destCount) = currentByte;
			destCount++;
		}
	
	}
	*afterSize = destCount;

}
void lcd_Compress(char *buffer,uint16_t *afterSize)
{

lcd_bmpCompress(gImage_logo,buffer,1100,afterSize);
}
#endif
