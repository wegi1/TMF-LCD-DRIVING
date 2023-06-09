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

void RA_DrawBitmap332(uint16_t x, uint16_t y, const uint8_t *data, uint8_t layer)
{
	uint16_t width=*(const uint16_t *)data++; data++;
	uint16_t height=*(const uint16_t *)data++; data++;
	
	RA_WaitForBTEReady();
	
	RA_BTE_SetDst(x, y, layer);
	RA_BTE_SetWidthHeight(width, height);
	
	RA_SendCmdWithData(RA_BTE_Function_Control_Register1, (RS8875_BECR1_Reg){.ROPOperation=RA_BTEOp_Write, .OpType=12}.byte);
	RA_SendCmdWithData(RA_BTE_Function_Control_Register0, (RS8875_BECR0_Reg){.DstDataType=RA_BTERectangular, .Enable=true}.byte); //Rozpocznij operacj�
	LCD_SendCmd(RA_Memory_Read_Write_Command);    // Zapis pod wskazan� pozycj�
	for(uint32_t ox=0; ox < width*height; ox++)  //Prze�li wszystkie piksele bitmapy z wyj�tkiem ostatniego
	{
		i8080_Write_B(*data++);
	}
	LCD_CS(1);
}

void RA_SetScrollWindow(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	RA_SendCmdWithDataW(RA_Horizontal_Start_Point_0_of_Scroll_Window, x1);
	RA_SendCmdWithDataW(RA_Vertical_Start_Point_0_of_Scroll_Window, y1);
	RA_SendCmdWithDataW(RA_Horizontal_End_Point_0_of_Scroll_Window, x2);
	RA_SendCmdWithDataW(RA_Vertical_End_Point_0_of_Scroll_Window, y2);
}

void RA_SetScrollOffset(uint16_t horizontal, uint16_t vertical)
{
	RA_SendCmdWithDataW(RA_Horizontal_Scroll_Offset_Register0, horizontal);
	RA_SendCmdWithDataW(RA_Vertical_Scroll_Offset_Register0, vertical);
}

int main(void)
{
    Set48MHzClk();
    delay_init();

	LCD_Interface_Init();                //Inicjalizacja inerfejsu ��cz�cego z LCD
	//LCD_Init65k();
	LCD_Init256();                       //Inicjalizacja LCD
	
	RA_SendCmdWithData(RA_Memory_Write_Control_Register1, (RS8875_MWCR1_Reg){.GrCursorEn=false, .CursorType=0, .Memory=RA_DestWriteLayer12, .Layer=0}.byte);
	LCD_Rect(0, 0, LCD_GetMaxX(), LCD_GetMaxY(), 0x0000); //Wyczy�� pierwszy ekran
	RA_SendCmdWithData(RA_Memory_Write_Control_Register1, (RS8875_MWCR1_Reg){.GrCursorEn=false, .CursorType=0, .Memory=RA_DestWriteLayer12, .Layer=1}.byte);
	LCD_Rect(0, 0, LCD_GetMaxX(), LCD_GetMaxY(), 0x0000); //Wyczy�� drugi ekran
	
	RA_SendCmdWithData(RA_Memory_Write_Control_Register0, (RS8875_MWCR0_Reg){.NoReadAutoIncr=0, .NoWriteAutoIncr=0, .Direction=RA_MWLeftRightTopDown}.byte);
	RA_SendCmdWithData(RA_Memory_Read_Cursor_Direction, (RS8875_MRCD_Reg){.Direction=RA_MWLeftRightTopDown}.byte);
	
	LCD_SetTextAttrs(false, false, FNTScale_x4, FNTScale_x4);
	RA_BTE_SetColor(0b0000100000000001);
	RA_BTE_SetBkgColor(0);
	RA_SendCmdWithData(RA_Memory_Write_Control_Register1, (RS8875_MWCR1_Reg){.GrCursorEn=false, .CursorType=0, .Memory=RA_DestWriteLayer12, .Layer=0}.byte);
	LCD_SetText(10, 200, "Przewijanie na warstwach", FNTypeISO8859_2, false);
	RA_SendCmdWithData(RA_Memory_Write_Control_Register1, (RS8875_MWCR1_Reg){.GrCursorEn=false, .CursorType=0, .Memory=RA_DestWriteLayer12, .Layer=1}.byte);
	LCD_SetText(10, 200, "z kontynuacja w buforze", FNTypeISO8859_2, false);

	RA_SendCmdWithData(RA_Layer_Transparency_Register0, (RS8875_LTPR0_Reg){.DisplayMode=RA_BTELayer1Visible, .ScrMode=RA_BTE_BufferScroll}.byte);
	RA_SetScrollWindow(0, 0, LCD_GetMaxX()-1, LCD_GetMaxY()-1);  //Wyznacz prostok�t, w kt�rym odbywa si� przewijanie
	uint16_t xofs=0;
	
	while(1)
	{
		RA_SetScrollOffset(xofs, 0);
		xofs=(xofs+1) % 1600;
		delay_ms(5);
	}
}
