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
#include <util/atomic.h>

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

void RA_BTE_SetSrc(uint16_t x, uint16_t y, uint8_t Layer)
{
	RA_SendCmdWithDataW(RA_Horizontal_Source_Point_0_of_BTE, x);
	if(Layer) y|=0x8000;   //Wybierz warstwê nr 1 - warstwa jest w tym samym rejeestrze co starsza czêœæ wsp. y
	RA_SendCmdWithDataW(RA_Vertical_Source_Point_0_of_BTE, y);
}

void RA_BTE_SetDst(uint16_t x, uint16_t y, uint8_t Layer)
{
	RA_SendCmdWithDataW(RA_Horizontal_Destination_Point_0_of_BTE, x);
	if(Layer) y|=0x8000;   //Wybierz warstwê nr 1 - warstwa jest w tym samym rejeestrze co starsza czêœæ wsp. y
	RA_SendCmdWithDataW(RA_Vertical_Destination_Point_0_of_BTE, y);
}

void RA_BTE_SetWidthHeight(uint16_t width, uint16_t height)
{
	RA_SendCmdWithDataW(RA_BTE_Width_Register0, width);
	RA_SendCmdWithDataW(RA_BTE_Height_Register0, height);
}

void RA_BTE_Move(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t width, uint16_t height, uint8_t srcLayer, uint8_t dstLayer, uint8_t direction)
{
	RA_BTE_SetSrc(x1, y1, srcLayer);
	RA_BTE_SetDst(x2, y2, dstLayer);
	RA_BTE_SetWidthHeight(width, height);
	
	if(direction) RA_SendCmdWithData(RA_BTE_Function_Control_Register1, (RS8875_BECR1_Reg){.ROPOperation=RA_BTEOp_MovePositive, .OpType=12}.byte);
		else RA_SendCmdWithData(RA_BTE_Function_Control_Register1, (RS8875_BECR1_Reg){.ROPOperation=RA_BTEOp_MoveNegative, .OpType=12}.byte);
		
	RA_SendCmdWithData(RA_BTE_Function_Control_Register0, (RS8875_BECR0_Reg){.DstDataType=RA_BTERectangular, .SrcDataType=RA_BTERectangular, .Enable=true}.byte); //Rozpocznij operacjê
}

void RA_BTE_test()
{
	while(RA_SendCmdReadData(RA_BTE_Function_Control_Register0) & (RS8875_BECR0_Reg){.Enable=1}.byte);  //Zaczekaj na koniec operacji BTE
	RA_BTE_SetDst(100, 400, 0);
	RA_BTE_SetWidthHeight(50, 50);
	
	//Prostok¹t z ROP - czerñ
	RA_SendCmdWithData(RA_BTE_Function_Control_Register1, (RS8875_BECR1_Reg){.ROPOperation=RA_BTEOp_MovePositive, .OpType=0}.byte);
	RA_SendCmdWithData(RA_BTE_Function_Control_Register0, (RS8875_BECR0_Reg){.DstDataType=RA_BTERectangular, .SrcDataType=RA_BTERectangular, .Enable=true}.byte); //Rozpocznij operacjê
	
	while(RA_SendCmdReadData(RA_BTE_Function_Control_Register0) & (RS8875_BECR0_Reg){.Enable=1}.byte);  //Zaczekaj na koniec operacji BTE
	
	//Prostok¹t z ROP - biel	
	RA_BTE_SetDst(200, 400, 0);
	RA_SendCmdWithData(RA_BTE_Function_Control_Register1, (RS8875_BECR1_Reg){.ROPOperation=RA_BTEOp_MovePositive, .OpType=15}.byte);
	RA_SendCmdWithData(RA_BTE_Function_Control_Register0, (RS8875_BECR0_Reg){.DstDataType=RA_BTERectangular, .SrcDataType=RA_BTERectangular, .Enable=true}.byte); //Rozpocznij operacjê

	while(RA_SendCmdReadData(RA_BTE_Function_Control_Register0) & (RS8875_BECR0_Reg){.Enable=1}.byte);  //Zaczekaj na koniec operacji BTE
	
	//Solid fill
	RA_BTE_SetDst(400, 400, 0);
	RA_SendCmdWithData(RA_Foreground_Color_Register0, 0);  
	RA_SendCmdWithData(RA_Foreground_Color_Register1, 0);  
	RA_SendCmdWithData(RA_Foreground_Color_Register2, 0b00011111);  
	RA_SendCmdWithData(RA_BTE_Function_Control_Register1, (RS8875_BECR1_Reg){.ROPOperation=RA_BTEOp_SolidFill, .OpType=0}.byte);
	RA_SendCmdWithData(RA_BTE_Function_Control_Register0, (RS8875_BECR0_Reg){.DstDataType=RA_BTERectangular, .SrcDataType=RA_BTERectangular, .Enable=true}.byte); //Rozpocznij operacjê
	
	while(RA_SendCmdReadData(RA_BTE_Function_Control_Register0) & (RS8875_BECR0_Reg){.Enable=1}.byte);  //Zaczekaj na koniec operacji BTE
	
	//Transparent write
	RA_BTE_SetDst(500, 400, 0);
	RA_BTE_SetWidthHeight(50, 50);
	RA_SendCmdWithData(RA_Foreground_Color_Register0, 0);
	RA_SendCmdWithData(RA_Foreground_Color_Register1, 0);
	RA_SendCmdWithData(RA_Foreground_Color_Register2, 0);
	RA_SendCmdWithData(RA_BTE_Function_Control_Register1, (RS8875_BECR1_Reg){.ROPOperation=RA_BTEOp_TransparentWrite, .OpType=12}.byte);
	RA_SendCmdWithData(RA_BTE_Function_Control_Register0, (RS8875_BECR0_Reg){.DstDataType=RA_BTERectangular, .SrcDataType=RA_BTELinear, .Enable=true}.byte); //Rozpocznij operacjê
	
	LCD_SendCmd(RA_Memory_Read_Write_Command);		//Po poleceniach zapisu przez BTE i tak musimy wys³aæ polecenie zapisu do GRAM
	for(uint16_t i=0; i<50*50; i++)
	{
		if(i & 1) i8080_Write_W(0x0);
		 else i8080_Write_W(0xffff);
	}
	LCD_CS(1);
}

