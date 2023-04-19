/*
 * Created: 2014-03-08 10:30:10
 *  Author: tmf
 */


#include <avr/io.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#include "Clk/Clk.h"
#include "GFXDrv.h"
#include "i8080-xmega.h"
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
		uint16_t x1=rand() % (LCD_GetMaxX() - x);        //Maksymalna wsp�rz�dna x
		uint16_t  y1=rand() % (LCD_GetMaxY() - y - 16);   //Maksymalna wsp�rz�dna y minus szeroko�� linii tekstu i bitmapy

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
		uint16_t x1=rand() % (LCD_GetMaxX() - x);        //Maksymalna wsp�rz�dna x
		uint16_t y1=rand() % (LCD_GetMaxY() - y - 16);  //Maksymalna wsp�rz�dna y minus szeroko�� linii tekstu i bitmapy
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

uint16_t LCD_StoreBMPMonoInGRAM(uint16_t x, uint16_t y, const uint8_t __flash *data, uint8_t Layer)
{
	uint16_t width=*(const uint16_t __flash *)data++; data++;
	uint16_t height=*(const uint16_t __flash *)data++; data++;
	
	RA_WaitForBTEReady();			 //Je�li aktualnie toczy si� jaka� operacja to zaczekaj na jej zako�czenie
	RA_BTE_SetDst(x, y, Layer);		 //Zapisujemy liniowy obszar na pocz�tku nieu�ywanej warstwy nr 1

	uint8_t owidth=(width+7)/8;  //Oblicz liczbe bajt�w w linii z zaokr�gleniem w g�r�

	RA_BTE_SetWidthHeight(owidth + 1, height);	 //Liczba bajt�w zaj�ta przez bitmap�
	RA_SendCmdWithData(RA_BTE_Function_Control_Register1, (RS8875_BECR1_Reg){.ROPOperation=RA_BTEOp_Write, .OpType=12}.byte);		//Zapis z MCU do liniowego bloku
	RA_SendCmdWithData(RA_BTE_Function_Control_Register0, (RS8875_BECR0_Reg){.DstDataType=RA_BTELinear, .SrcDataType=RA_BTELinear, .Enable=true}.byte); //Rozpocznij operacj�
	LCD_SendCmd(RA_Memory_Read_Write_Command);    // Zapis pod wskazan� pozycj�

	for(uint16_t oy=0; oy<height; oy++)
	{
		for(uint8_t ox=0; ox<owidth; ox++)
		{
			i8080_Write_B(*data++);
			RA_WaitForWAIT();			//Zaczekaj na wykonanie polecenia
		}

		i8080_Write_B(0);			//Ka�da linia ko�czy si� dodatkowym bajtem, kt�ry jest ignorowany przez BTE
		RA_WaitForWAIT();			//Zaczekaj na wykonanie polecenia
	}
	
	return (owidth+1)*height;       //Zwr�� liczb� wykorzystanych bajt�w
}

void LCD_MoveBMPMonoFromGRAM(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t fromx, uint16_t fromy, uint16_t color, uint16_t bkgcolor, uint8_t FromLayer, uint8_t ToLayer, _Bool transparency)
{
	RA_WaitForBTEReady();		    //Je�li aktualnie toczy si� jaka� operacja to zaczekaj na jej zako�czenie
	RA_BTE_SetBkgColor(bkgcolor);	//Ustaw kolor t�a bitmapy - niepotrzebne, je�li korzystamy z przezroczysto�ci
	RA_BTE_SetColor(color);			//Ustaw kolor pikseli
	
	RA_BTE_SetSrc(fromx, fromy, FromLayer);	
	RA_BTE_SetDst(x, y, ToLayer);
	RA_BTE_SetWidthHeight(width, height);
	
	RA_BTEROPOperationCode func=RA_BTEOp_MoveWithColorExpansion;
	if(transparency) func=RA_BTEOp_MoveWithColorExpansionAndTransparency; //Wy�wietlamy z przezroczysto�ci�
	RA_SendCmdWithData(RA_BTE_Function_Control_Register1, (RS8875_BECR1_Reg){.ROPOperation=func, .OpType=7}.byte);
	RA_SendCmdWithData(RA_BTE_Function_Control_Register0, (RS8875_BECR0_Reg){.DstDataType=RA_BTERectangular, .SrcDataType=RA_BTELinear, .Enable=true}.byte); //Rozpocznij operacj�
}

