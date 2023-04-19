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


void SPI_init();    //Inicjalizacja modu�u SPI

//Cz�stotliwo�� SPI nie wi�ksza ni� 10 MHz
static inline void SPI_SPICLK10M()
{
	LCD_USART.BAUD.reg=2;  //48MHz/6 = 8 MHz
}

//Maksymalna dost�pna w MCU szybko�� SPI
static inline void SPI_SPICLKMAX()
{
	LCD_USART.BAUD.reg=1;  //MAX SPI SCK dla D21 to 12 MHz
}

//Funkcja wysy�a/odbiera bajt z SPI, przy czym czeka na zako�czenie wysy�ki
static inline uint8_t SPI_RW(uint8_t ch)
{
	LCD_USART.DATA.reg=ch;
	while(!(LCD_USART.INTFLAG.reg & SERCOM_SPI_INTFLAG_TXC));  //Zaczekaj na wys�anie danych
	return LCD_USART.DATA.reg;   //Odczytaj bajt i skasuj flag�
}

#endif /* SPI_H_ */