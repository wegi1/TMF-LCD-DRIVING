/*
 * ssd2119_DMA.c
 *
 * Created: 2014-11-09 16:11:46
 *  Author: tmf
 */ 

#include "sam.h"
#include <stdbool.h>
#include <stdlib.h>
#include "Clk/SetClk.h"
#include "Delay/delay.h"
#include "SPI/spi.h"
#include "ssd2119.h"

typedef struct
{
	uint8_t reg;        //Numer rejestru uk³¹du SSD
	uint16_t value;     //Wartoœæ rejestru
} SSD_Regs;

const SSD_Regs SSD_Init262[] = {{0x28, 0x0006}, {0x00, 0x0001}, {0x10, 0x0000}, {0x01, 0x72EF}, {0x02, 0x0600}, {0x03, 0x6A38}, {0x11, 0x4870},
{0x0F, 0x0000}, {0x0B, 0x5308}, {0x0C, 0x0003}, {0x0D, 0x000A}, {0x0E, 0x2E00}, {0x1E, 0x00BE}, {0x25, 0x8000},
{0x26, 0x7800}, {0x4E, 0x0000}, {0x4F, 0x0000}, {0x12, 0x08D9}, {0x30, 0x0000}, {0x31, 0x0104}, {0x32, 0x0100},
{0x33, 0x0305}, {0x34, 0x0505}, {0x35, 0x0305}, {0x36, 0x0707}, {0x37, 0x0300}, {0x3A, 0x1200}, {0x3B, 0x0800},
{0x07, 0x0033}};

const SSD_Regs SSD_Init65[] = {{0x28, 0x0006}, {0x00, 0x0001}, {0x10, 0x0000}, {0x01, 0x72EF}, {0x02, 0x0600}, {0x03, 0x6A38}, {0x11, 0x6870},
{0x0F, 0x0000}, {0x0B, 0x5308}, {0x0C, 0x0003}, {0x0D, 0x000A}, {0x0E, 0x2E00}, {0x1E, 0x00BE}, {0x25, 0x8000},
{0x26, 0x7800}, {0x4E, 0x0000}, {0x4F, 0x0000}, {0x12, 0x08D9}, {0x30, 0x0000}, {0x31, 0x0104}, {0x32, 0x0100},
{0x33, 0x0305}, {0x34, 0x0505}, {0x35, 0x0305}, {0x36, 0x0707}, {0x37, 0x0300}, {0x3A, 0x1200}, {0x3B, 0x0800},
{0x07, 0x0033}};

void LCD_Init262()
{
	ssd2119_RESET();   //Zresetuj kontroler

	for(uint8_t indeks=0; indeks < sizeof(SSD_Init262)/sizeof(SSD_Init262[0]); indeks++)
	{
		ssd2119_SendCmd(SSD_Init262[indeks].reg);
		ssd2119_SendDataWord(SSD_Init262[indeks].value);
	}
}

void LCD_Init65()
{
	ssd2119_RESET();   //Zresetuj kontroler

	for(uint8_t indeks=0; indeks < sizeof(SSD_Init65)/sizeof(SSD_Init65[0]); indeks++)
	{
		ssd2119_SendCmd(SSD_Init65[indeks].reg);
		ssd2119_SendDataWord(SSD_Init65[indeks].value);
	}
}

void  SSD2119_Rect(uint16_t x1, uint8_t y1, uint16_t x2, uint8_t y2, uint32_t color)
{
	ssd2119_SendCmd(0x44);                //Górny i dolny brzeg obszaru
	ssd2119_SendDataWord(((uint16_t)y1) | (((uint16_t)y2) << 8));
	ssd2119_SendCmd(0x45);                //Lewy brzeg obszaru
	ssd2119_SendDataWord(x1);
	ssd2119_SendCmd(0x46);                //Prawy brzeg obszaru
	ssd2119_SendDataWord(x2);

	ssd2119_SendCmd(0x4E);    // Pozycja X piksela
	ssd2119_SendDataWord(x1);
	ssd2119_SendCmd(0x4F);    // Pozycja Y piksela
	ssd2119_SendDataWord(y1);

	ssd2119_SendCmd(0x22);    // Zapis pod wskazan¹ pozycjê

	ssd2119_CS(0);
	for(uint8_t y=y1; y<=y2; y++)
	for(uint16_t x=x1; x<=x2; x++)
	{
		ssd2119_SendDataByte(color & 0xff);  //Red
		ssd2119_SendDataByte((color >> 8) & 0xff);  //Green
		ssd2119_SendDataByte((color >> 16) & 0xff);  //Blue
	}
	ssd2119_CS(1);
}

int main(void)
{
	Set48MHzClk();
	
	LCD_Interface_Init();
	LCD_Init262();
	
    while(1)
    {
	    uint16_t x1=rand() % 320;   //Maksymalna wspó³rzêdna x
	    uint16_t x2=rand() % 320;
	    if(x1>x2)                   //x2 nie mo¿e byæ mniejsze ni¿ x1
	    {
		    uint16_t x=x2;
		    x2=x1;
		    x1=x;
	    }
	    uint8_t  y1=rand() % 240;   //Maksymalna wspó³rzêdna y
	    uint8_t  y2=rand() % 240;
	    if(y1>y2)                   //y2 nie mo¿e byæ mniejsze ni¿ y1
	    {
		    uint8_t y=y2;
		    y2=y1;
		    y1=y;
	    }
	    uint32_t color=rand() | (((uint32_t)rand()) << 8) | (((uint32_t)rand()) << 16); //Wybierz kolor
	    SSD2119_Rect(x1, y1, x2, y2, color);
    }
}
