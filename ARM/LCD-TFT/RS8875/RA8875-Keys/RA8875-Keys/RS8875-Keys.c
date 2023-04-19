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
#include "RA8875-Kbd.h"
#include "Fonts/Fonts.h"
#include "Icons.h"

int main(void)
{
    Set48MHzClk();
    delay_init();

	LCD_Interface_Init();                //Inicjalizacja inerfejsu ³¹cz¹cego z LCD
	LCD_Init65k();
	//LCD_Init256();                       //Inicjalizacja LCD
	
	RA_SendCmdWithData(RA_Memory_Write_Control_Register1, (RS8875_MWCR1_Reg){.GrCursorEn=false, .CursorType=0, .Memory=RA_DestWriteLayer12, .Layer=0}.byte);
	LCD_Rect(0, 0, LCD_GetMaxX(), LCD_GetMaxY(), 0x0000); //Wyczyœæ pierwszy ekran

	RA_KBDInit();
	
	char bufor[20];
	uint8_t k1, k2, k3, keyno;
	
	while(1)
	{
		keyno=RA_GetKeys(&k1, &k2, &k3);
		sprintf(bufor, "Liczba klawiszy %i", keyno);
		LCD_SetTextAA(1, 10, bufor, Times16AA332_array, 0xff00,0x0000);
		sprintf(bufor, "Klawisz 1=%03i", k1);
		LCD_SetTextAA(1, 30, bufor, Times16AA332_array, 0xff00,0x0000);
		sprintf(bufor, "Klawisz 2=%03i", k2);
		LCD_SetTextAA(1, 50, bufor, Times16AA332_array, 0xff00,0x0000);
		sprintf(bufor, "Klawisz 3=%03i", k3);
		LCD_SetTextAA(1, 70, bufor, Times16AA332_array, 0xff00,0x0000);
	}
}
