#include "QRencode.h"
#include "string.h"
#include "stdlib.h"
#include "math.h"
#include "stdbool.h"
#include "mainConfig.h"
//version = 3, QR level = L, mask = 0



#define VERSION_5 

#ifdef VERSION_6

#define DATA_CODE_BYTE 136
#define ALL_CODE_BYTE 172
#define PIXES_PRE_ROW 41
#define BYTE_PRE_ROW ((PIXES_PRE_ROW+7)>>3)
#define FORMAT_INFO 0x08
#define ERROR_CORRECT_BLOCK 2
#endif
#ifdef VERSION_3
//version 3
#define DATA_CODE_BYTE 55
#define ALL_CODE_BYTE 70
#define PIXES_PRE_ROW 29
#define BYTE_PRE_ROW ((PIXES_PRE_ROW+7)>>3)
#define FORMAT_INFO 0x08
#define ERROR_CORRECT_BLOCK 0
#endif
#ifdef VERSION_5
//version 3
#define DATA_CODE_BYTE 108
#define ALL_CODE_BYTE 134
#define PIXES_PRE_ROW 37
#define BYTE_PRE_ROW ((PIXES_PRE_ROW+7)>>3)
#define FORMAT_INFO 0x08
#define ERROR_CORRECT_BLOCK 0
#endif

//version = 6, QR level = L, mask = 0

//#define DATA_CODE_BYTE 108
//#define ALL_CODE_BYTE 134
//#define PIXES_PRE_ROW 37
//#define BYTE_PRE_ROW ((PIXES_PRE_ROW+7)>>3)
//#define FORMAT_INFO 0x00


