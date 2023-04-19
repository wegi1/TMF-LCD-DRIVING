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
#include "TWI/TWI.h"
#include "FT5x06.h"

static char buffer[30];
FT_TouchPoint_Reg Touch_XY[5];		//Tablica zawieraj¹ca dane o dotyku, max 5 punktów
uint16_t color[5]={0xFFFF, 0xFF00, 0x00FF, 0x0FF0, 0xF00F};

uint16_t FT_5x06_GetX(FT_TouchPoint_Reg XY)
{
	return XY.XPOS_LSB + (XY.XPOS_MSB << 8);
}

uint16_t FT_5x06_GetY(FT_TouchPoint_Reg XY)
{
	return XY.YPOS_LSB + (XY.YPOS_MSB << 8);
}

int main(void)
{
	SelectPLL(OSC_PLLSRC_RC2M_gc, 16);   //PLL 16x - na wyjœciu 32 MHz
	CPU_CCP=CCP_IOREG_gc;
	CLK.CTRL=CLK_SCLKSEL_PLL_gc;         //Wybierz wyjœcie PLL, czyli zegar 32 MHzs

	PMIC_CTRL=PMIC_LOLVLEN_bm;           //Odblokuj przerwania niskiego poziomu
	sei();
	
	CTP_Init();
	
	LCD_Interface_Init();                //Inicjalizacja inerfejsu ³¹cz¹cego z LCD

	LCD_Init65k();
	
	LCD_Rect(0, 0, LCD_GetMaxX(), LCD_GetMaxY(), 0x0000); //Wyczyœæ pierwszy ekran
	
	uint8_t val[2], touchno;
	
	if(CTP_ReadRegister(FT_5x06_ID_G_FIRMID, &val[0], 1))
	{
		sprintf(buffer, "Firmware ver.: %i", val[0]);
		LCD_SetTextAA(1 ,0, buffer, Times16AA2bit, 0xffff, 0x0000);
	}
	
	if(CTP_ReadRegister(FT_5x06_ID_G_LIB_VERSION_H, &val[0], 2))
	{
		sprintf(buffer, "Library ver.: %i.%i", val[1], val[0]);
		LCD_SetTextAA(1 ,20, buffer, Times16AA2bit, 0xffff, 0x0000);
	}
	
	while(1)
	{
		CTP_ReadRegister(FT_5x06_TD_STATUS, &touchno, 1);  //Odczytaj liczbê dotkniêtych punktów
		touchno&=0x0F;
		if(touchno)
		{
			CTP_ReadRegister(FT_5x06_TOUCH_XH, (uint8_t*)Touch_XY, sizeof(FT_TouchPoint_Reg)*touchno);  //Pobierz miejsca dotyku
			for(uint8_t i=0; i<touchno; i++)
			{
				if((Touch_XY[i].Type == FT_Contact) || (Touch_XY[i].Type == FT_PutDown))
				{
					LCD_SetWindow(0, 0, LCD_GetMaxX(), LCD_GetMaxY());
					RA_BTE_SetColor(color[i]);
					LCD_Circle(FT_5x06_GetX(Touch_XY[i]), FT_5x06_GetY(Touch_XY[i]), 20, true);	//Narysuj ko³o w miejscu dotyku
				}
			}
		}
	}
}