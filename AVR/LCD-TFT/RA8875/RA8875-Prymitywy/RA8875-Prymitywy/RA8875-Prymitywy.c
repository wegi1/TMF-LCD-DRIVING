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

void Line_Demo()
{
	uint16_t ile=30000;
	while(--ile)
	{
		uint16_t x1=rand() % LCD_GetMaxX();   //Maksymalna wspó³rzêdna x
		uint16_t y1=rand() % LCD_GetMaxY();   //Maksymalna wspó³rzêdna y minus szerokoœæ linii tekstu i bitmapy
		uint16_t x2=rand() % LCD_GetMaxX();
		uint16_t y2=rand() % LCD_GetMaxY();
		
		RA_WaitForWAIT();
		RA_BTE_SetColor(rand());
		LCD_LineTo(x1, y1, x2, y2);
	}
}

void Circle_Demo()
{
	uint16_t ile=1000;
	while(--ile)
	{
		uint16_t x1=rand() % LCD_GetMaxX();   //Maksymalna wspó³rzêdna x
		uint16_t y1=rand() % LCD_GetMaxY();   //Maksymalna wspó³rzêdna y minus szerokoœæ linii tekstu i bitmapy
		uint8_t radius=rand() % 256;
		
		RA_WaitForWAIT();
		RA_BTE_SetColor(rand());
		LCD_Circle(x1, y1, radius, false);
	}
}

void Triangle_Demo()
{
	uint16_t ile=15000;
	while(--ile)
	{
		uint16_t x1=rand() % LCD_GetMaxX();   //Maksymalna wspó³rzêdna x
		uint16_t y1=rand() % LCD_GetMaxY();   //Maksymalna wspó³rzêdna y minus szerokoœæ linii tekstu i bitmapy
		uint16_t x2=rand() % LCD_GetMaxX();
		uint16_t y2=rand() % LCD_GetMaxY();
		uint16_t x3=rand() % LCD_GetMaxX();
		uint16_t y3=rand() % LCD_GetMaxY();
		
		RA_WaitForWAIT();
		RA_BTE_SetColor(rand());
		LCD_DrawTriangle(x1, y1, x2, y2, x3, y3, true);
	}	
}

void RoundRect_Demo()
{
	uint16_t ile=5000;
	while(--ile)
	{
		uint16_t x1=rand() % LCD_GetMaxX();   //Maksymalna wspó³rzêdna x
		uint16_t y1=rand() % LCD_GetMaxY();   //Maksymalna wspó³rzêdna y minus szerokoœæ linii tekstu i bitmapy
		uint16_t x2=rand() % LCD_GetMaxX();
		uint16_t y2=rand() % LCD_GetMaxY();

		RA_WaitForWAIT();
		RA_BTE_SetColor(rand());
		LCD_DrawRoundedRect(x1, y1, x2, y2, 10, 10, true);
	}
}

int main(void)
{
	SelectPLL(OSC_PLLSRC_RC2M_gc, 16);   //PLL 16x - na wyjœciu 32 MHz
	CPU_CCP=CCP_IOREG_gc;
	CLK.CTRL=CLK_SCLKSEL_PLL_gc;         //Wybierz wyjœcie PLL, czyli zegar 32 MHz

	PMIC_CTRL=PMIC_LOLVLEN_bm;           //Odblokuj przerwania niskiego poziomu
	sei();

	LCD_Interface_Init();                //Inicjalizacja inerfejsu ³¹cz¹cego z LCD

	LCD_Init65k();
	
	LCD_Rect(0, 0, LCD_GetMaxX(), LCD_GetMaxY(), 0x0000); //Wyczyœæ ekran
	
	while(1)
    {
		LCD_Rect(0, 0, LCD_GetMaxX(), LCD_GetMaxY(), 0x0000); //Wyczyœæ ekran
		Line_Demo();
		LCD_Rect(0, 0, LCD_GetMaxX(), LCD_GetMaxY(), 0x0000); //Wyczyœæ ekran
		Circle_Demo();
		LCD_Rect(0, 0, LCD_GetMaxX(), LCD_GetMaxY(), 0x0000); //Wyczyœæ ekran
		Triangle_Demo();
		LCD_Rect(0, 0, LCD_GetMaxX(), LCD_GetMaxY(), 0x0000); //Wyczyœæ ekran
		RoundRect_Demo();
    }
}