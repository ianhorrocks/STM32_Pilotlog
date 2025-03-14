#include "ssd1306_oled.h"

#include <stdlib.h>
#include <string.h>

extern I2C_HandleTypeDef hi2c1;

#define SSD1306_WRITECOMMAND(command)	SSD1306_I2C_Write(SSD1306_I2C_ADDR, 0x00, (command))
#define SSD1306_WRITEDATA(data)      	SSD1306_I2C_Write(SSD1306_I2C_ADDR, 0x40, (data))
#define ABS(x)   ((x) > 0 ? (x) : -(x))

static uint8_t SSD1306_Buffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8];

typedef struct {
	uint16_t CurrentX;
	uint16_t CurrentY;
	uint8_t Inverted;
	uint8_t Initialized;
} SSD1306_t;

static SSD1306_t SSD1306;

void SSD1306_ScrollRight(uint8_t start_row, uint8_t end_row)
{
	SSD1306_WRITECOMMAND(SSD1306_RIGHT_HORIZONTAL_SCROLL);
	SSD1306_WRITECOMMAND(0x00);
	SSD1306_WRITECOMMAND(start_row);
	SSD1306_WRITECOMMAND(0x00);
	SSD1306_WRITECOMMAND(end_row);
	SSD1306_WRITECOMMAND(0x00);
	SSD1306_WRITECOMMAND(0xFF);
	SSD1306_WRITECOMMAND(SSD1306_ACTIVATE_SCROLL);
}

void SSD1306_ScrollLeft(uint8_t start_row, uint8_t end_row)
{
	SSD1306_WRITECOMMAND(SSD1306_LEFT_HORIZONTAL_SCROLL);
	SSD1306_WRITECOMMAND(0x00);
	SSD1306_WRITECOMMAND(start_row);
	SSD1306_WRITECOMMAND(0x00);
	SSD1306_WRITECOMMAND(end_row);
	SSD1306_WRITECOMMAND(0x00);
	SSD1306_WRITECOMMAND(0xFF);
	SSD1306_WRITECOMMAND(SSD1306_ACTIVATE_SCROLL);
}

void SSD1306_Scrolldiagright(uint8_t start_row, uint8_t end_row)
{
	SSD1306_WRITECOMMAND(SSD1306_SET_VERTICAL_SCROLL_AREA);
	SSD1306_WRITECOMMAND(0x00);
	SSD1306_WRITECOMMAND(SSD1306_HEIGHT);
	SSD1306_WRITECOMMAND(SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL);
	SSD1306_WRITECOMMAND(0x00);
	SSD1306_WRITECOMMAND(start_row);
	SSD1306_WRITECOMMAND(0x00);
	SSD1306_WRITECOMMAND(end_row);
	SSD1306_WRITECOMMAND(0x01);
	SSD1306_WRITECOMMAND(SSD1306_ACTIVATE_SCROLL);
}

void SSD1306_Scrolldiagleft(uint8_t start_row, uint8_t end_row)
{
	SSD1306_WRITECOMMAND(SSD1306_SET_VERTICAL_SCROLL_AREA);
	SSD1306_WRITECOMMAND(0x00);
	SSD1306_WRITECOMMAND(SSD1306_HEIGHT);
	SSD1306_WRITECOMMAND(SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL);
	SSD1306_WRITECOMMAND(0x00);
	SSD1306_WRITECOMMAND(start_row);
	SSD1306_WRITECOMMAND(0x00);
	SSD1306_WRITECOMMAND(end_row);
	SSD1306_WRITECOMMAND(0x01);
	SSD1306_WRITECOMMAND(SSD1306_ACTIVATE_SCROLL);
}

void SSD1306_Stopscroll(void)
{
	SSD1306_WRITECOMMAND(SSD1306_DEACTIVATE_SCROLL);
}

void SSD1306_InvertDisplay(int i)
{
	if(i) SSD1306_WRITECOMMAND (SSD1306_INVERTDISPLAY);
	else SSD1306_WRITECOMMAND (SSD1306_NORMALDISPLAY);
}

void SSD1306_DrawBitmap(int16_t x, int16_t y, const unsigned char* bitmap, int16_t w, int16_t h, uint16_t color)
{
	int16_t byteWidth = (w + 7) / 8;
    uint8_t byte = 0;
    for(int16_t j=0; j<h; j++, y++)
    {
        for(int16_t i=0; i<w; i++)
        {
            if(i & 7)
            {
               byte <<= 1;
            }
            else
            {
               byte = (*(const unsigned char *)(&bitmap[j * byteWidth + i / 8]));
            }
            if(byte & 0x80) SSD1306_DrawPixel(x+i, y, color);
        }
    }
}

