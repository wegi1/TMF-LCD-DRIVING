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

#ifdef FT_5x06_IRQEn
void EIC_Handler()
{
	if(EIC->INTFLAG.bit.EXTINT11)		//SprawdŸ czy zasz³o w³aœciwe przerwanie EIC
	{
		EIC->INTFLAG.reg = EIC_INTFLAG_EXTINT11;      //Skasuj flagê przerwania

		uint8_t touchno;
	
		CTP_ReadRegister(FT_5x06_TD_STATUS, &touchno, 1);  //Odczytaj liczbê dotkniêtych punktów
		touchno&=0x0F;
	
		if(touchno == 0)
		{
			RA_BTE_SetColor(color[1]);
		}
	
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

#endif

int main(void)
{
    Set48MHzClk();
    delay_init();

	CTP_Init();
	
	LCD_Interface_Init();                //Inicjalizacja inerfejsu ³¹cz¹cego z LCD

	LCD_Init65k();
	
	LCD_Rect(0, 0, LCD_GetMaxX(), LCD_GetMaxY(), 0x0000); //Wyczyœæ pierwszy ekran
	
	uint8_t val[2], touchno;
	
	if(CTP_ReadRegister(FT_5x06_ID_G_FIRMID, &val[0], 1))
	{
		sprintf(buffer, "Firmware ver.: %i", val[0]);
		LCD_SetTextAA(1, 0, buffer, Times16AA2bit, 0xffff, 0x0000);
	}
	
	if(CTP_ReadRegister(FT_5x06_ID_G_LIB_VERSION_H, &val[0], 2))
	{
		sprintf(buffer, "Library ver.: %i.%i", val[1], val[0]);
		LCD_SetTextAA(1, 20, buffer, Times16AA2bit, 0xffff, 0x0000);
	}

	while(1)
	{
/*
		CTP_ReadRegister(FT_5x06_TD_STATUS, &touchno, 1);  //Odczytaj liczbê dotkniêtych punktów
		touchno&=0x0F;
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
		
*/
	}
}
