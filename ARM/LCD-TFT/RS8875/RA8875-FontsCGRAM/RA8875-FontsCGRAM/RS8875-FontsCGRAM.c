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

void LCD_SetText(uint16_t x, uint16_t y, const char *tekst, RA_FNType FontType, _Bool CGRAM)
{
	RA_SendCmdWithData(RA_Memory_Write_Control_Register0, (RS8875_MWCR0_Reg){.TextModeEn=true, .MemCursorEn=true, .BlinkEn=true}.byte); //W��cz tryb tekstowy
	RA_SendCmdWithDataW(RA_Font_Write_Cursor_Horizontal_Position_Register0, x);	//Pozycja tekstu
	RA_SendCmdWithDataW(RA_Font_Write_Cursor_Vertical_Position_Register0, y);
	if(CGRAM) RA_SendCmdWithData(RA_Font_Control_Register0, (RS8875_FNCR0_Reg){.FNTSource=FNTCGRAM, .ExternalFont=true, .FontType=FontType}.byte);
	else RA_SendCmdWithData(RA_Font_Control_Register0, (RS8875_FNCR0_Reg){.FNTSource=FNTCGROM, .ExternalFont=false, .FontType=FontType}.byte);
	LCD_SendCmd(RA_Memory_Read_Write_Command);
	while(*tekst)
	{
		i8080_Write_B(*tekst++);
		RA_WaitForWAIT();
	}
	LCD_CS(1);
	RA_SendCmdWithData(RA_Memory_Write_Control_Register0, (RS8875_MWCR0_Reg){.TextModeEn=false, .MemCursorEn=true, .BlinkEn=true}.byte); //Wr�� do trybu graficznego
}

void LCD_SetTextAttrs(_Bool transparency, _Bool align, RA_FNTScale xscale, RA_FNTScale yscale)
{
	RA_SendCmdWithData(RA_Font_Control_Register1, (RS8875_FNCR1_Reg){.VerticalScale=yscale, .HorizontalScale=xscale, .FontTransparency=transparency, .FontAlignment=align}.byte);
}

void RA_SetCGRAMChar(uint8_t charno, const uint8_t desc[16])
{
	RA_SendCmdWithData(RA_Memory_Write_Control_Register1, (RS8875_MWCR1_Reg){.Memory=RA_DestWriteCGRAM}.byte);	//Zapis do CGRAM
	RA_SendCmdWithData(RA_CGRAM_Select_Register, charno);
	LCD_SendCmd(RA_Memory_Read_Write_Command);
	for(uint8_t i=0; i<16; i++) i8080_Write_B(desc[i]); //Zapisz now� matryc�
	RA_SendCmdWithData(RA_Memory_Write_Control_Register1, (RS8875_MWCR1_Reg){.Memory=RA_DestWriteLayer12}.byte); //Przywr�� zapis do GRAM
}

const uint8_t znak1[16]={0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255};  //Naprzemienne linie
const uint8_t znak2[16]={0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55};  //Szachownica


int main(void)
{
    Set48MHzClk();
    delay_init();

	LCD_Interface_Init();                //Inicjalizacja inerfejsu ��cz�cego z LCD
	LCD_Init65k();
	
	LCD_Rect(0, 0, LCD_GetMaxX(), LCD_GetMaxY(), 0x0000); //Wyczy�� ekran
	
	LCD_SetTextAA(1 ,0, "Test fontu z antyaliasingiem 2 bitowym", Times16AA2bit, 0b1111100000000000, 0x0000);
	LCD_SetTextAA(1, 20, "ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]", Times16AA2bit, 0xffff, 0x0000);
	
	LCD_SetTextAA(1, 40, "Test fontu antyaliasingiem 8 bitowym monochrom.", Times16AA8bitmono, 0xffff,0x0000);
	LCD_SetTextAA(1, 60, "ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]", Times16AA8bitmono, 0xffff,0x0000);

	LCD_SetTextAA(1, 80, "Test fontu z antyaliasingiem subpikselowym 3-3-2", Times16AA332_array, 0xffff,0x0000);
	LCD_SetTextAA(1, 100, "ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]", Times16AA332_array, 0xffff,0x0000);

	LCD_SetTextAA(1, 120, "Test fontu bez antyaliasingu", Times16WA_array, 0xffff,0x0000);
	LCD_SetTextAA(1, 140, "ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]", Times16WA_array, 0xffff,0x0000);

	LCD_SetWindow(0, 0, 799, 479);
	RA_SendCmdWithData(RA_Memory_Write_Control_Register0, (RS8875_MWCR0_Reg){.NoReadAutoIncr=0, .NoWriteAutoIncr=0, .Direction=RA_MWLeftRightTopDown}.byte);
	RA_SendCmdWithData(RA_Memory_Read_Cursor_Direction, (RS8875_MRCD_Reg){.Direction=RA_MWLeftRightTopDown}.byte);
	
	RA_BTE_SetColor(0x1f00);
	RA_BTE_SetBkgColor(0x001f);
	
	LCD_SetTextAttrs(false, false, FNTScale_x1, FNTScale_x1);
	LCD_SetText(20, 200, "Test fontu ISO8859-2", FNTypeISO8859_2, false);
	
	LCD_SetTextAttrs(false, false, FNTScale_x3, FNTScale_x3);
	LCD_SetText(20, 220, "Test fontu, skala 3x", FNTypeISO8859_2, false);

	LCD_SetTextAttrs(false, false, FNTScale_x2, FNTScale_x2);
	LCD_SetText(20, 270, "Test fontu ISO8859-2, skala 2x", FNTypeISO8859_2, false);
	
	LCD_SetTextAttrs(true, false, FNTScale_x2, FNTScale_x2);
	LCD_SetText(20, 330, "Test fontu przezroczystego", FNTypeISO8859_2, false);
	LCD_SetText(20, 340, "Test fontu przezroczystego", FNTypeISO8859_2, false);

	RA_SetCGRAMChar(1, znak1);
	RA_SetCGRAMChar(2, znak2);
	LCD_SetText(20, 380, "\1\2", FNTypeISO8859_2, true);
	
	while(1);
}
