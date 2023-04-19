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
#include "RA8875-Kbd.h"
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
/*	RA_SendCmdWithData(RA_Memory_Write_Control_Register1, (RS8875_MWCR1_Reg){.GrCursorEn=false, .CursorType=0, .Memory=RA_DestWriteLayer12, .Layer=1}.byte);
	LCD_Rect(0, 0, LCD_GetMaxX(), LCD_GetMaxY(), 0x0000); //Wyczyœæ drugi ekran
	
	LCD_SetTextAA(1 ,0, PSTR("Test fontu z antyaliasingiem 2 bitowym"), Times16AA2bit, 0b1111100000000000, 0x0000);
	LCD_SetTextAA(1, 20, PSTR("ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]"), Times16AA2bit, 0xffff, 0x0000);
	
	LCD_SetTextAA(1, 40, PSTR("Test fontu antyaliasingiem 8 bitowym monochrom."), Times16AA8bitmono, 0xffff,0x0000);
	LCD_SetTextAA(1, 60, PSTR("ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]"), Times16AA8bitmono, 0xffff,0x0000);

	LCD_SetTextAA(1, 80, PSTR("Test fontu z antyaliasingiem subpikselowym 3-3-2"), Times16AA332_array, 0xffff,0x0000);
	LCD_SetTextAA(1, 100, PSTR("ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]"), Times16AA332_array, 0xffff,0x0000);

	LCD_SetTextAA(1, 120, PSTR("Test fontu bez antyaliasingu"), Times16WA_array, 0xffff,0x0000);
	LCD_SetTextAA(1, 140, PSTR("ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]"), Times16WA_array, 0xffff,0x0000);


	RA_SendCmdWithData(RA_Memory_Write_Control_Register0, (RS8875_MWCR0_Reg){.NoReadAutoIncr=0, .NoWriteAutoIncr=0, .Direction=RA_MWLeftRightTopDown}.byte);
	RA_SendCmdWithData(RA_Memory_Read_Cursor_Direction, (RS8875_MRCD_Reg){.Direction=RA_MWLeftRightTopDown}.byte);
	*/

//	Touch_Panel_Init();                  //Inicjalizacja obs³ugi TP

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