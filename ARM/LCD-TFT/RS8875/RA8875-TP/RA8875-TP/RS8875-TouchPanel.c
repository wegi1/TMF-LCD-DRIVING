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
#include "RA8875-TP.h"
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
	Touch_Panel_Init();                  //Inicjalizacja obs³ugi TP

	char bufor[20];
	TP_Position XY;
	uint16_t touches=0;
	uint8_t prevTouch=0;
	
	while(1)
	{
		if(TouchPanel_GetPositionXY(&XY))
		{
			if(prevTouch == 0) touches++;
			prevTouch=1;
			LCD_SetTextAA(1, 30, "Pressed  ", Times16AA332_array, 0xff00,0x0000);
			sprintf(bufor, "Touches=%05d", touches);
			LCD_SetTextAA(1, 62, bufor, Times16AA332_array, 0xff00,0x0000);
			
		} else
		{
			LCD_SetTextAA(1, 30, "Released", Times16AA332_array, 0x00ff,0x0000);
			prevTouch=0;
		}
		
		sprintf(bufor, "X=%05d, Y=%05d", XY.X, XY.Y);
		LCD_SetTextAA(1, 10, bufor, Times16AA332_array, 0xffff,0x0000);
		
		delay_ms(100);  //OpóŸnienie dla pokazania rozrzutu wyników próbkowania
	}

}