#define  assert(x)
//#define  min(a,b) a<b?a:b
#ifdef VERSION_3
uint8_t drawBuffer[BYTE_PRE_ROW*PIXES_PRE_ROW] = 
{
0xFE,0x00,0x03,0xF8,0x82,0x00,0x02,0x08,0xBA,0x00,0x02,0xE8,0xBA,0x00,0x02,0xE8,0xBA,0x00,0x02,0xE8,
0x82,0x00,0x02,0x08,0xFE,0xAA,0xAB,0xF8,0x00,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x02,0x00,0x0F,0x80,0x00,0x00,0x08,0x80,0xFE,0x00,0x0A,0x80,0x82,0x00,0x08,0x80,0xBA,0x00,0x0F,0x80,
0xBA,0x00,0x00,0x00,0xBA,0x00,0x00,0x00,0x82,0x00,0x00,0x00,0xFE,0x00,0x00,0x00,/*"E:\PhotooShop\S100\rs1.bmp",0*/

};
 uint8_t versionBuffer[BYTE_PRE_ROW*PIXES_PRE_ROW] = 
{
0xFF,0x80,0x07,0xF8,0xFF,0x80,0x07,0xF8,0xFF,0x80,0x07,0xF8,0xFF,0x80,0x07,0xF8,0xFF,0x80,0x07,0xF8,
0xFF,0x80,0x07,0xF8,0xFF,0xFF,0xFF,0xF8,0xFF,0x80,0x07,0xF8,0xFF,0x80,0x07,0xF8,0x02,0x00,0x00,0x00,
0x02,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x02,0x00,0x00,0x00,
0x02,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x02,0x00,0x00,0x00,
0x02,0x00,0x0F,0x80,0xFF,0x80,0x0F,0x80,0xFF,0x80,0x0F,0x80,0xFF,0x80,0x0F,0x80,0xFF,0x80,0x0F,0x80,
0xFF,0x80,0x00,0x00,0xFF,0x80,0x00,0x00,0xFF,0x80,0x00,0x00,0xFF,0x80,0x00,0x00,/*"E:\PhotooShop\S100\QRver.bmp",0*/
};
#endif
#ifdef VERSION_6
unsigned char drawBuffer[246] = { /* 0X00,0X01,0X29,0X00,0X29,0X00, */
0XFE,0X00,0X00,0X00,0X3F,0X80,0X82,0X00,0X00,0X00,0X20,0X80,0XBA,0X00,0X00,0X00,
0X2E,0X80,0XBA,0X00,0X00,0X00,0X2E,0X80,0XBA,0X00,0X00,0X00,0X2E,0X80,0X82,0X00,
0X00,0X00,0X20,0X80,0XFE,0XAA,0XAA,0XAA,0XBF,0X80,0X00,0X00,0X00,0X00,0X00,0X00,
0X02,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X02,0X00,0X00,0X00,
0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X02,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
0X00,0X00,0X00,0X00,0X02,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
0X02,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X02,0X00,0X00,0X00,
0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X02,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
0X00,0X00,0X00,0X00,0X02,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
0X02,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X02,0X00,0X00,0X00,
0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X02,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
0X00,0X00,0X00,0X00,0X02,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
0X02,0X00,0X00,0X00,0XF8,0X00,0X00,0X00,0X00,0X00,0X88,0X00,0XFE,0X00,0X00,0X00,
0XA8,0X00,0X82,0X00,0X00,0X00,0X88,0X00,0XBA,0X00,0X00,0X00,0XF8,0X00,0XBA,0X00,
0X00,0X00,0X00,0X00,0XBA,0X00,0X00,0X00,0X00,0X00,0X82,0X00,0X00,0X00,0X00,0X00,
0XFE,0X00,0X00,0X00,0X00,0X00,};/*"d:\PhotooShop\D1\qrv6_finder.bmp",0*/
unsigned char versionBuffer[246] = { /* 0X00,0X01,0X29,0X00,0X29,0X00, */
0XFF,0X80,0X00,0X00,0X7F,0X80,0XFF,0X80,0X00,0X00,0X7F,0X80,0XFF,0X80,0X00,0X00,
0X7F,0X80,0XFF,0X80,0X00,0X00,0X7F,0X80,0XFF,0X80,0X00,0X00,0X7F,0X80,0XFF,0X80,
0X00,0X00,0X7F,0X80,0XFF,0XFF,0XFF,0XFF,0XFF,0X80,0XFF,0X80,0X00,0X00,0X7F,0X80,
0XFF,0X80,0X00,0X00,0X7F,0X80,0X02,0X00,0X00,0X00,0X00,0X00,0X02,0X00,0X00,0X00,
0X00,0X00,0X02,0X00,0X00,0X00,0X00,0X00,0X02,0X00,0X00,0X00,0X00,0X00,0X02,0X00,
0X00,0X00,0X00,0X00,0X02,0X00,0X00,0X00,0X00,0X00,0X02,0X00,0X00,0X00,0X00,0X00,
0X02,0X00,0X00,0X00,0X00,0X00,0X02,0X00,0X00,0X00,0X00,0X00,0X02,0X00,0X00,0X00,
0X00,0X00,0X02,0X00,0X00,0X00,0X00,0X00,0X02,0X00,0X00,0X00,0X00,0X00,0X02,0X00,
0X00,0X00,0X00,0X00,0X02,0X00,0X00,0X00,0X00,0X00,0X02,0X00,0X00,0X00,0X00,0X00,
0X02,0X00,0X00,0X00,0X00,0X00,0X02,0X00,0X00,0X00,0X00,0X00,0X02,0X00,0X00,0X00,
0X00,0X00,0X02,0X00,0X00,0X00,0X00,0X00,0X02,0X00,0X00,0X00,0X00,0X00,0X02,0X00,
0X00,0X00,0X00,0X00,0X02,0X00,0X00,0X00,0X00,0X00,0X02,0X00,0X00,0X00,0X00,0X00,
0X02,0X00,0X00,0X00,0XF8,0X00,0XFF,0X80,0X00,0X00,0XF8,0X00,0XFF,0X80,0X00,0X00,
0XF8,0X00,0XFF,0X80,0X00,0X00,0XF8,0X00,0XFF,0X80,0X00,0X00,0XF8,0X00,0XFF,0X80,
0X00,0X00,0X00,0X00,0XFF,0X80,0X00,0X00,0X00,0X00,0XFF,0X80,0X00,0X00,0X00,0X00,
0XFF,0X80,0X00,0X00,0X00,0X00,};/*"d:\PhotooShop\D1\qrv6_mask.bmp",0*/
#endif
#ifdef VERSION_5

