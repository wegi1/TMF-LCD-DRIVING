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

unsigned short isqrt(unsigned long a) {
	unsigned long rem = 0;
	int root = 0;
	int i;

	for (i = 0; i < 16; i++) {
		root <<= 1;
		rem <<= 2;
		rem += a >> 30;
		a <<= 2;

		if (root < rem) {
			root++;
			rem -= root;
			root++;
		}
	}

	return (unsigned short) (root >> 1);
}

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
	uint8_t GestID;						//Kod wykonanego gestu
	uint16_t xmiddle = 0, ymiddle = 0, radius = 0;
	
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
		CTP_ReadRegister(FT_5x06_TD_STATUS, &touchno, 1);  //Odczytaj liczbê dotkniêtych punktów
		touchno&=0x0F;
		if(touchno)
		{
			CTP_ReadRegister(FT_5x06_GEST_ID, &GestID, sizeof(GestID));			//Pobierz kod wykonanego gestu
			switch(GestID)
			{
				case FT_MoveUp:		LCD_SetTextAA(1, 40, "Gest: Ruch do gory ", Times16AA2bit, 0xffff, 0x0000); break;
				case FT_MoveLeft:	LCD_SetTextAA(1, 40, "Gest: Ruch w lewo  ", Times16AA2bit, 0xffff, 0x0000); break;
				case FT_MoveDown:	LCD_SetTextAA(1, 40, "Gest: Ruch w dol   ", Times16AA2bit, 0xffff, 0x0000); break;
				case FT_MoveRight:	LCD_SetTextAA(1, 40, "Gest: Ruch w prawo ", Times16AA2bit, 0xffff, 0x0000); break;
				case FT_ZoomOut:	LCD_SetTextAA(1, 40, "Gest: Pomniejszenie ", Times16AA2bit, 0xffff, 0x0000); break;
				case FT_ZoomIn:		LCD_SetTextAA(1, 40, "Gest: Powiekszenie  ", Times16AA2bit, 0xffff, 0x0000); break;
				case FT_NoGesture:	break; //LCD_SetTextAA(1, 30, "Gest: Brak gestu   ", Times16AA2bit, 0xffff, 0x0000); break;
			}
				
			CTP_ReadRegister(FT_5x06_TOUCH_XH, (uint8_t*)Touch_XY, sizeof(FT_TouchPoint_Reg)*touchno);  //Pobierz miejsca dotyku
			
			if((GestID == FT_ZoomIn) || (GestID == FT_ZoomOut))
			{
				RA_BTE_SetColor(0x0000);
				LCD_Circle(xmiddle, ymiddle, radius, true);
				xmiddle=(FT_5x06_GetX(Touch_XY[0]) + FT_5x06_GetX(Touch_XY[1]))/2;  //Policz wsp. œrodka okrêgu
				ymiddle=(FT_5x06_GetY(Touch_XY[0]) + FT_5x06_GetY(Touch_XY[1]))/2;
				radius=isqrt((FT_5x06_GetX(Touch_XY[0])-xmiddle)*(FT_5x06_GetX(Touch_XY[0])-xmiddle) + (FT_5x06_GetY(Touch_XY[0])-ymiddle)*(FT_5x06_GetY(Touch_XY[0])-ymiddle));
				RA_WaitForWAIT();	//Przed dostêpem do rejstrów musimy zaczekaæ na koniec poprzedniej operacji
				RA_BTE_SetColor(color[1]);
				LCD_Circle(xmiddle, ymiddle, radius, true);
				RA_WaitForWAIT();
			} /*else
				for(uint8_t i=0; i<touchno; i++)
				{
					if((Touch_XY[i].Type == FT_Contact) || (Touch_XY[i].Type == FT_PutDown))
					{
						LCD_SetWindow(0, 0, LCD_GetMaxX(), LCD_GetMaxY());
						RA_BTE_SetColor(color[i]);
						LCD_Circle(FT_5x06_GetX(Touch_XY[i]), FT_5x06_GetY(Touch_XY[i]), 20, true);	//Narysuj ko³o w miejscu dotyku
					}
				}*/
		}
	}	
}
