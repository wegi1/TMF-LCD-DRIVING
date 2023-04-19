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
    Set48MHzClk();
    delay_init();

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