uint8_t versionBuffer[185] = { /* 0X00,0X01,0X25,0X00,0X25,0X00, */
0XFF,0X80,0X00,0X07,0XF8,0XFF,0X80,0X00,0X07,0XF8,0XFF,0X80,0X00,0X07,0XF8,0XFF,
0X80,0X00,0X07,0XF8,0XFF,0X80,0X00,0X07,0XF8,0XFF,0X80,0X00,0X07,0XF8,0XFF,0XFF,
0XFF,0XFF,0XF8,0XFF,0X80,0X00,0X07,0XF8,0XFF,0X80,0X00,0X07,0XF8,0X02,0X00,0X00,
0X00,0X00,0X02,0X00,0X00,0X00,0X00,0X02,0X00,0X00,0X00,0X00,0X02,0X00,0X00,0X00,
0X00,0X02,0X00,0X00,0X00,0X00,0X02,0X00,0X00,0X00,0X00,0X02,0X00,0X00,0X00,0X00,
0X02,0X00,0X00,0X00,0X00,0X02,0X00,0X00,0X00,0X00,0X02,0X00,0X00,0X00,0X00,0X02,
0X00,0X00,0X00,0X00,0X02,0X00,0X00,0X00,0X00,0X02,0X00,0X00,0X00,0X00,0X02,0X00,
0X00,0X00,0X00,0X02,0X00,0X00,0X00,0X00,0X02,0X00,0X00,0X00,0X00,0X02,0X00,0X00,
0X00,0X00,0X02,0X00,0X00,0X00,0X00,0X02,0X00,0X00,0X00,0X00,0X02,0X00,0X00,0X0F,
0X80,0XFF,0X80,0X00,0X0F,0X80,0XFF,0X80,0X00,0X0F,0X80,0XFF,0X80,0X00,0X0F,0X80,
0XFF,0X80,0X00,0X0F,0X80,0XFF,0X80,0X00,0X00,0X00,0XFF,0X80,0X00,0X00,0X00,0XFF,
0X80,0X00,0X00,0X00,0XFF,0X80,0X00,0X00,0X00,};



uint8_t drawBuffer[185] = { /* 0X00,0X01,0X25,0X00,0X25,0X00, */
0XFE,0X00,0X00,0X03,0XF8,0X82,0X00,0X00,0X02,0X08,0XBA,0X00,0X00,0X02,0XE8,0XBA,
0X00,0X00,0X02,0XE8,0XBA,0X00,0X00,0X02,0XE8,0X82,0X00,0X00,0X02,0X08,0XFE,0XAA,
0XAA,0XAB,0XF8,0X00,0X00,0X00,0X00,0X00,0X02,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
0X00,0X00,0X02,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X02,0X00,0X00,0X00,
0X00,0X00,0X00,0X00,0X00,0X00,0X02,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
0X02,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X02,0X00,0X00,0X00,0X00,0X00,
0X00,0X00,0X00,0X00,0X02,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X02,0X00,
0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X02,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
0X00,0X00,0X02,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X02,0X00,0X00,0X0F,
0X80,0X00,0X00,0X00,0X08,0X80,0XFE,0X00,0X00,0X0A,0X80,0X82,0X00,0X00,0X08,0X80,
0XBA,0X00,0X00,0X0F,0X80,0XBA,0X00,0X00,0X00,0X00,0XBA,0X00,0X00,0X00,0X00,0X82,
0X00,0X00,0X00,0X00,0XFE,0X00,0X00,0X00,0X00,};
#endif

static uint16_t setBitStream(uint8_t* buffer, uint16_t index, uint8_t data, uint8_t size)
{
	assert(size>8);
	for (uint8_t i = 0; i < size; i++)
	{
		uint8_t byteindex = (index + i)>>3;
		uint8_t bitindex = 0x07 - ((index + i)&0x07);
		if ((1<<(size - i - 1))&data)
		{
			buffer[byteindex] |= 1<<(bitindex);
		} 
		else
		{
			buffer[byteindex] &= ~(1<<(bitindex));
		}
	}
	return index+size;
}

static uint16_t getBitStream(const uint8_t* buffer, uint16_t index, uint8_t* data, uint8_t size)
{
	assert(size>8);
	*data = 0;
	for (uint8_t i = 0; i < size; i++)
	{
		uint8_t byteindex = (index + i)>>3;
		uint8_t bitindex = 0x07 - ((index + i)&0x07);
		uint8_t bitMask = 1<<bitindex;
		if (buffer[byteindex]&bitMask)
		{
			*data |= (1<<(size - i - 1));
		}
	}
	return index+size;
}