uint8_t SSD1306_Init(void)
{
	SSD1306_I2C_Init();
	if(HAL_I2C_IsDeviceReady(&hi2c1, SSD1306_I2C_ADDR, 1, 20000) != HAL_OK){
		return 0;
	}

	uint32_t p = 2500;
	while(p>0)
		p--;

	SSD1306_WRITECOMMAND(0xAE);
	SSD1306_WRITECOMMAND(0x20);
	SSD1306_WRITECOMMAND(0x10);
	SSD1306_WRITECOMMAND(0xB0);
	SSD1306_WRITECOMMAND(0xC8);
	SSD1306_WRITECOMMAND(0x00);
	SSD1306_WRITECOMMAND(0x10);
	SSD1306_WRITECOMMAND(0x40);
	SSD1306_WRITECOMMAND(0x81);
	SSD1306_WRITECOMMAND(0xFF);
	SSD1306_WRITECOMMAND(0xA1);
	SSD1306_WRITECOMMAND(0xA6);
	SSD1306_WRITECOMMAND(0xA8);
	SSD1306_WRITECOMMAND(0x3F);
	SSD1306_WRITECOMMAND(0xA4);
	SSD1306_WRITECOMMAND(0xD3);
	SSD1306_WRITECOMMAND(0x00);
	SSD1306_WRITECOMMAND(0xD5);
	SSD1306_WRITECOMMAND(0xF0);
	SSD1306_WRITECOMMAND(0xD9);
	SSD1306_WRITECOMMAND(0x22);
	SSD1306_WRITECOMMAND(0xDA);
	SSD1306_WRITECOMMAND(0x12);
	SSD1306_WRITECOMMAND(0xDB);
	SSD1306_WRITECOMMAND(0x20);
	SSD1306_WRITECOMMAND(0x8D);
	SSD1306_WRITECOMMAND(0x14);
	SSD1306_WRITECOMMAND(0xAF);
	SSD1306_WRITECOMMAND(SSD1306_DEACTIVATE_SCROLL);
	SSD1306_Fill(BLACK);
	SSD1306_UpdateScreen();
	SSD1306.CurrentX = 0;
	SSD1306.CurrentY = 0;
	SSD1306.Initialized = 1;
	return 1;
}

void SSD1306_UpdateScreen(void)
{
	uint8_t m;
	for(m=0; m<8; m++)
	{
		SSD1306_WRITECOMMAND(0xB0 + m);
		SSD1306_WRITECOMMAND(0x00);
		SSD1306_WRITECOMMAND(0x10);
		SSD1306_I2C_WriteMulti(SSD1306_I2C_ADDR, 0x40, &SSD1306_Buffer[SSD1306_WIDTH * m], SSD1306_WIDTH);
	}
}

void SSD1306_ToggleInvert(void)
{
	uint16_t i;
	SSD1306.Inverted = !SSD1306.Inverted;
	for(i=0; i < sizeof(SSD1306_Buffer); i++)
	{
		SSD1306_Buffer[i] = ~SSD1306_Buffer[i];
	}
}

void SSD1306_Fill(SSD1306_COLOR_t color)
{
	memset(SSD1306_Buffer, (color == BLACK) ? 0x00 : 0xFF, sizeof(SSD1306_Buffer));
}

void SSD1306_DrawPixel(uint16_t x, uint16_t y, SSD1306_COLOR_t color)
{
	if(x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT){
		return;
	}

	if(SSD1306.Inverted){
		color = (SSD1306_COLOR_t)!color;
	}

	if(color == WHITE){
		SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] |= 1 << (y % 8);
	}else{
		SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] &= ~(1 << (y % 8));
	}
}

void SSD1306_GotoXY(uint16_t x, uint16_t y)
{
	SSD1306.CurrentX = x;
	SSD1306.CurrentY = y;
}

