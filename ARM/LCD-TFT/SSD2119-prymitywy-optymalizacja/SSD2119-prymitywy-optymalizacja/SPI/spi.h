/*
 * spi.h
 *
 * Created: 2017-01-03 10:58:25
 *  Author: tmf
 */ 


#ifndef SPI_H_
#define SPI_H_

#include <sam.h>
#include <stdint.h>
#include "../Delay/delay.h"

//Pod��czenie LCD:
// PA10 - RESET
// PA17 - CS
// PA20 - RS
//SERCOM1-SPI:
// PA19 - SCK
// PA16 - MISO
// PA18 - MOSI

#define LCD_CS   PORT_PA17       //Sygna� CS kontrolera LCD
#define LCD_RS   PORT_PA20       //Sygna� RS kontrolera LCD
#define LCD_RESET   PORT_PA10    //Sygna� RESET kontrolera LCD
#define LCD_USART SERCOM1->SPI		    //Port USART wykorzystywany do komunikacji z LCD
#define LCD_PORT  PORT->Group[0]        //Port do ktorego pod��czony jest kontroler

extern volatile uint32_t counter;

void SPI_init();    //Inicjalizacja modu�u SPI

//Cz�stotliwo�� SPI nie wi�ksza ni� 10 MHz
static __attribute__((always_inline)) inline void SPI_SPICLK10M()
{
	LCD_USART.BAUD.reg=2;  //48MHz/6 = 8 MHz
}

//Maksymalna dost�pna w MCU szybko�� SPI
static __attribute__((always_inline)) inline void SPI_SPICLKMAX()
{
	LCD_USART.BAUD.reg=0;  //MAX SPI SCK dla D21 to 12 MHz
}

//Funkcja wysy�a/odbiera bajt z SPI, przy czym czeka na zako�czenie wysy�ki
static __attribute__((always_inline)) inline uint8_t SPI_RW(uint8_t ch)
{
	LCD_USART.DATA.reg=ch;
	counter++;
	while(!(LCD_USART.INTFLAG.reg & SERCOM_SPI_INTFLAG_TXC));  //Zaczekaj na wys�anie danych
	return LCD_USART.DATA.reg;   //Odczytaj bajt i skasuj flag�
}

static __attribute__((always_inline)) inline void SPI_RW_Byte(uint8_t byte)
{
	while(!(LCD_USART.INTFLAG.reg & SERCOM_SPI_INTFLAG_DRE));  //Czy w buforze jest miejsce?
	counter++;
	LCD_USART.DATA.reg=byte;
	while(!(LCD_USART.INTFLAG.reg & SERCOM_SPI_INTFLAG_TXC));  //Zaczekaj na wys�anie danych
}

static __attribute__((always_inline)) inline void ssd2119_RESET()
{
	LCD_PORT.OUTCLR.reg=LCD_RESET;   //Aktywuj RESET
	delay_us(20);                    //Sygna� reset musi trwa� >20 us
	LCD_PORT.OUTSET.reg=LCD_RESET;   //Deaktywuj RESET
}

static __attribute__((always_inline)) inline void ssd2119_CS(uint8_t state)     //Funkcja zmieniaj�ca stan sygna�u CS
{
	if(state==0) LCD_PORT.OUTCLR.reg=LCD_CS;   //Aktywuj CS
	else LCD_PORT.OUTSET.reg=LCD_CS;           //Deaktywuj CS
}

static __attribute__((always_inline)) inline void ssd2119_RS(uint8_t state)	//Funkcja zmieniaj�ca stan sygna�u RS
{
	if(state==0) LCD_PORT.OUTCLR.reg=LCD_RS;   //Aktywuj RS
	else LCD_PORT.OUTSET.reg=LCD_RS;           //Deaktywuj RS
}

static void __attribute__((always_inline)) inline ssd2119_SendDataWord(uint16_t data)
{
	ssd2119_CS(0);
	SPI_RW(data >> 8);  //Wy�lij bardziej znacz�cy bajt danych
	ssd2119_CS(1);
	ssd2119_CS(0);
	SPI_RW(data & 0xff);  //Wy�lij mniej znacz�cy bajt danych
	ssd2119_CS(1);
}

static __attribute__((always_inline)) inline void ssd2119_SendDataByte(uint8_t data)
{
	while(!(LCD_USART.INTFLAG.reg & SERCOM_SPI_INTFLAG_DRE));  //Czy w buforze jest miejsce?
	LCD_USART.DATA.reg=data;
	counter++;
}

static __attribute__((always_inline)) inline void ssd2119_SendCmd(uint8_t cmd)
{
	ssd2119_RS(0);
	ssd2119_CS(0);
	SPI_RW_Byte(cmd);
	ssd2119_RS(1);     //Deaktywuj tryb wysy�ania polece�
}

#endif /* SPI_H_ */