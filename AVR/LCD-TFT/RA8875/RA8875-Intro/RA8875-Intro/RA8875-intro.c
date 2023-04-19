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

#include "Clk/Clk.h"
#include "GFXDrv.h"
#include "i8080-xmega.h"
#include "RA8875.h"
#include "Fonts/Fonts.h"
#include "Icons.h"

void LCD_RectHW(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
	LCD_SetWindow(x1, y1, x2, y2);
	RA_SendCmdWithData(RA_Background_Color_Register0, (LCD_RGB565){.word=color}.red);
	RA_SendCmdWithData(RA_Background_Color_Register1, (LCD_RGB565){.word=color}.green);
	RA_SendCmdWithData(RA_Background_Color_Register2, (LCD_RGB565){.word=color}.blue);
	RA_SendCmdWithData(RA_Memory_Clear_Control_Register, (RS8875_MCLR_Reg){.CLRArea=true, .MCLR=true}.byte); //Wyczyœæ okienko
	while(RA_SendCmdReadData(RA_Memory_Clear_Control_Register) & (RS8875_MCLR_Reg) {.MCLR=true}.byte);		//Zaczekaj na koniec operacji
}

int main(void)
{
	SelectPLL(OSC_PLLSRC_RC2M_gc, 16);   //PLL 16x - na wyjœciu 32 MHz
	CPU_CCP=CCP_IOREG_gc;
	CLK.CTRL=CLK_SCLKSEL_PLL_gc;         //Wybierz wyjœcie PLL, czyli zegar 32 MHz

	PMIC_CTRL=PMIC_LOLVLEN_bm;           //Odblokuj przerwania niskiego poziomu

	LCD_Interface_Init();                //Inicjalizacja inerfejsu ³¹cz¹cego z LCD

	LCD_Init65k();                       //Inicjalizacja LCD

	while(1)
    {
		uint16_t x1=rand() % LCD_GetMaxX();   //Maksymalna wspó³rzêdna x
		uint16_t x2=rand() % LCD_GetMaxX();
		if(x1>x2)                   //x2 nie mo¿e byæ mniejsze ni¿ x1
		{
			uint16_t x=x2;
			x2=x1;
			x1=x;
		}
		uint16_t  y1=rand() % LCD_GetMaxY();   //Maksymalna wspó³rzêdna y
		uint16_t  y2=rand() % LCD_GetMaxY();
		if(y1>y2)                   //y2 nie mo¿e byæ mniejsze ni¿ y1
		{
			uint8_t y=y2;
			y2=y1;
			y1=y;
		}
		uint16_t color=rand(); //Wybierz kolor
        LCD_Rect(x1, y1, x2, y2, color);
		//LCD_RectHW(x1, y1, x2, y2, color);
    }
}