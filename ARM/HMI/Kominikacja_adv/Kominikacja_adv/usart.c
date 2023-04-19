/*
 * usart.c
 *
 * Created: 2013-01-17 21:39:25
 *  Author: tmf
 */ 

#include "usart.h"

void USART_putchar(SercomUsart * const usart, char ch)
{
	while(!(usart->INTFLAG.reg & SERCOM_USART_INTFLAG_DRE));
	usart->DATA.reg=ch;
}

void USART_send(SercomUsart * const usart, const char *txt)
{
	while(*txt)
	{
		USART_putchar(usart, *txt);
		++txt;
	}
}

void USART_send_F(SercomUsart * const usart, const char *txt)
{
	char tmp;
	while((tmp=*txt))
	{
		USART_putchar(usart, tmp);
		++txt;
	}
}

void USART_send_block(SercomUsart * const usart, const uint8_t *block, uint8_t size)
{
	while(size--)
	{
		USART_putchar(usart, *block);
		++block;
	}
}

void USART_send_block_F(SercomUsart * const usart, const uint8_t *block, uint8_t size)
{
	while(size--)
	{
		USART_putchar(usart, *block);
		++block;
	}
}
