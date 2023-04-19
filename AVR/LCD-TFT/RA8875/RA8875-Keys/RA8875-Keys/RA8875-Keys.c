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

#include "GFXDrv.h"
#include "i8080-xmega.h"
#include "RA8875.h"
#include "RA8875-Kbd.h"
#include "Fonts/Fonts.h"

_Bool OSC_wait_for_rdy(uint8_t clk)
{
	uint8_t czas=255;
	while ((!(OSC.STATUS & clk)) && (--czas)) // Czekaj na ustabilizowanie siê generatora
	_delay_ms(1);
	return czas;   //false jeœli generator nie wystartowa³, true jeœli jest ok
}

void SelectPLL(OSC_PLLSRC_t src, uint8_t mult)
{
	mult&=OSC_PLLFAC_gm;
	OSC.PLLCTRL=src | mult;              //Ustaw Ÿród³o i mno¿nik PLL
	OSC.CTRL|=OSC_PLLEN_bm;				 //W³¹cz uk³ad PLL
	OSC_wait_for_rdy(OSC_PLLRDY_bm);     //Poczekaj na ustabilizowanie siê PLL
}

int main(void)
{
	SelectPLL(OSC_PLLSRC_RC2M_gc, 16);   //PLL 16x - na wyjœciu 32 MHz
	CPU_CCP=CCP_IOREG_gc;
	CLK.CTRL=CLK_SCLKSEL_PLL_gc;         //Wybierz wyjœcie PLL, czyli zegar 32 MHzs

	PMIC_CTRL=PMIC_LOLVLEN_bm;           //Odblokuj przerwania niskiego poziomu
	sei();

	LCD_Interface_Init();                //Inicjalizacja inerfejsu ³¹cz¹cego z LCD

	LCD_Init65k();
	//LCD_Init256();                       //Inicjalizacja LCD
	
	RA_SendCmdWithData(RA_Memory_Write_Control_Register1, (RS8875_MWCR1_Reg){.GrCursorEn=false, .CursorType=0, .Memory=RA_DestWriteLayer12, .Layer=0}.byte);
	LCD_Rect(0, 0, LCD_GetMaxX(), LCD_GetMaxY(), 0x0000); //Wyczyœæ pierwszy ekran

	RA_KBDInit();
	
	char bufor[20];
	uint8_t k1, k2, k3, keyno;
	
	while(1)
	{
		keyno=RA_GetKeys(&k1, &k2, &k3);
		sprintf(bufor, "Liczba klawiszy %i", keyno);
		LCD_SetTextAA(1, 10, bufor, Times16AA332_array, 0xff00,0x0000);
		sprintf(bufor, "Klawisz 1=%03i", k1);
		LCD_SetTextAA(1, 30, bufor, Times16AA332_array, 0xff00,0x0000);
		sprintf(bufor, "Klawisz 2=%03i", k2);
		LCD_SetTextAA(1, 50, bufor, Times16AA332_array, 0xff00,0x0000);
		sprintf(bufor, "Klawisz 3=%03i", k3);
		LCD_SetTextAA(1, 70, bufor, Times16AA332_array, 0xff00,0x0000);
	}
}