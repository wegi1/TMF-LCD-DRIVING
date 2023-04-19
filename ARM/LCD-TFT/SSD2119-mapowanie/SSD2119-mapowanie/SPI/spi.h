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

//Pod³¹czenie LCD:
// PA10 - RESET
// PA17 - CS
// PA20 - RS
//SERCOM1-SPI:
// PA19 - SCK
// PA16 - MISO
// PA18 - MOSI

#define LCD_CS   PORT_PA17       //Sygna³ CS kontrolera LCD
#define LCD_RS   PORT_PA20       //Sygna³ RS kontrolera LCD
#define LCD_RESET   PORT_PA10    //Sygna³ RESET kontrolera LCD
#define LCD_USART SERCOM1->SPI		    //Port USART wykorzystywany do komunikacji z LCD
#define LCD_PORT  PORT->Group[0]        //Port do ktorego pod³¹czony jest kontroler


void SPI_init();    //Inicjalizacja modu³u SPI

//Czêstotliwoœæ SPI nie wiêksza ni¿ 10 MHz
static inline void SPI_SPICLK10M()
{
	LCD_USART.BAUD.reg=2;  //48MHz/6 = 8 MHz
}

//Maksymalna dostêpna w MCU szybkoœæ SPI
static inline void SPI_SPICLKMAX()
{
	LCD_USART.BAUD.reg=1;  //MAX SPI SCK dla D21 to 12 MHz
}

//Funkcja wysy³a/odbiera bajt z SPI, przy czym czeka na zakoñczenie wysy³ki
static inline uint8_t SPI_RW(uint8_t ch)
{
	LCD_USART.DATA.reg=ch;
	while(!(LCD_USART.INTFLAG.reg & SERCOM_SPI_INTFLAG_TXC));  //Zaczekaj na wys³anie danych
	return LCD_USART.DATA.reg;   //Odczytaj bajt i skasuj flagê
}

#endif /* SPI_H_ */