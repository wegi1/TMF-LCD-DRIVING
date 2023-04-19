/*
 * SSD2119_parallel16bit.c
 *
 * Created: 2017-04-15 10:52:41
 * Author : tmf
 */ 


#include "sam.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "Clk/SetClk.h"
#include "Delay/delay.h"

#include "GFXDrv.h"
#include "i8080-arm.h"
#include "ssd2119.h"
#include "Fonts/Fonts.h"
#include "Icons.h"

volatile uint32_t counter;
volatile _Bool counterrdy;

//Przerwanie compare match TC6
void TC6_Handler()
{
	REG_TC6_INTFLAG=TC_INTFLAG_MC0;   //Skasuj flagê
	REG_TC6_COUNT32_COUNT=0;
	counterrdy=true;
}

void CountSCK_Init()
{
	REG_PM_APBCMASK|=PM_APBCMASK_TC6 | PM_APBCMASK_TC7;  //W³¹cz zegar dla TC6 i 7
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN(0) | GCLK_CLKCTRL_ID_TC6_TC7;

	REG_TC6_INTENSET=TC_INTFLAG_MC0;  //Odblokuj przerwanie compare match
	REG_TC6_CTRLA=TC_CTRLA_ENABLE | TC_CTRLA_MODE_COUNT32 | TC_CTRLA_PRESCALER_DIV1 | TC_CTRLA_WAVEGEN_NFRQ;
	REG_TC6_COUNT32_CC0=SystemCoreClock;  //Zg³oœ przerwanie co jedn¹ sekundê
	NVIC_EnableIRQ(TC6_IRQn);
}

void Text_Demo()
{
	char bufor[50];

	uint16_t ile=900;
	while(--ile)
	{
		uint16_t x1=rand() % LCD_GetMaxX();   //Maksymalna wspó³rzêdna x
		uint8_t  y1=rand() % (LCD_GetMaxY()-16);   //Maksymalna wspó³rzêdna y
		uint32_t color=rand() + (((uint32_t)rand()) << 8) + (((uint32_t)rand()) << 16); //Wybierz kolor
		LCD_SetTextOpaque(x1, y1+16, "ARM&AVR. LCD-TFT", system16_array, color, 0x000000ul);

		if(counterrdy)
		{
			sprintf(bufor, "%7lu b/s, %6lu pix/s", counter*2, counter/50);
			LCD_SetTextOpaque(0, 0, bufor, system16_array, 0xfffffful, 0x000000ul);
			counter=0; counterrdy=false;
		}
	}
}

void Line_Demo()
{
	char bufor[50];

	uint16_t ile=6000;
	while(--ile)
	{
		uint16_t x1=rand() % LCD_GetMaxX();   //Maksymalna wspó³rzêdna x
		uint16_t x2=rand() % LCD_GetMaxX();
		uint8_t  y1=rand() % (LCD_GetMaxY()-16);   //Maksymalna wspó³rzêdna y minus szerokoœæ linii tekstu
		uint8_t  y2=rand() % (LCD_GetMaxY()-16);
		uint32_t color=rand() + (((uint32_t)rand()) << 8) + (((uint32_t)rand()) << 16); //Wybierz kolor
		LCD_LineTo(x1, y1+16, x2, y2+16, color);

		if(counterrdy)
		{
			sprintf(bufor, "%7lu b/s, %7lu pix/s", counter*2, counter/5);
			LCD_SetTextOpaque(0, 0, bufor, system16_array, 0xfffffful, 0x000000ul);
			counter=0; counterrdy=false;
		}
	}
}

void Bitmap_Demo()
{
	char bufor[50];

	uint16_t x,y;
	LCD_GetBitmapSize(&x, &y, image_data_Browser48x48);  //Rozmiar bitmapy

	uint16_t ile=3000;
	while(--ile)
	{
		uint16_t x1=rand() % (LCD_GetMaxX() - x);        //Maksymalna wspó³rzêdna x
		uint8_t  y1=rand() % (LCD_GetMaxY() - y - 16);   //Maksymalna wspó³rzêdna y minus szerokoœæ linii tekstu i bitmapy

		LCD_DrawBitmap_565(x1, y1 + 16, image_data_Browser48x48);

		if(counterrdy)
		{
			sprintf(bufor, "%5lu b/s, %5lu pix/s", counter*2, counter);
			LCD_SetTextOpaque(0, 0, bufor, system16_array, 0xfffffful, 0x000000ul);
			counter=0; counterrdy=false;
		}
	}
}

void Bitmap_Mono_Demo()
{
	char bufor[50];

	uint16_t x,y;
	LCD_GetBitmapSize(&x, &y, image_data_MGlass_ico);  //Rozmiar bitmapy
	uint16_t ile=3000;
	while(--ile)
	{
		uint16_t x1=rand() % (LCD_GetMaxX() - x);        //Maksymalna wspó³rzêdna x
		uint8_t  y1=rand() % (LCD_GetMaxY() - y - 16);   //Maksymalna wspó³rzêdna y minus szerokoœæ linii tekstu i bitmapy
		uint16_t col=rand() + (((uint32_t)rand()) << 8);
		uint16_t colbkg=rand() + (((uint32_t)rand()) << 8);

		LCD_DrawBitmap_Mono(x1, y1 + 16,col, colbkg, image_data_MGlass_ico);

		if(counterrdy)
		{
			sprintf(bufor, "%5lu b/s, %5lu pix/s", counter*2, counter);
			LCD_SetTextOpaque(0, 0, bufor, system16_array, 0xfffffful, 0x000000ul);
			counter=0; counterrdy=false;
		}
	}
}

