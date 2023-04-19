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
static __attribute__((always_inline)) inline void SPI_SPICLK10M()
{
	LCD_USART.BAUD.reg=2;  //48MHz/6 = 8 MHz
}

//Maksymalna dostêpna w MCU szybkoœæ SPI
static __attribute__((always_inline)) inline void SPI_SPICLKMAX()
{
	LCD_USART.BAUD.reg=1;  //MAX SPI SCK dla D21 to 12 MHz
}

//Funkcja wysy³a/odbiera bajt z SPI, przy czym czeka na zakoñczenie wysy³ki
static __attribute__((always_inline)) inline uint8_t SPI_RW(uint8_t ch)
{
	LCD_USART.DATA.reg=ch;
	while(!(LCD_USART.INTFLAG.reg & SERCOM_SPI_INTFLAG_TXC));  //Zaczekaj na wys³anie danych
	return LCD_USART.DATA.reg;   //Odczytaj bajt i skasuj flagê
}

void SPI_init()
{
	//Konfiguracja SPI
	REG_PM_APBCMASK|=PM_APBCMASK_SERCOM1;  //W³¹cz zegar dla SERCOM1
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(GCLK_CLKCTRL_ID_SERCOM1_CORE_Val) | //Generic Clock 0
	GCLK_CLKCTRL_GEN_GCLK0 |                            // jest Ÿród³em zegara
	GCLK_CLKCTRL_CLKEN;
	
	while(GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY); //Zaczekaj na zynchronizacjê
	
	LCD_USART.CTRLA.bit.ENABLE=0;      //Wy³¹cz SERCOM1
	while(LCD_USART.SYNCBUSY.bit.ENABLE);
	LCD_USART.CTRLA.bit.SWRST=1;      //Zresetuj SERCOM1
	while(LCD_USART.CTRLA.bit.SWRST || LCD_USART.SYNCBUSY.bit.SWRST);
	
	SPI_SPICLKMAX();  //Zegar SPI max - 12 MHz
	LCD_USART.CTRLB.reg=SERCOM_SPI_CTRLB_RXEN; //Odblokuj odbiornik SPI, ramka 8 bitowa, programowa kontrola SS
	while(LCD_USART.SYNCBUSY.reg & SERCOM_SPI_SYNCBUSY_MASK);  //Zaczekaj na synchreonizacjê zapisu rejestrów
	
	LCD_PORT.WRCONFIG.reg=PORT_WRCONFIG_HWSEL | PORT_WRCONFIG_WRPINCFG | PORT_WRCONFIG_WRPMUX | PORT_WRCONFIG_PMUX(2) | PORT_WRCONFIG_PMUXEN | 0b1101; //Wybierz funkcjê SERCOM1 dla PA16-19
	
	LCD_USART.CTRLA.reg=SERCOM_SPI_CTRLA_ENABLE | SERCOM_SPI_CTRLA_MODE_SPI_MASTER | SERCOM_SPI_CTRLA_DOPO(1) | SERCOM_SPI_CTRLA_RUNSTDBY; //Tryb master SPI, Mode 0, MSB, PAD0 - MISO
	while(LCD_USART.SYNCBUSY.reg & SERCOM_SPI_SYNCBUSY_MASK);  //Zaczekaj na synchreonizacjê zapisu rejestrów
}

static __attribute__((always_inline)) inline void ssd2119_RESET()
{
	LCD_PORT.OUTCLR.reg=LCD_RESET;   //Aktywuj RESET
	_delay_ms(1);
	//_delay_us(20);                         //Sygna³ reset musi trwaæ >20 us
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

__attribute__ ((flatten)) void ssd2119_SendCmd(uint8_t cmd)
{
	ssd2119_RS(0);                                //dla synchronizacji nadawania z sygna³em RS
	ssd2119_CS(0);
	SPI_RW(cmd);
	ssd2119_CS(1);     //Deaktywuj kontroler
	ssd2119_RS(1);     //Deaktywuj tryb wysy³ania poleceñ
}

__attribute__ ((flatten)) void ssd2119_SendCmdWithData(uint8_t cmd, uint16_t data)
{
	ssd2119_SendCmd(cmd);
	ssd2119_SendDataWord(data);
}

void LCD_Interface_Init()
{
	ssd2119_CS(false);  //Sygna³ CS nieaktywny
	LCD_PORT.DIRSET.reg=LCD_CS | LCD_RS | LCD_RESET;	//Piny CS, RS i RESET jako wyjœcia
	SPI_init();   //Zainicjuj u¿ywany SPI (SERCOM1)
}
#endif /* SPI_H_ */