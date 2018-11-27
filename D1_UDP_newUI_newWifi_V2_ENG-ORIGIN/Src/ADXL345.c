#include "ADXL345.h"
#define ADXL345_ADDRESS (0x53<<1)
I2C_HandleTypeDef* g_adxl345_hi2c;

static uint8_t hw_write(uint8_t address, uint8_t value)
{
	uint8_t buffer[2] = {address,value};
	if (HAL_I2C_Master_Transmit(g_adxl345_hi2c,ADXL345_ADDRESS,buffer,2,10) == HAL_OK)
	{
		return 1;
	}
	return 0;
}
static uint8_t hw_read(uint8_t address, uint8_t* value, uint8_t size)
{
	if(HAL_I2C_Master_Transmit(g_adxl345_hi2c,ADXL345_ADDRESS,&address,1,10) == HAL_OK)
	{
			if (HAL_I2C_Master_Receive(g_adxl345_hi2c,ADXL345_ADDRESS,value,size,10) == HAL_OK)
			{
				return 1;
			}
	}
	return 0;
}

void adxl345_init(I2C_HandleTypeDef *hi2c)
{
	g_adxl345_hi2c = hi2c;
	hw_write(0x31,0x08);
	hw_write(0x2c,0x08);
	hw_write(0x2d,0x08);
	hw_write(0x2e,0xf0);
	/*DUR寄存器0x21的值大于0x10(10ms)， latent
寄存器0x22的值大于0x10(20ms)， window寄存器0x23的值大于0x40
(80ms)和THRESH_TAP寄存器0x1d的值大于0x30(3g)。*/
	hw_write(0x1d,0x30);// tap
	hw_write(0x21,0x30);
	hw_write(0x22,0x30);
	hw_write(0x23,0xA0);
	
	hw_write(0x24,0x30);	//act 
	hw_write(0x27,0x60);	//act 
	hw_write(0x2a,0x06);
//	hw_write(0x2e,0x80);
	hw_write(0x1e,0x00);
	hw_write(0x1f,0x00);
	hw_write(0x20,0x00);
}

uint8_t adxl345_getXYZ(int16_t* x,int16_t* y,int16_t* z)
{
	uint8_t data[6];

	if (!hw_read(0x32,data,sizeof(data)))
	{
		return 0;
	};
	*x = *(int16_t*)(data);
	*y = *(int16_t*)(data+2);
	*z = *(int16_t*)(data+4);

	
//	console_printf(g_main_console,"x= %i, y=%i, z=%i \r\n",x,y,z);
	return 1;
}
uint8_t	adxl345_getShaking(void)
{
	__IO uint8_t buffer;
	hw_read(0x30,&buffer,1);
	if ((buffer&0x80) && (buffer&0x60) && (buffer&0x10))
	{
		//data[0]=buffer;
	return 1;
	}
	return 0;
}