char SSD1306_Putc(char ch, FontDef_t* Font, SSD1306_COLOR_t color)
{
	uint32_t i, b, j;
	if(SSD1306_WIDTH <= (SSD1306.CurrentX + Font->FontWidth) || SSD1306_HEIGHT <= (SSD1306.CurrentY + Font->FontHeight)){
		return 0;
	}

	for(i=0; i<Font->FontHeight; i++){
		b = Font->data[(ch - 32) * Font->FontHeight + i];
		for(j=0; j<Font->FontWidth; j++){
			if((b << j) & 0x8000){
				SSD1306_DrawPixel(SSD1306.CurrentX + j, (SSD1306.CurrentY + i), (SSD1306_COLOR_t) color);
			}else{
				SSD1306_DrawPixel(SSD1306.CurrentX + j, (SSD1306.CurrentY + i), (SSD1306_COLOR_t)!color);
			}
		}
	}
	SSD1306.CurrentX += Font->FontWidth;
	return ch;
}

char SSD1306_Puts(char* str, FontDef_t* Font, SSD1306_COLOR_t color)
{
	while(*str)
	{
		if(SSD1306_Putc(*str, Font, color) != *str){
			return *str;
		}
		str++;
	}
	return *str;
}

void SSD1306_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, SSD1306_COLOR_t c)
{
	int16_t dx, dy, sx, sy, err, e2, i, tmp;

	if(x0 >= SSD1306_WIDTH){
		x0 = SSD1306_WIDTH - 1;
	}
	if(x1 >= SSD1306_WIDTH){
		x1 = SSD1306_WIDTH - 1;
	}
	if(y0 >= SSD1306_HEIGHT){
		y0 = SSD1306_HEIGHT - 1;
	}
	if(y1 >= SSD1306_HEIGHT){
		y1 = SSD1306_HEIGHT - 1;
	}
	dx = (x0 < x1) ? (x1 - x0) : (x0 - x1);
	dy = (y0 < y1) ? (y1 - y0) : (y0 - y1);
	sx = (x0 < x1) ? 1 : -1;
	sy = (y0 < y1) ? 1 : -1;
	err = ((dx > dy) ? dx : -dy) / 2;

	if(dx == 0){
		if(y1 < y0){
			tmp = y1;
			y1 = y0;
			y0 = tmp;
		}
		if(x1 < x0){
			tmp = x1;
			x1 = x0;
			x0 = tmp;
		}
		for(i=y0; i<=y1; i++){
			SSD1306_DrawPixel(x0, i, c);
		}
		return;
	}

	if(dy == 0){
		if(y1 < y0){
			tmp = y1;
			y1 = y0;
			y0 = tmp;
		}
		if(x1 < x0){
			tmp = x1;
			x1 = x0;
			x0 = tmp;
		}
		for(i = x0; i <= x1; i++){
			SSD1306_DrawPixel(i, y0, c);
		}
		return;
	}

	while(1)
	{
		SSD1306_DrawPixel(x0, y0, c);
		if(x0 == x1 && y0 == y1){
			break;
		}
		e2 = err;
		if(e2 > -dx){
			err -= dy;
			x0 += sx;
		}
		if(e2 < dy){
			err += dx;
			y0 += sy;
		}
	}
}

void SSD1306_DrawRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, SSD1306_COLOR_t c)
{
	if(x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT){
		return;
	}
	if((x + w) >= SSD1306_WIDTH){
		w = SSD1306_WIDTH - x;
	}
	if((y + h) >= SSD1306_HEIGHT){
		h = SSD1306_HEIGHT - y;
	}
	SSD1306_DrawLine(x, y, x + w, y, c);
	SSD1306_DrawLine(x, y + h, x + w, y + h, c);
	SSD1306_DrawLine(x, y, x, y + h, c);
	SSD1306_DrawLine(x + w, y, x + w, y + h, c);
}

void SSD1306_DrawFilledRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, SSD1306_COLOR_t c) {
	uint8_t i;
	if(x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT){
		return;
	}
	if((x + w) >= SSD1306_WIDTH){
		w = SSD1306_WIDTH - x;
	}
	if((y + h) >= SSD1306_HEIGHT){
		h = SSD1306_HEIGHT - y;
	}
	for(i=0; i<=h; i++){
		SSD1306_DrawLine(x, y + i, x + w, y + i, c);
	}
}

void SSD1306_DrawTriangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, SSD1306_COLOR_t color)
{
	SSD1306_DrawLine(x1, y1, x2, y2, color);
	SSD1306_DrawLine(x2, y2, x3, y3, color);
	SSD1306_DrawLine(x3, y3, x1, y1, color);
}

