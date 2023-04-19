/*
 * bufusart.c
 *
 * Created: 2013-01-24 18:13:44
 *  Author: tmf
 */ 

#include "bufusart.h"
#include "usart.h"
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/atomic.h>
#include <stdlib.h>
#include <string.h>

CircBuffer recBuf;
CircBuffer sendBuf;

uint8_t recBufData[CB_MAXTRANS];
uint8_t SendBufData[CB_MAXTRANS];

bool cb_Add(CircBuffer *cb, uint8_t elem)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		if(cb_IsFull(cb)) return false;         //Czy jest miejsce w kolejce?
		uint8_t end = (cb->Beg + cb->Count) % CB_MAXTRANS;
		cb->elements[end] = elem;              //Dodaj transakcjê
		++cb->Count;                           //Liczba elementów w buforze
	}
	return true;      //Wszystko ok
}

uint8_t cb_Read(CircBuffer *cb)
{
	uint8_t elem;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		if(cb_IsEmpty(cb)) return 0;       //Bufor pusty, nie mo¿na zwróciæ elementu
		elem = cb->elements[cb->Beg];
		cb->Beg = (cb->Beg + 1) % CB_MAXTRANS;
		-- cb->Count;                        //Zmniejszamy liczbê elementów pozosta³ych
	}		                                 //w buforze
	return elem;
}

void USART_bufs_init()
{
	recBuf.elements=recBufData;
	sendBuf.elements=SendBufData;
}

ISR(USARTC0_DRE_vect)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		if(!(cb_IsEmpty(&sendBuf)))
		{
			uint8_t ch=cb_Read(&sendBuf);
			USARTC0_DATA=ch;
		} else USARTC0.CTRLA=USART_RXCINTLVL_LO_gc | USART_DREINTLVL_OFF_gc;  //Nie ma wiêcej znaków wiêc wy³¹cz przerwania nadajnika
	}
}

ISR(USARTC0_RXC_vect)
{
	uint8_t ch=USARTC0_DATA;
	cb_Add(&recBuf, ch);
}

_Bool USART_send_buf(const char __memx *txt)
{
	uint8_t elemsno=cb_ElementsNo(&sendBuf);
	uint8_t len=strlen_P(txt);
	if((CB_MAXTRANS-elemsno) < (len+1)) return false; //Nie ma miejsca w buforze
	 else
	 {
		 for(uint8_t index=0; index<len; index++) cb_Add(&sendBuf, txt[index]);
	 }
	USARTC0.CTRLA=USART_RXCINTLVL_LO_gc | USART_DREINTLVL_LO_gc;  //W³¹cz przerwania nadajnika
	return true;
}
