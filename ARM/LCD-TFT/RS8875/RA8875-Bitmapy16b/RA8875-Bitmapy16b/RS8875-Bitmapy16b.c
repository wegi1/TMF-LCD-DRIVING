/*
 * ILI_parallel.c
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
#include "RA8875.h"
#include "Fonts/Fonts.h"
#include "Icons.h"

void Bitmap_Mono_DemoHW(_Bool transparency)
{
	uint16_t x,y;
	LCD_GetBitmapSize(&x, &y, image_data_rocketbw134);  //Rozmiar bitmapy
	LCD_StoreBMPMonoInGRAM16b(0, 0, image_data_rocketbw134, 1);		//Zapisz w GRAM obraz rakiety
	uint16_t ile=3000;
	while(--ile)
	{
		uint16_t x1=rand() % (LCD_GetMaxX() - x);        //Maksymalna wspó³rzêdna x
		uint16_t y1=rand() % (LCD_GetMaxY() - y - 16);  //Maksymalna wspó³rzêdna y minus szerokoœæ linii tekstu i bitmapy
		uint16_t col=rand();
		uint16_t colbkg=rand();

		LCD_MoveBMPMonoFromGRAM16b(x1, y1, 134, 136, 0, 0, col, colbkg, 1, 0, transparency);
	}
}

void Bitmap_Mono_DemoHW_MCU(_Bool transparency)
{
	uint16_t x,y;
	LCD_GetBitmapSize(&x, &y, image_data_rocketbw134);  //Rozmiar bitmapy
	uint16_t ile=3000;
	while(--ile)
	{
		uint16_t x1=rand() % (LCD_GetMaxX() - x);        //Maksymalna wspó³rzêdna x
		uint16_t y1=rand() % (LCD_GetMaxY() - y - 16);  //Maksymalna wspó³rzêdna y minus szerokoœæ linii tekstu i bitmapy
		uint16_t col=rand();
		uint16_t colbkg=rand();

		LCD_DrawBitmap_Mono(x1, y1, col, colbkg, image_data_rocketbw134, transparency);
	}
}

void Bitmap565_Demo()
{
	uint16_t x,y;
	LCD_GetBitmapSize(&x, &y, image_data_Browser48x48);  //Rozmiar bitmapy

	uint16_t ile=3000;
	while(--ile)
	{
		uint16_t x1=rand() % (LCD_GetMaxX() - x);        //Maksymalna wspó³rzêdna x
		uint16_t  y1=rand() % (LCD_GetMaxY() - y - 16);   //Maksymalna wspó³rzêdna y minus szerokoœæ linii tekstu i bitmapy

		LCD_DrawBitmap_565(x1, y1 + 16, image_data_Browser48x48);
	}
}

void Bitmap565HW_Demo()
{
	LCD_StoreBMP565InGRAM(0, 0, image_data_Browser48x48, 1);
	
	uint16_t x,y;
	LCD_GetBitmapSize(&x, &y, image_data_Browser48x48);  //Rozmiar bitmapy

	uint16_t ile=3000;
	while(--ile)
	{
		uint16_t x1=rand() % (LCD_GetMaxX() - x);        //Maksymalna wspó³rzêdna x
		uint16_t  y1=rand() % (LCD_GetMaxY() - y - 16);   //Maksymalna wspó³rzêdna y minus szerokoœæ linii tekstu i bitmapy

		LCD_MoveBMP565FromGRAM(x1, y1, 48, 48, 0, 0, 1, 0, false, 0);
	}
}

int main(void)
{
    Set48MHzClk();
    delay_init();

	LCD_Interface_Init();                //Inicjalizacja inerfejsu ³¹cz¹cego z LCD
	LCD_Init65k();
	
	LCD_Rect(0, 0, LCD_GetMaxX(), LCD_GetMaxY(), 0x0000); //Wyczyœæ ekran

	while(1)
	{
		LCD_Rect(0, 0, LCD_GetMaxX(), LCD_GetMaxY(), 0x0000);
		Bitmap_Mono_DemoHW_MCU(false);
		LCD_Rect(0, 0, LCD_GetMaxX(), LCD_GetMaxY(), 0x0000);
		Bitmap_Mono_DemoHW(false);
		LCD_Rect(0, 0, LCD_GetMaxX(), LCD_GetMaxY(), 0x0000);
		Bitmap565_Demo();
		LCD_Rect(0, 0, LCD_GetMaxX(), LCD_GetMaxY(), 0x0000);
		Bitmap565HW_Demo();
	}
}
