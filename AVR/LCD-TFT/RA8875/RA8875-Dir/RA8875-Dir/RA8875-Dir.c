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

volatile uint32_t counter;
uint32_t cnt;

ISR(TCE0_OVF_vect)
{
	cnt=counter;
	counter=0;
}

void CountSCK_Init()
{
	TCE0.PER=F_CPU/1024 - 1;                  //Dla 32 MHz zlicz do 31250 co da czas 1 sekundy
	TCE0.INTCTRLA=TC_OVFINTLVL_LO_gc;         //Odblokuj przerwania nadmiaru timera
	TCE0.CTRLA=TC_CLKSEL_DIV1024_gc;          //Timer odmierzaj¹cy sekundê
}

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
	SelectPLL(OSC_PLLSRC_RC2M_gc, 16);   //PLL 16x - na wyjœciu 32 MHz
	CPU_CCP=CCP_IOREG_gc;
	CLK.CTRL=CLK_SCLKSEL_PLL_gc;         //Wybierz wyjœcie PLL, czyli zegar 32 MHz

	PMIC_CTRL=PMIC_LOLVLEN_bm;           //Odblokuj przerwania niskiego poziomu
	sei();

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