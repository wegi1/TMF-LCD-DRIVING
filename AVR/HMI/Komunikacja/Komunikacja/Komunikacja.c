/*
 * Komunikacja.c
 *
 * Created: 2017-01-22 21:44:53
 * Author : tmf
 */ 

#include <avr/io.h>
#include "Clk/Clk.h"
#include <string.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <util/atomic.h>
#include <stdbool.h>
#include <avr/pgmspace.h>

#include "usart.h"
#include "bufusart.h"

//Makro umieszczaj¹ce zadany ³añcuch w przestrzeni adresowej __flash
#define PGM_STR(X) ((const __flash char[]) { X })

void USART_init()
{
	USART_bufs_init();
	PORTC_OUTSET=PIN3_bm;
	PORTC_DIRSET=PIN3_bm;                          //Pin TxD musi byæ wyjœciem
	USARTC0.CTRLA=USART_RXCINTLVL_LO_gc;           //Odblokuj przerwania odbiornika i nadajnika
	PMIC_CTRL|=PMIC_LOLVLEN_bm;                    //Odblokuj przerwanie niskiego poziomu
	USARTC0.CTRLB=USART_TXEN_bm | USART_RXEN_bm;   //W³¹cz nadajnik USART
	USARTC0.CTRLC=USART_CHSIZE_8BIT_gc;            //Ramka 8 bitów, bez parzystoœci, 1 bit stopu
	usart_set_baudrate(&USARTC0, 9600, F_CPU);     //Szybkoœæ transmisji
}

int main(void)
{
	SelectPLL(OSC_PLLSRC_RC2M_gc, 16);   //PLL 16x - na wyjœciu 32 MHz
	CPU_CCP=CCP_IOREG_gc;
	CLK.CTRL=CLK_SCLKSEL_PLL_gc;         //Wybierz wyjœcie PLL, czyli zegar 32 MHz
	
	USART_init();
	sei();
	
	USART_send_buf(PSTR("bkcmd=3\xff\xff\xff"));   //W³¹cz potwierdzenia wykonania przes³anych poleceñ
	
	USART_send_buf(PSTR("j0.val=100\xff\xff\xff"));

	USART_send_buf(PSTR("j1.val=50\xff\xff\xff"));

	USART_send_buf(PSTR("value.val=80\xff\xff\xff"));
	
    while (1);
}