int main(void)
{
	SelectPLL(OSC_PLLSRC_RC2M_gc, 16);   //PLL 16x - na wyjœciu 32 MHz
	CPU_CCP=CCP_IOREG_gc;
	CLK.CTRL=CLK_SCLKSEL_PLL_gc;         //Wybierz wyjœcie PLL, czyli zegar 32 MHz

	PMIC_CTRL=PMIC_LOLVLEN_bm;           //Odblokuj przerwania niskiego poziomu
	sei();

	LCD_Interface_Init();                //Inicjalizacja inerfejsu ³¹cz¹cego z LCD

	LCD_Init65k();                       //Inicjalizacja LCD
	LCD_Rect(0, 0, LCD_GetMaxX(), LCD_GetMaxY(), 0x001f); //Wyczyœæ ekran
	
	RA_SendCmdWithData(RA_Memory_Write_Control_Register0, (RS8875_MWCR0_Reg){.NoWriteAutoIncr=false, .Direction=RA_MWLeftRightTopDown}.byte);
	RA_Rect_Color(8, 32);

	RA_SendCmdWithData(RA_Memory_Write_Control_Register0, (RS8875_MWCR0_Reg){.NoWriteAutoIncr=false, .Direction=RA_MWRightLeftTopDown}.byte);
	RA_Rect_Color(88, 32);

	RA_SendCmdWithData(RA_Memory_Write_Control_Register0, (RS8875_MWCR0_Reg){.NoWriteAutoIncr=false, .Direction=RA_MWTopDownLeftRight}.byte);
	RA_Rect_Color(168, 32);

	RA_SendCmdWithData(RA_Memory_Write_Control_Register0, (RS8875_MWCR0_Reg){.NoWriteAutoIncr=false, .Direction=RA_MWDownTopLeftRight}.byte);
	RA_Rect_Color(248, 32);

	RA_SendCmdWithData(RA_Memory_Write_Control_Register0, (RS8875_MWCR0_Reg){.NoWriteAutoIncr=false, .Direction=RA_MWLeftRightTopDown}.byte);

	RA_BTE_Move(248, 96, 260, 198, 200, 64, 0, 0, 0);
	while(RA_SendCmdReadData(RA_BTE_Function_Control_Register0) & (RS8875_BECR0_Reg){.Enable=1}.byte);  //Zaczekaj na koniec operacji BTE
	RA_BTE_Move(0, 32, 260, 298, 200, 64, 0, 0, 1);
	
	RA_BTE_test();
	
	while(1) {}
}