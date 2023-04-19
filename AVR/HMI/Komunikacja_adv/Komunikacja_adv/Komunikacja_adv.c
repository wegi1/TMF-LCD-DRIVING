/*
 * Komunikacja_adv.c
 *
 * Created: 2017-01-22 21:44:53
 * Author : tmf
 */ 

#include <avr/io.h>
#include <string.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <util/atomic.h>
#include <stdbool.h>
#include <avr/pgmspace.h>

#include "Clk/Clk.h"
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

void HMI_WaitForTxAndResponse()
{
	while(cb_ElementsNo(&recBuf) < 4);      //Zaczekaj na odpowied� z modu�u - otrzymanie 4 bajt�w
}

uint8_t HMI_GetResponse()
{
	uint8_t res=-1;  //Brak poprawnej odpowiedzi
	
	if(cb_ElementsNo(&recBuf) >= 4)
	{
		uint8_t tmpres=cb_Read(&recBuf);
		uint8_t ff=cb_Read(&recBuf) & cb_Read(&recBuf) & cb_Read(&recBuf);
		if(ff==0xFF) res=tmpres;  //Je�li format odpowiedzi jest poprawny to j� zwr��
	}
	return res;  
}

uint8_t HMI_GetNumber(uint32_t *liczba)
{
	uint8_t res=-1;  //Brak poprawnej odpowiedzi
	
	if(cb_ElementsNo(&recBuf) >= 4)
	{
		res=cb_Read(&recBuf);   //Pobierz typ danych
		if(res == 0x71)         //Komunikat zawiera liczb�
		{
			while(cb_ElementsNo(&recBuf) < 7);  //Komunikat musi zawiera� kolejnych 7 bajt�w
			*liczba=cb_Read(&recBuf) + (cb_Read(&recBuf) << 8) + ((uint32_t)cb_Read(&recBuf) << 8) + ((uint32_t)cb_Read(&recBuf) << 8);
		}
		uint8_t ff=cb_Read(&recBuf) & cb_Read(&recBuf) & cb_Read(&recBuf);
		if(ff != 0xFF) res=-1;  //Je�li format odpowiedzi jest poprawny to j� zwr��
	}
	return res;	
}

uint8_t HMI_GetString(char *str)
{
	uint8_t res=-1;  //Brak poprawnej odpowiedzi
	
	if(cb_ElementsNo(&recBuf) >= 4)
	{
		res=cb_Read(&recBuf);  //Pobierz typ danych
		if(res == 0x70)        //Komunikat zawiera �a�cuch
		{
			uint16_t indeks=0;
			uint8_t znak, sufiks=0;
			do{
				while(cb_IsEmpty(&recBuf));  //Sprawd� czy jest kolejny znak, je�li nie to zaczekaj
				znak=cb_Read(&recBuf);
				if(znak != 0xFF) 
				{
					str[indeks++]=znak;
					sufiks=0;
				} else sufiks++;
			} while(sufiks<3);
			str[indeks]=0;   //Utw�rz poprawny �a�cuch j�zyka C
			res=0x01;
		} else
		{
			uint8_t ff=cb_Read(&recBuf) & cb_Read(&recBuf) & cb_Read(&recBuf);
			if(ff != 0xFF) res=-1;  //Je�li format odpowiedzi nie jest poprawny to zwr�� -1
		}
	}
	return res;
}

volatile static uint8_t resp;
volatile static uint32_t wynik;
volatile static uint8_t tekst[30];

int main(void)
{
	SelectPLL(OSC_PLLSRC_RC2M_gc, 16);   //PLL 16x - na wyj�ciu 32 MHz
	CPU_CCP=CCP_IOREG_gc;
	CLK.CTRL=CLK_SCLKSEL_PLL_gc;         //Wybierz wyj�cie PLL, czyli zegar 32 MHz
	
	USART_init();
	sei();
	
	USART_send_buf(PSTR("bkcmd=3\xff\xff\xff"));   //W��cz potwierdzenia wykonania przes�anych polece�
	HMI_WaitForTxAndResponse();
	resp=HMI_GetResponse();
	
	USART_send_buf(PSTR("j0.val=100\xff\xff\xff"));
	HMI_WaitForTxAndResponse();
	resp=HMI_GetResponse();

	USART_send_buf(PSTR("j1.val=50\xff\xff\xff"));
	HMI_WaitForTxAndResponse();
	resp=HMI_GetResponse();

	USART_send_buf(PSTR("value.val=80\xff\xff\xff"));
	HMI_WaitForTxAndResponse();
	resp=HMI_GetResponse();
	
	USART_send_buf(PSTR("get value.val\xff\xff\xff"));
	HMI_WaitForTxAndResponse();
	resp=HMI_GetNumber(&wynik);
	
	//USART_send_buf(PSTR("get t1.val\xff\xff\xff"));  //Nieprawid�owe polecenie - powinno zwr�ci� 0x1A
	USART_send_buf(PSTR("get t1.txt\xff\xff\xff"));
	HMI_WaitForTxAndResponse();
	resp=HMI_GetString(&tekst);
	
    while (1);
}

