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
#define LCD_PORT  PORT->Group[0]        //Port do ktorego pod³¹czony jest kontroler
#define LCD_RX   PORT_PA17       //Sygna³ Rx
#define LCD_TX   PORT_PA18       //Sygna³ Tx

#define LCD_USART_IRQ_HANDLER SERCOM1_Handler  //Handler przerwañ wybranego portu SERCOM

#include <sam.h>
#include <stdint.h>
#include <stdbool.h>

void USART_putchar(SercomUsart * const usart, char ch);                                    //Wyœlij znak
void USART_send(SercomUsart * const usart, const char *txt);                               //Wyœlij ³añcuch w formacie NULLZ
void USART_send_F(SercomUsart *const usart, const char *txt);                              //Wyœlij ³añcuch w formacie NULLZ z FLASH
void USART_send_block(SercomUsart * const usart, const uint8_t *block, uint8_t size);      //Wyœlij blok o d³ugoœci size z pamiêci SRAM
void USART_send_block_F(SercomUsart * const usart, const uint8_t *block, uint8_t size);    //Wyœlij blok z pamiêci FLASH

static inline void waitforTx(SercomUsart *usart)            //Zaczekaj na koniec nadawania znaku
{
	while(!(usart->INTFLAG.reg & SERCOM_USART_INTFLAG_TXC));
	usart->INTFLAG.reg=SERCOM_USART_INTFLAG_TXC;
}

#endif /* USART_H_ */