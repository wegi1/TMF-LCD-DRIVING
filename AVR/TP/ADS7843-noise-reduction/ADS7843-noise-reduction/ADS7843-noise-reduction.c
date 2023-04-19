/*
 * SSD2119_prymitywy_DMA.c
 *
 * Created: 2014-03-08 10:30:10
 *  Author: tmf
 */


#include <avr/io.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <util/delay.h>

#include "GFXDrv.h"
#include "spi.h"
#include "Fonts/Fonts.h"
#include "Icons.h"
#include "ADS7843/ADS7843.h"

_Bool OSC_wait_for_rdy(uint8_t clk)
{
	uint8_t czas=255;
	while ((!(OSC.STATUS & clk)) && (--czas)) // Czekaj na ustabilizowanie si� generatora
	_delay_ms(1);
	return czas;   //false je�li generator nie wystartowa�, true je�li jest ok
}

void SelectPLL(OSC_PLLSRC_t src, uint8_t mult)
{
	mult&=OSC_PLLFAC_gm;
	OSC.PLLCTRL=src | mult;              //Ustaw �r�d�o i mno�nik PLL
	OSC.CTRL|=OSC_PLLEN_bm;				 //W��cz uk�ad PLL
	OSC_wait_for_rdy(OSC_PLLRDY_bm);     //Poczekaj na ustabilizowanie si� PLL
}

int main(void)
{
	SelectPLL(OSC_PLLSRC_RC2M_gc, 16);   //PLL 16x - na wyj�ciu 32 MHz
	CPU_CCP=CCP_IOREG_gc;
	CLK.CTRL=CLK_SCLKSEL_PLL_gc;         //Wybierz wyj�cie PLL, czyli zegar 32 MHz

	LCD_Interface_Init();                //Inicjalizacja interfejsu ��cz�cego z LCD
	LCD_Init262();                       //Inicjalizacja LCD
	
	Touch_Panel_Init();                  //Inicjalizacja obs�ugi TP

	char bufor[20];
	TP_Position XY;
	uint16_t touches=0;
	uint8_t prevTouch=0;
	
	while(1)
	{
		TouchPanel_GetPositionXY(&XY);
		//XY.X=TouchPanel_GetPosition(TouchPanel_CordX);  //Mo�na u�y� alternatywnie dla sprawdzenia innych sposob�w komunikacji
		//XY.Y=TouchPanel_GetPosition(TouchPanel_CordY);
		sprintf(bufor, "X=%05d, Y=%05d", XY.X, XY.Y);
		LCD_SetTextAA(1, 10, bufor, Times16AA332_array, 0xffff00,0x000000);
		
		if((VPORT0_IN & TP_INT) == 0)
		{		
			if(prevTouch == 0) touches++;
			prevTouch=1;
			LCD_SetTextAA(1, 30, "Pressed  ", Times16AA332_array, 0xff0000,0x000000);
			sprintf(bufor, "Touches=%05d", touches);
			LCD_SetTextAA(1, 62, bufor, Times16AA332_array, 0xff0000,0x000000);
			
		} else
		{
			LCD_SetTextAA(1, 30, "Released", Times16AA332_array, 0x00ff00,0x000000);
			prevTouch=0;
		}
		_delay_ms(500);  //Op�nienie dla pokazania rozrzutu wynik�w pr�bkowania
	}
}