void Circle_Demo()
{
	char bufor[50];

	uint16_t ile=1000;
	while(--ile)
	{
		uint8_t radius=rand() % 100;
		uint16_t x1=rand() % (LCD_GetMaxX() - 2*radius);        //Maksymalna wspó³rzêdna x
		uint8_t  y1=rand() % (LCD_GetMaxY() - 2*radius - 16);   //Maksymalna wspó³rzêdna y minus szerokoœæ linii tekstu i bitmapy
		uint32_t color=rand() + (((uint32_t)rand()) << 8) + (((uint32_t)rand()) << 16);

		LCD_Circle(x1 + radius, y1 + radius + 16, radius, true, color);

		if(counterrdy)
		{
			sprintf(bufor, "%5lu b/s, %5lu pix/s", counter*2, counter/5);
			LCD_SetTextOpaque(0, 0, bufor, system16_array, 0xfffffful, 0x000000ul);
			counter=0; counterrdy=false;
		}
	}
}

void Alfablending_Demo()
{
	char bufor[50];

	uint16_t x,y;
	LCD_GetBitmapSize(&x, &y, image_data_Browser48x48);  //Rozmiar bitmapy

	uint16_t ile=300;
	while(--ile)
	{
		uint16_t x1=rand() % (LCD_GetMaxX() - x);        //Maksymalna wspó³rzêdna x
		uint8_t  y1=rand() % (LCD_GetMaxY() - y - 16);   //Maksymalna wspó³rzêdna y minus szerokoœæ linii tekstu i bitmapy

		LCD_DrawBitmap_565_Alpha(x1, y1 + 16, 240, image_data_Browser48x48);

		if(counterrdy)
		{
			sprintf(bufor, "%5lu b/s, %5lu pix/s", counter*2, counter);
			LCD_SetTextOpaque(0, 0, bufor, system16_array, 0xfffffful, 0x000000ul);
			counter=0; counterrdy=false;
		}
	}
}

void AAText_Demo()
{
	char bufor[50];

	uint16_t ile=900;
	while(--ile)
	{
		uint16_t x1=rand() % LCD_GetMaxX();   //Maksymalna wspó³rzêdna x
		uint8_t  y1=rand() % (LCD_GetMaxY()-16);   //Maksymalna wspó³rzêdna y
		uint32_t color=rand() + (((uint32_t)rand()) << 8) + (((uint32_t)rand()) << 16); //Wybierz kolor
		LCD_SetTextAA(x1, y1+16, "ARM&AVR. LCD-TFT", Times16AA332_array, color, 0x000000ul);

		if(counterrdy)
		{
			sprintf(bufor, "%7lu b/s, %6lu pix/s", counter*2, counter/50);
			LCD_SetTextOpaque(0, 0, bufor, system16_array, 0xfffffful, 0x000000ul);
			counter=0; counterrdy=false;
		}
	}
}

void AACircle_Demo()
{
	char bufor[50];

	uint16_t ile=100;
	while(--ile)
	{
		uint32_t color=rand() + (((uint32_t)rand()) << 8) + (((uint32_t)rand()) << 16);

		for(uint8_t r=10; r<110; r+=10)
		{
			LCD_CircleAA(160, 120, r, color, 0x000000);
		}
		
		if(counterrdy)
		{
			sprintf(bufor, "%5lu b/s, %5lu pix/s", counter*2, counter/5);
			LCD_SetTextOpaque(0, 0, bufor, system16_array, 0xfffffful, 0x000000ul);
			counter=0; counterrdy=false;
		}
	}
}

int main(void)
{
    Set48MHzClk();
    delay_init();

	LCD_Interface_Init();                //Inicjalizacja inerfejsu ³¹cz¹cego z LCD
	LCD_Init262();                       //Inicjalizacja LCD

	LCD_Rect(0, 0, 319, 239, 0x000000ul);  //Skasuj ekran i przy okazji ustaw okno dostêpu do GRAM

	CountSCK_Init();                     //Inicjalizacja pomiaru szybkoœci transferu

	while(1)
	{
		LCD_Rect(0, 0, 319, 239, 0x000000ul);  //Skasuj ekran i przy okazji ustaw okno dostêpu do GRAM
		Text_Demo();
		LCD_Rect(0, 0, 319, 239, 0x000000ul);
		AAText_Demo();
		LCD_Rect(0, 0, 319, 239, 0x000000ul);
		Line_Demo();
		LCD_Rect(0, 0, 319, 239, 0x000000ul);
		Bitmap_Demo();
		LCD_Rect(0, 0, 319, 239, 0x000000ul);
		Alfablending_Demo();
		LCD_Rect(0, 0, 319, 239, 0x000000ul);
		Bitmap_Mono_Demo();
		LCD_Rect(0, 0, 319, 239, 0x000000ul);
		Circle_Demo();
		LCD_Rect(0, 0, 319, 239, 0x000000ul);
		AACircle_Demo();
		
	}
}
