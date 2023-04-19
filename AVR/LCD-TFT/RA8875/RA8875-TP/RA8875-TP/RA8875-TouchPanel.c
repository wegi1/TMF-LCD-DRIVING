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
#include "RA8875-TP.h"
#include "Fonts/Fonts.h"
#include "Icons.h"
#include <util/atomic.h>

int main(void)
{
	SelectPLL(OSC_PLLSRC_RC2M_gc, 16);   //PLL 16x - na wyjœciu 32 MHz
	CPU_CCP=CCP_IOREG_gc;
	CLK.CTRL=CLK_SCLKSEL_PLL_gc;         //Wybierz wyjœcie PLL, czyli zegar 32 MHzs

	PMIC_CTRL=PMIC_LOLVLEN_bm;           //Odblokuj przerwania niskiego poziomu
	sei();

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
		
		_delay_ms(100);  //OpóŸnienie dla pokazania rozrzutu wyników próbkowania
	}
}