void SSD1306_DrawFilledTriangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, SSD1306_COLOR_t color)
{
	int16_t deltax = 0, deltay = 0, x = 0, y = 0, xinc1 = 0, xinc2 = 0,
	yinc1 = 0, yinc2 = 0, den = 0, num = 0, numadd = 0, numpixels = 0,
	curpixel = 0;

	deltax = ABS(x2 - x1);
	deltay = ABS(y2 - y1);
	x = x1;
	y = y1;

	if(x2 >= x1){
		xinc1 = 1;
		xinc2 = 1;
	}else{
		xinc1 = -1;
		xinc2 = -1;
	}

	if(y2 >= y1){
		yinc1 = 1;
		yinc2 = 1;
	}else{
		yinc1 = -1;
		yinc2 = -1;
	}

	if(deltax >= deltay){
		xinc1 = 0;
		yinc2 = 0;
		den = deltax;
		num = deltax / 2;
		numadd = deltay;
		numpixels = deltax;
	}else{
		xinc2 = 0;
		yinc1 = 0;
		den = deltay;
		num = deltay / 2;
		numadd = deltax;
		numpixels = deltay;
	}

	for(curpixel=0; curpixel<=numpixels; curpixel++)
	{
		SSD1306_DrawLine(x, y, x3, y3, color);
		num += numadd;
		if(num >= den){
			num -= den;
			x += xinc1;
			y += yinc1;
		}
		x += xinc2;
		y += yinc2;
	}
}

void SSD1306_DrawCircle(int16_t x0, int16_t y0, int16_t r, SSD1306_COLOR_t c)
{
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

    SSD1306_DrawPixel(x0, y0 + r, c);
    SSD1306_DrawPixel(x0, y0 - r, c);
    SSD1306_DrawPixel(x0 + r, y0, c);
    SSD1306_DrawPixel(x0 - r, y0, c);

    while(x < y){
        if(f >= 0){
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        SSD1306_DrawPixel(x0 + x, y0 + y, c);
        SSD1306_DrawPixel(x0 - x, y0 + y, c);
        SSD1306_DrawPixel(x0 + x, y0 - y, c);
        SSD1306_DrawPixel(x0 - x, y0 - y, c);

        SSD1306_DrawPixel(x0 + y, y0 + x, c);
        SSD1306_DrawPixel(x0 - y, y0 + x, c);
        SSD1306_DrawPixel(x0 + y, y0 - x, c);
        SSD1306_DrawPixel(x0 - y, y0 - x, c);
    }
}

void SSD1306_DrawFilledCircle(int16_t x0, int16_t y0, int16_t r, SSD1306_COLOR_t c)
{
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

    SSD1306_DrawPixel(x0, y0 + r, c);
    SSD1306_DrawPixel(x0, y0 - r, c);
    SSD1306_DrawPixel(x0 + r, y0, c);
    SSD1306_DrawPixel(x0 - r, y0, c);
    SSD1306_DrawLine(x0 - r, y0, x0 + r, y0, c);

    while(x < y){
        if(f >= 0){
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        SSD1306_DrawLine(x0 - x, y0 + y, x0 + x, y0 + y, c);
        SSD1306_DrawLine(x0 + x, y0 - y, x0 - x, y0 - y, c);
        SSD1306_DrawLine(x0 + y, y0 + x, x0 - y, y0 + x, c);
        SSD1306_DrawLine(x0 + y, y0 - x, x0 - y, y0 - x, c);
    }
}

void SSD1306_Clear(void)
{
	SSD1306_Fill(0);
    SSD1306_UpdateScreen();
}

void SSD1306_ON(void)
{
	SSD1306_WRITECOMMAND(0x8D);
	SSD1306_WRITECOMMAND(0x14);
	SSD1306_WRITECOMMAND(0xAF);
}

void SSD1306_OFF(void)
{
	SSD1306_WRITECOMMAND(0x8D);
	SSD1306_WRITECOMMAND(0x10);
	SSD1306_WRITECOMMAND(0xAE);
}

void SSD1306_I2C_Init(void)
{
	uint32_t p = 250000;
	while(p>0)
		p--;
}

void SSD1306_I2C_WriteMulti(uint8_t address, uint8_t reg, uint8_t* data, uint16_t count)
{
	uint8_t dt[256];
	dt[0] = reg;
	uint8_t i;
	for(i = 0; i < count; i++)
	dt[i+1] = data[i];
	HAL_I2C_Master_Transmit(&hi2c1, address, dt, count+1, 10);
}


void SSD1306_I2C_Write(uint8_t address, uint8_t reg, uint8_t data)
{
	uint8_t dt[2];
	dt[0] = reg;
	dt[1] = data;
	HAL_I2C_Master_Transmit(&hi2c1, address, dt, 2, 10);
}
