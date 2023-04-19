/*
 * SetClk.h
 *
 * Created: 2016-12-03 19:43:33
 *  Author: tmf
 */ 


#ifndef SETCLK_H_
#define SETCLK_H_

#include <stdint.h>
#include <avr/io.h>
#include<util/delay.h>

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

void SetClkMax()	//Ustaw maksymalne taktowanie dla procesora
{
	SelectPLL(OSC_PLLSRC_RC2M_gc, 16);   //PLL 16x - na wyj�ciu 32 MHz
	CPU_CCP=CCP_IOREG_gc;
	CLK.CTRL=CLK_SCLKSEL_PLL_gc;         //Wybierz wyj�cie PLL, czyli zegar 32 MHz
}

#endif /* SETCLK_H_ */