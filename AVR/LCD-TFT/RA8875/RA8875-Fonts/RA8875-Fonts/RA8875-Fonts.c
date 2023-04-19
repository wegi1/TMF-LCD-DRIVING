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

void LCD_SetText(uint16_t x, uint16_t y, const __memx char *tekst, RA_FNType FontType)
{
	RA_SendCmdWithData(RA_Memory_Write_Control_Register0, (RS8875_MWCR0_Reg){.TextModeEn=true, .MemCursorEn=true, .BlinkEn=true}.byte); //W³¹cz tryb tekstowy
	RA_SendCmdWithDataW(RA_Font_Write_Cursor_Horizontal_Position_Register0, x);	//Pozycja tekstu
	RA_SendCmdWithDataW(RA_Font_Write_Cursor_Vertical_Position_Register0, y);
	RA_SendCmdWithData(RA_Font_Control_Register0, (RS8875_FNCR0_Reg){.FNTSource=FNTCGROM_1, .ExternalFont=false, .FontType=FontType}.byte);
	LCD_SendCmd(RA_Memory_Read_Write_Command);
	while(*tekst)
	{
		i8080_Write_B(*tekst++);
		RA_WaitForWAIT();
	}
	LCD_CS(1);
	RA_SendCmdWithData(RA_Memory_Write_Control_Register0, (RS8875_MWCR0_Reg){.TextModeEn=false, .MemCursorEn=true, .BlinkEn=true}.byte); //Wróæ do trybu graficznego
}

void LCD_SetTextAttrs(_Bool transparency, _Bool align, RA_FNTScale xscale, RA_FNTScale yscale)
{
	RA_SendCmdWithData(RA_Font_Control_Register1, (RS8875_FNCR1_Reg){.VerticalScale=yscale, .HorizontalScale=xscale, .FontTransparency=transparency, .FontAlignment=align}.byte);
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
	
	LCD_SetTextAA(1 ,0, PSTR("Test fontu z antyaliasingiem 2 bitowym"), Times16AA2bit, 0b1111100000000000, 0x0000);
	LCD_SetTextAA(1, 20, PSTR("ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]"), Times16AA2bit, 0xffff, 0x0000);
	
	LCD_SetTextAA(1, 40, PSTR("Test fontu antyaliasingiem 8 bitowym monochrom."), Times16AA8bitmono, 0xffff,0x0000);
	LCD_SetTextAA(1, 60, PSTR("ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]"), Times16AA8bitmono, 0xffff,0x0000);

	LCD_SetTextAA(1, 80, PSTR("Test fontu z antyaliasingiem subpikselowym 3-3-2"), Times16AA332_array, 0xffff,0x0000);
	LCD_SetTextAA(1, 100, PSTR("ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]"), Times16AA332_array, 0xffff,0x0000);

	LCD_SetTextAA(1, 120, PSTR("Test fontu bez antyaliasingu"), Times16WA_array, 0xffff,0x0000);
	LCD_SetTextAA(1, 140, PSTR("ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]"), Times16WA_array, 0xffff,0x0000);

	LCD_SetWindow(0, 0, 799, 479);
	RA_SendCmdWithData(RA_Memory_Write_Control_Register0, (RS8875_MWCR0_Reg){.NoReadAutoIncr=0, .NoWriteAutoIncr=0, .Direction=RA_MWLeftRightTopDown}.byte);
	RA_SendCmdWithData(RA_Memory_Read_Cursor_Direction, (RS8875_MRCD_Reg){.Direction=RA_MWLeftRightTopDown}.byte);
	
	RA_BTE_SetColor(0x1f00);
	RA_BTE_SetBkgColor(0x001f);
	LCD_SetTextAttrs(false, false, FNTScale_x1, FNTScale_x1);
	LCD_SetText(20, 200, "Test fontu ISO8859-2", FNTypeISO8859_2);
	
	LCD_SetTextAttrs(false, false, FNTScale_x3, FNTScale_x3);
	LCD_SetText(20, 220, "Test fontu, skala 3x", FNTypeISO8859_2);

	LCD_SetTextAttrs(false, false, FNTScale_x2, FNTScale_x2);
	LCD_SetText(20, 270, "Test fontu ISO8859-2, skala 2x", FNTypeISO8859_2);
	
	LCD_SetTextAttrs(true, false, FNTScale_x2, FNTScale_x2);
	LCD_SetText(20, 330, "Test fontu przezroczystego", FNTypeISO8859_2);
	LCD_SetText(20, 340, "Test fontu przezroczystego", FNTypeISO8859_2);

	while(1);
 }