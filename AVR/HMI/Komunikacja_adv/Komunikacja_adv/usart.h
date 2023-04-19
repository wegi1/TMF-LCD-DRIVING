/*
 * usart.h
 *
 * Created: 2013-01-17 21:37:01
 *  Author: tmf
 */ 


#ifndef USART_H_
#define USART_H_

#include <avr/io.h>
#include <stdint.h>
#include <stdbool.h>

bool usart_set_baudrate(USART_t * const usart, uint32_t baud, uint32_t cpu_hz);        //Ustaw pr�dko�� transmisji USART
void USART_putchar(USART_t * const usart, char ch);                                    //Wy�lij znak
void USART_send(USART_t * const usart, const char *txt);                               //Wy�lij �a�cuch w formacie NULLZ
void USART_send_F(USART_t *const usart, const char *txt);                              //Wy�lij �a�cuch w formacie NULLZ z FLASH
void USART_send_block(USART_t * const usart, const uint8_t *block, uint8_t size);      //Wy�lij blok o d�ugo�ci size z pami�ci SRAM
void USART_send_block_F(USART_t * const usart, const uint8_t *block, uint8_t size);    //Wy�lij blok z pami�ci FLASH

static inline void waitforTx(USART_t *usart)            //Zaczekaj na koniec nadawania znaku
{
	while(!(usart->STATUS & USART_TXCIF_bm));
	usart->STATUS=USART_TXCIF_bm;
}

#endif /* USART_H_ */