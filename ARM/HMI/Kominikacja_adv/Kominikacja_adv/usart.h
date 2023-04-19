/*
 * usart.h
 *
 * Created: 2013-01-17 21:37:01
 *  Author: tmf
 */ 


#ifndef USART_H_
#define USART_H_

//SERCOM1-USART:
// PA16 - Rx
// PA18 - Tx

#define LCD_USART SERCOM1->USART	    //Port USART wykorzystywany do komunikacji z LCD
#define LCD_PORT  PORT->Group[0]        //Port do ktorego pod��czony jest kontroler
#define LCD_RX   PORT_PA17       //Sygna� Rx
#define LCD_TX   PORT_PA18       //Sygna� Tx

#define LCD_USART_IRQ_HANDLER SERCOM1_Handler  //Handler przerwa� wybranego portu SERCOM

#include <sam.h>
#include <stdint.h>
#include <stdbool.h>

void USART_putchar(SercomUsart * const usart, char ch);                                    //Wy�lij znak
void USART_send(SercomUsart * const usart, const char *txt);                               //Wy�lij �a�cuch w formacie NULLZ
void USART_send_F(SercomUsart *const usart, const char *txt);                              //Wy�lij �a�cuch w formacie NULLZ z FLASH
void USART_send_block(SercomUsart * const usart, const uint8_t *block, uint8_t size);      //Wy�lij blok o d�ugo�ci size z pami�ci SRAM
void USART_send_block_F(SercomUsart * const usart, const uint8_t *block, uint8_t size);    //Wy�lij blok z pami�ci FLASH

static inline void waitforTx(SercomUsart *usart)            //Zaczekaj na koniec nadawania znaku
{
	while(!(usart->INTFLAG.reg & SERCOM_USART_INTFLAG_TXC));
	usart->INTFLAG.reg=SERCOM_USART_INTFLAG_TXC;
}

#endif /* USART_H_ */