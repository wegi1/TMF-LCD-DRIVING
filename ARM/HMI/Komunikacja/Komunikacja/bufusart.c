/*
 * bufusart.c
 *
 * Created: 2013-01-24 18:13:44
 *  Author: tmf
 */ 

#include "bufusart.h"
#include "usart.h"
#include "sam.h"
#include <stdlib.h>
#include <string.h>
#include "Clk/SetClk.h"

CircBuffer recBuf;
CircBuffer sendBuf;

uint8_t recBufData[CB_MAXTRANS];
uint8_t SendBufData[CB_MAXTRANS];

bool cb_Add(CircBuffer *cb, uint8_t elem)
{
	cpu_irq_disable();
	//NVIC_DisableIRQ(SERCOM1_IRQn);
	
		if(cb_IsFull(cb)) return false;         //Czy jest miejsce w kolejce?
		uint8_t end = (cb->Beg + cb->Count) % CB_MAXTRANS;
		cb->elements[end] = elem;              //Dodaj transakcjê
		++cb->Count;                           //Liczba elementów w buforze
	
	cpu_irq_enable();
	//NVIC_EnableIRQ(SERCOM1_IRQn);
	
	return true;      //Wszystko ok
}

uint8_t cb_Read(CircBuffer *cb)
{
	uint8_t elem;
	cpu_irq_disable();
	//NVIC_DisableIRQ(SERCOM1_IRQn);
	
		if(cb_IsEmpty(cb)) return 0;       //Bufor pusty, nie mo¿na zwróciæ elementu
		elem = cb->elements[cb->Beg];
		cb->Beg = (cb->Beg + 1) % CB_MAXTRANS;
		-- cb->Count;                        //Zmniejszamy liczbê elementów pozosta³ych w buforze
		
	cpu_irq_enable();
	//NVIC_EnableIRQ(SERCOM1_IRQn);
	return elem;
}

void USART_bufs_init()
{
	recBuf.elements=recBufData;
	sendBuf.elements=SendBufData;
}

void LCD_USART_IRQ_HANDLER()  //Obs³uga przewañ SERCOM1 RxC i TxC
{
	if(LCD_USART.INTFLAG.bit.DRE)
	{
		if(!(cb_IsEmpty(&sendBuf)))
		{
			uint8_t ch=cb_Read(&sendBuf);
			LCD_USART.DATA.reg=ch;
		} else LCD_USART.INTENCLR.reg=SERCOM_USART_INTENSET_DRE;  //Nie ma wiêcej znaków wiêc wy³¹cz przerwania nadajnika
	}

	if(LCD_USART.INTFLAG.bit.RXC)	
	{
		uint8_t ch=LCD_USART.DATA.reg;
		cb_Add(&recBuf, ch);
	}
}

_Bool USART_send_buf(const char *txt)
{
	uint8_t elemsno=cb_ElementsNo(&sendBuf);
	uint8_t len=strlen(txt);
	if((CB_MAXTRANS-elemsno) < (len+1)) return false; //Nie ma miejsca w buforze
	 else
	 {
		 for(uint8_t index=0; index<len; index++) cb_Add(&sendBuf, txt[index]);
	 }
	LCD_USART.INTENSET.reg=SERCOM_USART_INTENSET_DRE | SERCOM_USART_INTENSET_RXC;  //W³¹cz przerwania nadajnika i odbiornika
	return true;
}