static void getRSCodeWord(uint8_t * buffer, int dataSize, int rsSize)
{
	const uint8_t byExpToInt[] = {  
		1,   2,   4,   8,  16,  32,  64, 128,  29,  58, 116, 232, 205, 135,  19,  38,
		76, 152,  45,  90, 180, 117, 234, 201, 143,   3,   6,  12,  24,  48,  96, 192,
		157,  39,  78, 156,  37,  74, 148,  53, 106, 212, 181, 119, 238, 193, 159,  35,
		70, 140,   5,  10,  20,  40,  80, 160,  93, 186, 105, 210, 185, 111, 222, 161,
		95, 190,  97, 194, 153,  47,  94, 188, 101, 202, 137,  15,  30,  60, 120, 240,
		253, 231, 211, 187, 107, 214, 177, 127, 254, 225, 223, 163,  91, 182, 113, 226,
		217, 175,  67, 134,  17,  34,  68, 136,  13,  26,  52, 104, 208, 189, 103, 206,
		129,  31,  62, 124, 248, 237, 199, 147,  59, 118, 236, 197, 151,  51, 102, 204,
		133,  23,  46,  92, 184, 109, 218, 169,  79, 158,  33,  66, 132,  21,  42,  84,
		168,  77, 154,  41,  82, 164,  85, 170,  73, 146,  57, 114, 228, 213, 183, 115,
		230, 209, 191,  99, 198, 145,  63, 126, 252, 229, 215, 179, 123, 246, 241, 255,
		227, 219, 171,  75, 150,  49,  98, 196, 149,  55, 110, 220, 165,  87, 174,  65,
		130,  25,  50, 100, 200, 141,   7,  14,  28,  56, 112, 224, 221, 167,  83, 166,
		81, 162,  89, 178, 121, 242, 249, 239, 195, 155,  43,  86, 172,  69, 138,   9,
		18,  36,  72, 144,  61, 122, 244, 245, 247, 243, 251, 235, 203, 139,  11,  22,
		44,  88, 176, 125, 250, 233, 207, 131,  27,  54, 108, 216, 173,  71, 142,   1
	};
	const uint8_t byIntToExp[] = {  
		0,   0,   1,  25,   2,  50,  26, 198,   3, 223,  51, 238,  27, 104, 199,  75,
		4, 100, 224,  14,  52, 141, 239, 129,  28, 193, 105, 248, 200,   8,  76, 113,
		5, 138, 101,  47, 225,  36,  15,  33,  53, 147, 142, 218, 240,  18, 130,  69,
		29, 181, 194, 125, 106,  39, 249, 185, 201, 154,   9, 120,  77, 228, 114, 166,
		6, 191, 139,  98, 102, 221,  48, 253, 226, 152,  37, 179,  16, 145,  34, 136,
		54, 208, 148, 206, 143, 150, 219, 189, 241, 210,  19,  92, 131,  56,  70,  64,
		30,  66, 182, 163, 195,  72, 126, 110, 107,  58,  40,  84, 250, 133, 186,  61,
		202,  94, 155, 159,  10,  21, 121,  43,  78, 212, 229, 172, 115, 243, 167,  87,
		7, 112, 192, 247, 140, 128,  99,  13, 103,  74, 222, 237,  49, 197, 254,  24,
		227, 165, 153, 119,  38, 184, 180, 124,  17,  68, 146, 217,  35,  32, 137,  46,
		55,  63, 209,  91, 149, 188, 207, 205, 144, 135, 151, 178, 220, 252, 190,  97,
		242,  86, 211, 171,  20,  42,  93, 158, 132,  60,  57,  83,  71, 109,  65, 162,
		31,  45,  67, 216, 183, 123, 164, 118, 196,  23,  73, 236, 127,  12, 111, 246,
		108, 161,  59,  82,  41, 157,  85, 170, 251,  96, 134, 177, 187, 204,  62,  90,
		203,  89,  95, 176, 156, 169, 160,  81,  11, 245,  22, 235, 122, 117,  44, 215,
		79, 174, 213, 233, 230, 231, 173, 232, 116, 214, 244, 234, 168,  80,  88, 175
	};
//	const uint8_t byRSExp16[] = {120, 104, 107, 109, 102, 161,  76,   3,  91, 191, 147, 169, 182, 194, 225, 120};
//	const uint8_t byRSExp10[] = {251,  67,  46,  61, 118,  70,  64,  94,  32,  45};  
//	const uint8_t byRSExp15[] = {  8, 183,  61,  91, 202,  37,  51,  58,  58, 237, 140, 124,   5,  99, 105}; 
//	const uint8_t byRSExp36[] = {200, 183,  98,  16, 172,  31, 246, 234,  60, 152, 115,   0, 167, 152, 113, 248, 238, 107,  18,  63,
//						   218,  37,  87, 210, 105, 177, 120,  74, 121, 196, 117, 251, 113, 233,  30, 120};	
	const uint8_t byRSExp26[] = {173, 125, 158,   2, 103, 182, 118,  17, 145, 201, 111,  28, 165,  53, 161,  21, 245, 142,  13, 102,
						    48, 227, 153, 145, 218,  70};
//	const uint8_t char*  byRSExp[] = {0,0,0,0,0,0,0,0,0,0,byRSExp10,0,0,0,0,0, byRSExp16, 0, 0, 0};
	int i, j;

	for (i = 0; i < dataSize ; ++i)
	{
		if (buffer[0] != 0)
		{
			uint8_t nExpFirst = byIntToExp[buffer[0]]; // 初項係数より乗数算出

			for (j = 0; j < rsSize; ++j)
			{
				// 各項乗数に初項乗数を加算（% 255 → α^255 = 1）
				#ifdef VERSION_3
				uint8_t nExpElement = (uint8_t)(((int)(byRSExp15[j] + nExpFirst)) % 255);
				#endif
				#ifdef VERSION_6
				uint8_t nExpElement = (uint8_t)(((int)(byRSExp36[j] + nExpFirst)) % 255);
				#endif
				#ifdef VERSION_5
				uint8_t nExpElement = (uint8_t)(((int)(byRSExp26[j] + nExpFirst)) % 255);
				#endif
				// 排他論理和による剰余算出
				buffer[j] = (uint8_t)(buffer[j + 1] ^ byExpToInt[nExpElement]);
			}

			// 残り桁をシフト
			for (j = rsSize; j < dataSize + rsSize - 1; ++j)
				buffer[j] = buffer[j + 1];
		}
		else
		{
			// 残り桁をシフト
			for (j = 0; j < dataSize + rsSize - 1; ++j)
				buffer[j] = buffer[j + 1];
		}
	}
}
uint8_t lStr ;
uint16_t bitStreamSize = 0;
static uint8_t* stringToDataCode(const char* inputString, uint8_t* rstCode)
{
//	uint8_t nDataCode = 0;
	lStr = strlen(inputString);
	uint8_t bitStreamBuffer[DATA_CODE_BYTE] = {0};
	
	bitStreamSize = setBitStream(bitStreamBuffer,bitStreamSize,0x04,4);
	bitStreamSize = setBitStream(bitStreamBuffer,bitStreamSize,lStr,8);
	for (uint8_t i = 0; i<lStr; i++)
	{
	//	bitStreamBuffer[i+1] = inputString[i];
		bitStreamSize = setBitStream(bitStreamBuffer,bitStreamSize,inputString[i],8);
	}
int ncTerminater = 4;
//	int ncTerminater = fmax(0,fmin(4, ((uint32_t)DATA_CODE_BYTE << 3) - bitStreamSize));
	bitStreamSize = setBitStream(bitStreamBuffer,bitStreamSize,0,ncTerminater);
	uint8_t paddingCode = 0xec;
	for (uint8_t i = ((bitStreamSize+7)>>3); i<DATA_CODE_BYTE; i++)
	{
		bitStreamBuffer[i] = paddingCode;
		paddingCode = (paddingCode == 0xec ? 0x11 : 0xec);
	}
	//data code compelet

	uint8_t RSbuffer[ALL_CODE_BYTE] = {0};
	memcpy(RSbuffer,bitStreamBuffer,DATA_CODE_BYTE);
	getRSCodeWord(RSbuffer, DATA_CODE_BYTE, ALL_CODE_BYTE-DATA_CODE_BYTE);
//	getRSCodeWord(RSbuffer, DATA_CODE_BYTE, 4);
	//uint8_t* rstCode = (uint8_t*)malloc(DATA_CODE_BYTE);
	if (rstCode == NULL)
	{
		return NULL;
	}
	memcpy(rstCode,bitStreamBuffer,DATA_CODE_BYTE);
	memcpy(rstCode+DATA_CODE_BYTE,RSbuffer,ALL_CODE_BYTE-DATA_CODE_BYTE);

	return rstCode;
}

