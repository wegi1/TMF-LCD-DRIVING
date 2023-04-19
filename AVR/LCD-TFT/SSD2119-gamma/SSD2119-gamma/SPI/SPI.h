/*
 * SPI.h
 *
 * Created: 2017-02-04 21:38:59
 *  Author: tmf
 */ 


#ifndef SPI_H_
#define SPI_H_

#include <avr/io.h>
#include <util/delay.h>
#include "spi-defs.h"

static inline void ssd2119_RS(_Bool state)
{
	if(state) LCD_PORT.OUTSET=LCD_RS;
	else LCD_PORT.OUTCLR=LCD_RS;
}

static inline void ssd2119_CS(_Bool state)
{
	if(state) LCD_PORT.OUTSET=LCD_CS;
	else LCD_PORT.OUTCLR=LCD_CS;
}

uint8_t SPI_RW_Byte(uint8_t byte)
{
	LCD_USART.STATUS=USART_TXCIF_bm;
	LCD_USART.DATA=byte;
	while(!(LCD_USART.STATUS & USART_TXCIF_bm));   //Zaczekaj na koniec nadawania
	LCD_USART.STATUS=USART_TXCIF_bm;               //Potrzebne tylko przy nadawaniu poleceñ
	return LCD_USART.DATA;
}

static inline void ssd2119_RESET()
{
	LCD_PORT.OUTCLR=LCD_RESET;
	_delay_us(20);               //Sygna³ reset musi trwaæ >20 us
	LCD_PORT.OUTSET=LCD_RESET;
}

void ssd2119_SendDataWord(uint16_t data)
{
	ssd2119_CS(0);
	SPI_RW_Byte(data >> 8);  //Wyœlij bardziej znacz¹cy bajt danych
	ssd2119_CS(1);
	ssd2119_CS(0);
	SPI_RW_Byte(data & 0xff);  //Wyœlij mniej znacz¹cy bajt danych
	ssd2119_CS(1);
}

static inline void ssd2119_SendDataByte(uint8_t data)
{
	while(!(LCD_USART.STATUS & USART_DREIF_bm));
	LCD_USART.DATA=data;
}

void ssd2119_SendCmd(uint8_t cmd)
{
	ssd2119_RS(0);                                //dla synchronizacji nadawania z sygna³em RS
	ssd2119_CS(0);
	SPI_RW_Byte(cmd);
	ssd2119_CS(1);     //Deaktywuj kontroler
	ssd2119_RS(1);     //Deaktywuj tryb wysy³ania poleceñ
}

void ssd2119_SendCmdWithData(uint8_t cmd, uint16_t data)
{
	ssd2119_SendCmd(cmd);
	ssd2119_SendDataWord(data);
}

void LCD_Interface_Init()
{
	LCD_PORT.OUTCLR=LCD_RESET;
	LCD_PORT.OUTSET=LCD_CS | LCD_RS | SD_CS;  //Deaktywujemy kontrole i inne urz¹dzenia na magistrali SPI
	LCD_PORT.DIRSET=LCD_CS | LCD_RS | SD_CS | LCD_RESET | LCD_SCK | LCD_MOSI; //Ustaw odpowiednie piny jako wyjœcia

	LCD_USART.BAUDCTRLA=0;
	LCD_USART.BAUDCTRLB=0;                     //Fclk=FPER/2 - maksymalne taktowanie SPI
	LCD_USART.CTRLC=USART_CMODE_MSPI_gc;       //Tryb SPI 0
	LCD_USART.CTRLB=USART_TXEN_bm | USART_RXEN_bm;
}


#endif /* SPI_H_ */