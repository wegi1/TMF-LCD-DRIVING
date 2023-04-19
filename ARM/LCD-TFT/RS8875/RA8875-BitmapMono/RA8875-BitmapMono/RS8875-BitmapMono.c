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

void Bitmap_Demo()
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

void Bitmap_Mono_Demo()
{
	uint16_t x,y;
	LCD_GetBitmapSize(&x, &y, image_data_MGlass_ico);  //Rozmiar bitmapy
	uint16_t ile=3000;
	while(--ile)
	{
		uint16_t x1=rand() % (LCD_GetMaxX() - x);        //Maksymalna wspó³rzêdna x
		uint16_t y1=rand() % (LCD_GetMaxY() - y - 16);  //Maksymalna wspó³rzêdna y minus szerokoœæ linii tekstu i bitmapy
		uint16_t col=rand();
		uint16_t colbkg=rand();

		LCD_DrawBitmap_Mono(x1, y1 + 16,col, colbkg, image_data_MGlass_ico);
	}
}

void RA_BTE_SetBkgColor(uint16_t color)
{
	LCD_RGB565 col;
	col.word=color;
	
	RA_SendCmdWithData(RA_Background_Color_Register0, col.red);
	RA_SendCmdWithData(RA_Background_Color_Register1, col.green);
	RA_SendCmdWithData(RA_Background_Color_Register2, col.blue);
}

void RA_BTE_SetColor(uint16_t color)
{
	LCD_RGB565 col;
	col.word=color;
	
	RA_SendCmdWithData(RA_Foreground_Color_Register0, col.red);
	RA_SendCmdWithData(RA_Foreground_Color_Register1, col.green);
	RA_SendCmdWithData(RA_Foreground_Color_Register2, col.blue);
}

void LCD_DrawBitmap_MonoHW(uint16_t x, uint16_t y, uint16_t color, uint16_t bkgcolor, const uint8_t *data, _Bool transparency)
{
	uint16_t width=*(const uint16_t *)data++; data++;
	uint16_t height=*(const uint16_t *)data++; data++;
	
	RA_WaitForBTEReady();			//Jeœli aktualnie toczy siê jakaœ operacja to zaczekaj na jej zakoñczenie
	RA_BTE_SetBkgColor(bkgcolor);	//Ustaw kolor t³a bitmapy - niepotrzebne, jeœli korzystamy z przezroczystoœci
	RA_BTE_SetColor(color);			//Ustaw kolor pikseli
	
	RA_BTE_SetDst(x, y, 0);
	RA_BTE_SetWidthHeight(width, height);
	
	RA_BTEROPOperationCode func=RA_BTEOp_ColorExpansion;
	if(transparency) func=RA_BTEOp_ColorExpansionWithTransparency; //Wyœwietlamy z przezroczystoœci¹

	RA_SendCmdWithData(RA_BTE_Function_Control_Register1, (RS8875_BECR1_Reg){.ROPOperation=func, .OpType=15}.byte);
	RA_SendCmdWithData(RA_BTE_Function_Control_Register0, (RS8875_BECR0_Reg){.DstDataType=RA_BTERectangular, .Enable=true}.byte); //Rozpocznij operacjê

	LCD_SendCmd(RA_Memory_Read_Write_Command);    // Zapis pod wskazan¹ pozycjê
	
	uint8_t owidth=(width+7)/8;  //Oblicz liczbe bajtów w linii z zaokr¹gleniem w górê
	uint8_t app=0;
	if(owidth & 1) {app=1; owidth&=0xfe;}
	
	for(uint16_t oy=0; oy<height; oy++)
	{
		for(uint8_t ox=0; ox<owidth; ox+=2)
		{
			uint16_t wrd=((uint16_t)(*data++)) << 8;
			wrd|=((uint16_t)*data++);
			i8080_Write_W(wrd);
			RA_WaitForWAIT();			//Zaczekaj na wykonanie polecenia
		}
		if(app)
		{
			uint16_t wrd=((uint16_t)(*data++)) << 8;
			i8080_Write_W(wrd);
			RA_WaitForWAIT();			//Zaczekaj na wykonanie polecenia
		}
	}
	LCD_CS(1);
}

void Bitmap_Mono_DemoHW(_Bool transparency)
{
	uint16_t x,y;
	LCD_GetBitmapSize(&x, &y, image_data_MGlass_ico);  //Rozmiar bitmapy
	uint16_t ile=3000;
	while(--ile)
	{
		uint16_t x1=rand() % (LCD_GetMaxX() - x);        //Maksymalna wspó³rzêdna x
		uint16_t y1=rand() % (LCD_GetMaxY() - y - 16);  //Maksymalna wspó³rzêdna y minus szerokoœæ linii tekstu i bitmapy
		uint16_t col=rand();
		uint16_t colbkg=rand();

		LCD_DrawBitmap_MonoHW(x1, y1 + 16,col, colbkg, image_data_rocketbw134, transparency); //image_data_MGlass_ico);
	}
}

int main(void)
{
    Set48MHzClk();
    delay_init();

	LCD_Interface_Init();                //Inicjalizacja inerfejsu ³¹cz¹cego z LCD
	LCD_Init65k();                       //Inicjalizacja LCD
	
	LCD_Rect(0, 0, LCD_GetMaxX(), LCD_GetMaxY(), 0x0000); //Wyczyœæ ekran

	LCD_DrawBitmap_Mono(10, 10, 0x000f, 0xffff, image_data_MGlass_ico);
	
	LCD_DrawBitmap_Mono(10, 200, 0x000f, 0xffff, image_data_rocketbw134SW);

	LCD_DrawBitmap_MonoHW(80, 10, 0xf, 0xffff, image_data_MGlass_ico, false);

	LCD_DrawBitmap_MonoHW(10, 55, 0xf, 0xffff, image_data_MGlass_ico, false);
	
	LCD_DrawBitmap_MonoHW(150, 200, 0x000f, 0xffff, image_data_rocketbw134, false);
	
	while(1)
	{
		//Bitmap_Mono_Demo();  //Dla testów mo¿na zakomentowaæ lub odkomentowaæ odpowiednie funkcje
		LCD_Rect(0, 0, LCD_GetMaxX() - 1, LCD_GetMaxY() - 1, 0x0000);
		Bitmap_Mono_DemoHW(false); //Wyœwietlaj bitmapy bez przezroczystoœci
		LCD_Rect(0, 0, LCD_GetMaxX() - 1, LCD_GetMaxY() - 1, 0x0000);
		Bitmap_Mono_DemoHW(true);  //Wyœwietlaj bitmapy z przezroczystoœci¹
		//Bitmap_Demo();
	}
}
