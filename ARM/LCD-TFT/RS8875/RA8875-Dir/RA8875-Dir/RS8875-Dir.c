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

void RA_Rect_Color(uint16_t x1, uint16_t y1)
{
	LCD_RGB565 color;
	
	LCD_SetWindow(x1, y1, x1+63, y1+63);
	LCD_SetPosition(x1, y1);
	color.blue=0;
	
	LCD_SendCmd(RA_Memory_Read_Write_Command);    // Zapis pod wskazan¹ pozycjê

	for(uint16_t y=y1; y<=y1+63; y++)
	for(uint16_t x=x1; x<=x1+63; x++)
	{
		color.green=x-x1;
		color.red=(y-y1) >> 1;
		i8080_Write_W(color.word);
	}
	LCD_CS(1);
}

int main(void)
{
    Set48MHzClk();
    delay_init();

	LCD_Interface_Init();                //Inicjalizacja inerfejsu ³¹cz¹cego z LCD
	LCD_Init65k();                       //Inicjalizacja LCD
	LCD_Rect(0, 0, LCD_GetMaxX(), LCD_GetMaxY(), 0x0000); //Wyczyœæ ekran
	
	//CountSCK_Init();                     //Inicjalizacja pomiaru szybkoœci transferu

	RA_SendCmdWithData(RA_Memory_Write_Control_Register0, (RS8875_MWCR0_Reg){.NoWriteAutoIncr=false, .Direction=RA_MWLeftRightTopDown}.byte);
	RA_Rect_Color(8, 32);

	RA_SendCmdWithData(RA_Memory_Write_Control_Register0, (RS8875_MWCR0_Reg){.NoWriteAutoIncr=false, .Direction=RA_MWRightLeftTopDown}.byte);
	RA_Rect_Color(88, 32);

	RA_SendCmdWithData(RA_Memory_Write_Control_Register0, (RS8875_MWCR0_Reg){.NoWriteAutoIncr=false, .Direction=RA_MWTopDownLeftRight}.byte);
	RA_Rect_Color(168, 32);

	RA_SendCmdWithData(RA_Memory_Write_Control_Register0, (RS8875_MWCR0_Reg){.NoWriteAutoIncr=false, .Direction=RA_MWDownTopLeftRight}.byte);
	RA_Rect_Color(248, 32);

	while(1) {}
}