static void setPattern(uint8_t* buffer, uint8_t x, uint8_t y, bool isBlack)
{
	uint16_t byteIndex = y*BYTE_PRE_ROW + (x>>3);
	uint8_t bitIndex = x&0x07;
	uint8_t bitMask = 0x80>>bitIndex;
	if (isBlack)
	{
		buffer[byteIndex] |= bitMask;
	}
	else
	{
		buffer[byteIndex] &= ~bitMask;
	}
}
static bool getPattern(const uint8_t* buffer, uint8_t x, uint8_t y)
{
	uint16_t byteIndex = y*BYTE_PRE_ROW + (x>>3);
	uint16_t bitIndex = x&0x07;
	uint16_t bitMask = 0x80>>bitIndex;
	return buffer[byteIndex] & bitMask?true:false;
}

static void setCodeWordPattern(const uint8_t* code)
{
	int16_t x = PIXES_PRE_ROW;
	int16_t y = PIXES_PRE_ROW - 1;

	int16_t nCoef_x = 1;
	int16_t nCoef_y = 1;

	for (uint16_t i = 0; i < (ALL_CODE_BYTE<<3); ++i)
	{
		do
		{
			x += nCoef_x;
			nCoef_x *= -1;

			if (nCoef_x < 0)
			{
				y += nCoef_y;

				if (y < 0 || y == PIXES_PRE_ROW)
				{
					y = (y < 0) ? 0 : PIXES_PRE_ROW - 1;
					nCoef_y *= -1;

					x -= 2;

					if (x == 6)
						--x;
				}
			}
		}
		while (getPattern(versionBuffer,x,y)); // 機能モジュールを除外
		uint8_t data;
		getBitStream(code,i,&data,1);
		setPattern(drawBuffer,x,y,data?true:false);
	}
}

