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

//Makro umieszczaj�ce zadany �a�cuch w przestrzeni adresowej __flash
#define PGM_STR(X) ((const __flash char[]) { X })

void USART_init()
{
	USART_bufs_init();
	PORTC_OUTSET=PIN3_bm;
	PORTC_DIRSET=PIN3_bm;                          //Pin TxD musi by� wyj�ciem
	USARTC0.CTRLA=USART_RXCINTLVL_LO_gc;           //Odblokuj przerwania odbiornika i nadajnika
	PMIC_CTRL|=PMIC_LOLVLEN_bm;                    //Odblokuj przerwanie niskiego poziomu
	USARTC0.CTRLB=USART_TXEN_bm | USART_RXEN_bm;   //W��cz nadajnik USART
	USARTC0.CTRLC=USART_CHSIZE_8BIT_gc;            //Ramka 8 bit�w, bez parzysto�ci, 1 bit stopu
	usart_set_baudrate(&USARTC0, 9600, F_CPU);     //Szybko�� transmisji
}

int main(void)
{
	SelectPLL(OSC_PLLSRC_RC2M_gc, 16);   //PLL 16x - na wyj�ciu 32 MHz
	CPU_CCP=CCP_IOREG_gc;
	CLK.CTRL=CLK_SCLKSEL_PLL_gc;         //Wybierz wyj�cie PLL, czyli zegar 32 MHz
	
	USART_init();
	sei();
	
	USART_send_buf(PSTR("bkcmd=3\xff\xff\xff"));   //W��cz potwierdzenia wykonania przes�anych polece�
	
	USART_send_buf(PSTR("j0.val=100\xff\xff\xff"));

	USART_send_buf(PSTR("j1.val=50\xff\xff\xff"));

	USART_send_buf(PSTR("value.val=80\xff\xff\xff"));
	
    while (1);
}

