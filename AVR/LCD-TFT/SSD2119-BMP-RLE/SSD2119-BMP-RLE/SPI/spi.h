/*
 * xmega_spi.h
 *
 * Created: 2014-02-24 21:17:04
 *  Author: tmf
 */


#ifndef XMEGA_SPI_H_
#define XMEGA_SPI_H_

#include <avr/io.h>
#include <util/delay.h>
#include "spi-defs.h"

static __attribute__((always_inline)) inline void ssd2119_RS(_Bool state)
{
	if(state) VPORT0_OUT|=LCD_RS;
	else VPORT0_OUT&=~LCD_RS;
}

static __attribute__((always_inline)) inline void ssd2119_CS(_Bool state)
{
	if(state) VPORT0_OUT|=LCD_CS;
	else VPORT0_OUT&=~LCD_CS;
}

static __attribute__((always_inline)) inline void SPI_RW_Byte(uint8_t byte)
{
	LCD_USART.STATUS=USART_TXCIF_bm;    //Skasuj flagê TXCIF
	LCD_USART.DATA=byte;
	while(!(LCD_USART.STATUS & USART_TXCIF_bm));
	LCD_USART.STATUS=USART_TXCIF_bm;
}

static inline void ssd2119_RESET()
{
	VPORT0_OUT&=~LCD_RESET;
	_delay_us(20);               //Sygna³ reset musi trwaæ >20 us
	VPORT0_OUT|=LCD_RESET;
}

static __attribute__((always_inline)) inline void ssd2119_SendDataWord(uint16_t data)
{
	ssd2119_CS(0);
	SPI_RW_Byte(data >> 8);  //Wyœlij bardziej znacz¹cy bajt danych
	ssd2119_CS(1);
	ssd2119_CS(0);
	SPI_RW_Byte(data & 0xff);  //Wyœlij mniej znacz¹cy bajt danych
	ssd2119_CS(1);
}

static __attribute__((always_inline)) inline void ssd2119_SendDataByte(uint8_t data)
{
	while(!(LCD_USART.STATUS & USART_DREIF_bm));
	LCD_USART.DATA=data;
}

static inline void ssd2119_SendCmd(uint8_t cmd)
{
	ssd2119_RS(0);
	ssd2119_CS(0);
	SPI_RW_Byte(cmd);
	ssd2119_CS(1);     //Deaktywuj kontroler
	ssd2119_RS(1);     //Deaktywuj tryb wysy³ania poleceñ
}

static inline void LCD_Interface_Init()
{
	PORTCFG_VPCTRLA=LCD_PORT;            //Zmapuj port do którego pod³¹czony jest LCD na port wirtualny
	VPORT0_OUT=LCD_CS | LCD_RS | SD_CS;  //Deaktywujemy kontrole i inne urz¹dzenia na magistrali SPI
	VPORT0_DIR=LCD_CS | LCD_RS | SD_CS | LCD_RESET | LCD_SCK | LCD_MOSI; //Ustaw odpowiednie piny jako wyjœcia

	LCD_USART.BAUDCTRLA=0;
	LCD_USART.BAUDCTRLB=0;                     //Fclk=FPER/2 - maksymalne taktowanie SPI
	LCD_USART.CTRLC=USART_CMODE_MSPI_gc;       //Tryb SPI 0
	LCD_USART.CTRLB=USART_TXEN_bm;
}

#endif /* XMEGA_SPI_H_ */