void LCD_DrawBitmap_MonoHWLinear(uint16_t x, uint16_t y, uint16_t color, uint16_t bkgcolor, const uint8_t __flash *data, uint8_t transparency)
{
	uint16_t bx, by;
	
	LCD_StoreBMPMonoInGRAM(0, 0, data, 1);
	LCD_GetBitmapSize(&bx, &by, data);
	LCD_MoveBMPMonoFromGRAM(x, y, bx, by, 0, 0, color, bkgcolor, 1, 0, transparency);
}

void Bitmap_Mono_DemoHW(_Bool transparency)
{
	uint16_t x,y;
	LCD_GetBitmapSize(&x, &y, image_data_rocketbw134);  //Rozmiar bitmapy
	uint16_t ile=3000;
	while(--ile)
	{
		uint16_t x1=rand() % (LCD_GetMaxX() - x);        //Maksymalna wsp�rz�dna x
		uint16_t y1=rand() % (LCD_GetMaxY() - y - 16);  //Maksymalna wsp�rz�dna y minus szeroko�� linii tekstu i bitmapy
		uint16_t col=rand();
		uint16_t colbkg=rand();

		LCD_MoveBMPMonoFromGRAM(x1, y1, 134, 136, 0, 0, col, colbkg, 1, 0, transparency);
	}
}

void Bitmap_Mono_DemoHW_MCU(_Bool transparency)
{
	uint16_t x,y;
	LCD_GetBitmapSize(&x, &y, image_data_rocketbw134);  //Rozmiar bitmapy
	uint16_t ile=3000;
	while(--ile)
	{
		uint16_t x1=rand() % (LCD_GetMaxX() - x);        //Maksymalna wsp�rz�dna x
		uint16_t y1=rand() % (LCD_GetMaxY() - y - 16);  //Maksymalna wsp�rz�dna y minus szeroko�� linii tekstu i bitmapy
		uint16_t col=rand();
		uint16_t colbkg=rand();

		LCD_DrawBitmap_MonoHWLinear(x1, y1, col, colbkg, image_data_rocketbw134, transparency);
	}
}

int main(void)
{
	SelectPLL(OSC_PLLSRC_RC2M_gc, 16);   //PLL 16x - na wyj�ciu 32 MHz
	CPU_CCP=CCP_IOREG_gc;
	CLK.CTRL=CLK_SCLKSEL_PLL_gc;         //Wybierz wyj�cie PLL, czyli zegar 32 MHz

	PMIC_CTRL=PMIC_LOLVLEN_bm;           //Odblokuj przerwania niskiego poziomu
	sei();

	LCD_Interface_Init();                //Inicjalizacja inerfejsu ��cz�cego z LCD

	LCD_Init256();                       //Inicjalizacja LCD
	
	LCD_Rect(0, 0, LCD_GetMaxX(), LCD_GetMaxY(), 0x0000); //Wyczy�� ekran
	
//	LCD_DrawBitmap_MonoHWLinear(10, 10, 0xff00, 0x00ff, image_data_rocketbw134, false);
//	LCD_DrawBitmap_MonoHWLinear(10, 100, 0xff00, 0x00ff, image_data_rocketbw134, true);
//	LCD_DrawBitmap_MonoHWLinear(10, 200, 0xff00, 0x00ff, image_data_MGlass_ico, false);

	LCD_StoreBMPMonoInGRAM(0, 0, image_data_rocketbw134, 1);	//Zapisz w GRAM informacj� o obrazie
//	LCD_StoreBMPMonoInGRAM(0, 50, image_data_MGlass_ico, 1);
	
	while(1)
    {
		LCD_Rect(0, 0, LCD_GetMaxX(), LCD_GetMaxY(), 0x0000);
		Bitmap_Mono_DemoHW_MCU(false);
		LCD_Rect(0, 0, LCD_GetMaxX(), LCD_GetMaxY(), 0x0000);
		Bitmap_Mono_DemoHW(false); //Wy�wietlaj bitmapy bez przezroczysto�ci
		LCD_Rect(0, 0, LCD_GetMaxX(), LCD_GetMaxY(), 0x0000);
		Bitmap_Mono_DemoHW(true);
    }
}