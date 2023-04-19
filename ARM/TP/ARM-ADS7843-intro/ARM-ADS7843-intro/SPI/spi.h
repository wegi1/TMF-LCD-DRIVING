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

//Pod³¹czenie LCD:
// PA10 - RESET
// PA17 - CS
// PA20 - RS
//SERCOM1-SPI:
// PA19 - SCK
// PA16 - MISO
// PA18 - MOSI
//Pod³¹czenie TC:
// PC5 - INT z uk³adu TC
// PC0 - TPCS

#define TP_CS       PORT_PA06       //Sygna³ CS kontrolera TP
#define TP_INT      PORT_PA07       //Sygna³ PENIRQ z kontrolera TP
#define LCD_CS      PORT_PA17       //Sygna³ CS kontrolera LCD
#define LCD_RS      PORT_PA20       //Sygna³ RS kontrolera LCD
#define LCD_RESET   PORT_PA10    //Sygna³ RESET kontrolera LCD
#define LCD_USART SERCOM1->SPI		    //Port USART wykorzystywany do komunikacji z LCD
#define LCD_PORT  PORT->Group[0]        //Port do ktorego pod³¹czony jest kontroler

void SPI_init();    //Inicjalizacja modu³u SPI

static __attribute__((always_inline)) inline void SPI_Disable()
{
	LCD_USART.CTRLA.reg&=~SERCOM_SPI_CTRLA_ENABLE; //Zablokuj SERCOM
}

static __attribute__((always_inline)) inline void SPI_Enable()
{
	LCD_USART.CTRLA.reg|=SERCOM_SPI_CTRLA_ENABLE; //Odblokuj SERCOM
}

static __attribute__((always_inline)) inline _Bool SPI_isEnabled()
{
	return LCD_USART.CTRLA.reg & SERCOM_SPI_CTRLA_ENABLE;
}

//Czêstotliwoœæ SPI nie wiêksza ni¿ 10 MHz
static __attribute__((always_inline)) inline void SPI_SPICLK10M()
{
	_Bool spien;
	spien=SPI_isEnabled();
	if(spien) SPI_Disable();
	LCD_USART.BAUD.reg=2;  //48MHz/6 = 8 MHz
	if(spien) SPI_Enable();
}

//Maksymalna dostêpna w MCU szybkoœæ SPI
static __attribute__((always_inline)) inline void SPI_SPICLKMAX()
{
	_Bool spien;
	spien=SPI_isEnabled();
	if(spien) SPI_Disable();
	LCD_USART.BAUD.reg=0;  //MAX SPI SCK dla D21 to 12 MHz
	if(spien) SPI_Enable();
}

static __attribute__((always_inline)) inline void SPI_Speed_120kHz()
{
	_Bool spien;
	spien=SPI_isEnabled();
	if(spien) SPI_Disable();
	SPI_Disable(); //Zablokuj SERCOM
	//LCD_USART.BAUD.reg=SystemCoreClock/120000 - 1;
	LCD_USART.BAUD.reg=255;
	if(spien) SPI_Enable();
}

//Funkcja wysy³a/odbiera bajt z SPI, przy czym czeka na zakoñczenie wysy³ki
static __attribute__((always_inline)) inline uint8_t SPI_RW(uint8_t ch)
{
	LCD_USART.DATA.reg=ch;
	while(!(LCD_USART.INTFLAG.reg & SERCOM_SPI_INTFLAG_TXC));  //Zaczekaj na wys³anie danych
	return LCD_USART.DATA.reg;   //Odczytaj bajt i skasuj flagê
}

static __attribute__((always_inline)) inline void SPI_RW_Byte(uint8_t byte)
{
	while(!(LCD_USART.INTFLAG.reg & SERCOM_SPI_INTFLAG_DRE));  //Czy w buforze jest miejsce?
	LCD_USART.DATA.reg=byte;
	while(!(LCD_USART.INTFLAG.reg & SERCOM_SPI_INTFLAG_TXC));  //Zaczekaj na wys³anie danych
}

static __attribute__((always_inline)) inline void ssd2119_RESET()
{
	LCD_PORT.OUTCLR.reg=LCD_RESET;   //Aktywuj RESET
	delay_us(20);                    //Sygna³ reset musi trwaæ >20 us
	LCD_PORT.OUTSET.reg=LCD_RESET;   //Deaktywuj RESET
}

static __attribute__((always_inline)) inline void ssd2119_CS(uint8_t state)     //Funkcja zmieniaj¹ca stan sygna³u CS
{
	if(state==0) LCD_PORT.OUTCLR.reg=LCD_CS;   //Aktywuj CS
	else LCD_PORT.OUTSET.reg=LCD_CS;           //Deaktywuj CS
}

static __attribute__((always_inline)) inline void ssd2119_RS(uint8_t state)	//Funkcja zmieniaj¹ca stan sygna³u RS
{
	if(state==0) LCD_PORT.OUTCLR.reg=LCD_RS;   //Aktywuj RS
	else LCD_PORT.OUTSET.reg=LCD_RS;           //Deaktywuj RS
}

static void __attribute__((always_inline)) inline ssd2119_SendDataWord(uint16_t data)
{
	ssd2119_CS(0);
	SPI_RW(data >> 8);  //Wyœlij bardziej znacz¹cy bajt danych
	ssd2119_CS(1);
	ssd2119_CS(0);
	SPI_RW(data & 0xff);  //Wyœlij mniej znacz¹cy bajt danych
	ssd2119_CS(1);
}

static __attribute__((always_inline)) inline void ssd2119_SendDataByte(uint8_t data)
{
	while(!(LCD_USART.INTFLAG.reg & SERCOM_SPI_INTFLAG_DRE));  //Czy w buforze jest miejsce?
	LCD_USART.DATA.reg=data;
}

static __attribute__((always_inline)) inline void ssd2119_SendCmd(uint8_t cmd)
{
	ssd2119_RS(0);
	ssd2119_CS(0);
	SPI_RW_Byte(cmd);
	ssd2119_RS(1);     //Deaktywuj tryb wysy³ania poleceñ
}

#endif /* SPI_H_ */