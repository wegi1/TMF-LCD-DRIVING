/*
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
#include "RingBuffer.h"

static char buffer[30];
uint16_t color[5]={0xFFFF, 0xFF00, 0x00FF, 0x0FF0, 0xF00F};
CircBuffer TouchBuffer;		//Bufor na dane o punktach dotyku

#ifdef FT_5x06_IRQEn
void EIC_Handler()
{
	if(EIC->INTFLAG.bit.EXTINT11)		//Sprawd� czy zasz�o w�a�ciwe przerwanie EIC
	{
		EIC->INTFLAG.reg = EIC_INTFLAG_EXTINT11;      //Skasuj flag� przerwania

		uint8_t touchno;
		FT_TouchPoint_Reg Touch_XY[5];		//Tablica zawieraj�ca dane o dotyku, max 5 punkt�w
	
		CTP_ReadRegister(FT_5x06_TD_STATUS, &touchno, 1);  //Odczytaj liczb� dotkni�tych punkt�w
		touchno&=0x0F;
	
		if (touchno) CTP_ReadRegister(FT_5x06_TOUCH_XH, (uint8_t*)Touch_XY, sizeof(FT_TouchPoint_Reg)*touchno);  //Pobierz miejsca dotyku
	
		for(uint8_t i=0; i<touchno; i++)
		{
			cb_Add(&TouchBuffer, Touch_XY[i]);
		}
	}
}

#endif

int main(void)
{
    Set48MHzClk();
    delay_init();

	CTP_Init();
	
	LCD_Interface_Init();                //Inicjalizacja inerfejsu ��cz�cego z LCD

	LCD_Init65k();
	
	LCD_Rect(0, 0, LCD_GetMaxX(), LCD_GetMaxY(), 0x0000); //Wyczy�� pierwszy ekran
	
	uint8_t val[2];
	
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
		while(!cb_IsEmpty(&TouchBuffer))
		{
			FT_TouchPoint_Reg Touch_XY;
			cb_Read(&TouchBuffer, &Touch_XY);
			if((Touch_XY.Type == FT_Contact) || (Touch_XY.Type == FT_PutDown))
			{
				LCD_SetWindow(0, 0, LCD_GetMaxX(), LCD_GetMaxY());
				RA_BTE_SetColor(0x00FF);
				LCD_Circle(FT_5x06_GetX(Touch_XY), FT_5x06_GetY(Touch_XY), 20, true);	//Narysuj ko�o w miejscu dotyku
			}
		}
		delay_ms(500);	//Tylko dla uwidocznienia op�nie� wprowadzanych przez aplikacj�
	}
}