void maskPattern(void)
{
	for (int i = 0; i<PIXES_PRE_ROW; i++)
	{
		for (int j = 0; j < PIXES_PRE_ROW; j++)
		{
			if (!getPattern(versionBuffer,j,i))
			{
				bool bMask = (((i + j) & 0x01) == 0);
				setPattern(drawBuffer,j,i,(bMask^getPattern(drawBuffer,j,i)));
			}
		}
	}
}

uint16_t nFormatData;
void setFormatInfoPattern(uint8_t nPatternNo)
{

	__IO uint16_t nFormatInfo = FORMAT_INFO;
//	nFormatInfo += nPatternNo;
	//	uint16_t nFormatData = nFormatInfo << 10;
	nFormatData = nFormatInfo << 10;
	// 剰余ビット算出
	for (uint8_t i = 0; i < 5; ++i)
	{
		if (nFormatData & (1 << (14 - i)))
		{
			nFormatData ^= (0x0537 << (4 - i)); // 10100110111b
		}
	}

	nFormatData += nFormatInfo << 10;

	// マスキング
	nFormatData ^= 0x5412; // 101010000010010b
//		nFormatData = 0x5412; // 101010000010010b
//nFormatData = 0x77c4;//ver 3 LEVEL_L
	// 左上位置検出パターン周り配置
	for (uint8_t i = 0; i <= 5; ++i)
	{
		setPattern(drawBuffer,8,i,(nFormatData & (1 << i))?true:false);
	}
	setPattern(drawBuffer,8,7,(nFormatData & (1 << 6))?true:false);
	setPattern(drawBuffer,8,8,(nFormatData & (1 << 7))?true:false);
	setPattern(drawBuffer,7,8,(nFormatData & (1 << 8))?true:false);

	for (uint8_t i = 9; i <= 14; ++i)
	{
		setPattern(drawBuffer,14 - i,8,(nFormatData & (1 << i))?true:false);
	}

	// 右上位置検出パターン下配置
	for (uint8_t i = 0; i <= 7; ++i)
	{
		setPattern(drawBuffer,PIXES_PRE_ROW - 1 - i,8,(nFormatData & (1 << i))?true:false);
	}
	setPattern(drawBuffer,8,PIXES_PRE_ROW -8,true);

	for (uint8_t i = 8; i <= 14; ++i)
	{
		setPattern(drawBuffer,8,PIXES_PRE_ROW - 15 + i,(nFormatData & (1 << i))?true:false);
	}
}

uint8_t* QR_stringToBuffer(const char* inputString)
{

	uint8_t code[ALL_CODE_BYTE];
	stringToDataCode(inputString,code);
	setCodeWordPattern(code);
	maskPattern();
	setFormatInfoPattern(ERROR_CORRECT_BLOCK);
	

	return drawBuffer;
}
