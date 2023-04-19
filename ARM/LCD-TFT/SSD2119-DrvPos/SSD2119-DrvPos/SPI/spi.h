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
static __attribute__((always_inline)) inline void SPI_SPICLK10M()
{
	LCD_USART.BAUD.reg=2;  //48MHz/6 = 8 MHz
}

//Maksymalna dost�pna w MCU szybko�� SPI
static __attribute__((always_inline)) inline void SPI_SPICLKMAX()
{
	LCD_USART.BAUD.reg=1;  //MAX SPI SCK dla D21 to 12 MHz
}

//Funkcja wysy�a/odbiera bajt z SPI, przy czym czeka na zako�czenie wysy�ki
static __attribute__((always_inline)) inline uint8_t SPI_RW(uint8_t ch)
{
	LCD_USART.DATA.reg=ch;
	while(!(LCD_USART.INTFLAG.reg & SERCOM_SPI_INTFLAG_TXC));  //Zaczekaj na wys�anie danych
	return LCD_USART.DATA.reg;   //Odczytaj bajt i skasuj flag�
}

void SPI_init()
{
	//Konfiguracja SPI
	REG_PM_APBCMASK|=PM_APBCMASK_SERCOM1;  //W��cz zegar dla SERCOM1
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(GCLK_CLKCTRL_ID_SERCOM1_CORE_Val) | //Generic Clock 0
	GCLK_CLKCTRL_GEN_GCLK0 |                            // jest �r�d�em zegara
	GCLK_CLKCTRL_CLKEN;
	
	while(GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY); //Zaczekaj na zynchronizacj�
	
	LCD_USART.CTRLA.bit.ENABLE=0;      //Wy��cz SERCOM1
	while(LCD_USART.SYNCBUSY.bit.ENABLE);
	LCD_USART.CTRLA.bit.SWRST=1;      //Zresetuj SERCOM1
	while(LCD_USART.CTRLA.bit.SWRST || LCD_USART.SYNCBUSY.bit.SWRST);
	
	SPI_SPICLKMAX();  //Zegar SPI max - 12 MHz
	LCD_USART.CTRLB.reg=SERCOM_SPI_CTRLB_RXEN; //Odblokuj odbiornik SPI, ramka 8 bitowa, programowa kontrola SS
	while(LCD_USART.SYNCBUSY.reg & SERCOM_SPI_SYNCBUSY_MASK);  //Zaczekaj na synchreonizacj� zapisu rejestr�w
	
	LCD_PORT.WRCONFIG.reg=PORT_WRCONFIG_HWSEL | PORT_WRCONFIG_WRPINCFG | PORT_WRCONFIG_WRPMUX | PORT_WRCONFIG_PMUX(2) | PORT_WRCONFIG_PMUXEN | 0b1101; //Wybierz funkcj� SERCOM1 dla PA16-19
	
	LCD_USART.CTRLA.reg=SERCOM_SPI_CTRLA_ENABLE | SERCOM_SPI_CTRLA_MODE_SPI_MASTER | SERCOM_SPI_CTRLA_DOPO(1) | SERCOM_SPI_CTRLA_RUNSTDBY; //Tryb master SPI, Mode 0, MSB, PAD0 - MISO
	while(LCD_USART.SYNCBUSY.reg & SERCOM_SPI_SYNCBUSY_MASK);  //Zaczekaj na synchreonizacj� zapisu rejestr�w
}

static __attribute__((always_inline)) inline void ssd2119_RESET()
{
	LCD_PORT.OUTCLR.reg=LCD_RESET;   //Aktywuj RESET
	_delay_ms(1);
	//_delay_us(20);                         //Sygna� reset musi trwa� >20 us
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
}

__attribute__ ((flatten)) void ssd2119_SendCmd(uint8_t cmd)
{
	ssd2119_RS(0);                                //dla synchronizacji nadawania z sygna�em RS
	ssd2119_CS(0);
	SPI_RW(cmd);
	ssd2119_CS(1);     //Deaktywuj kontroler
	ssd2119_RS(1);     //Deaktywuj tryb wysy�ania polece�
}

__attribute__ ((flatten)) void ssd2119_SendCmdWithData(uint8_t cmd, uint16_t data)
{
	ssd2119_SendCmd(cmd);
	ssd2119_SendDataWord(data);
}

void LCD_Interface_Init()
{
	ssd2119_CS(false);  //Sygna� CS nieaktywny
	LCD_PORT.DIRSET.reg=LCD_CS | LCD_RS | LCD_RESET;	//Piny CS, RS i RESET jako wyj�cia
	SPI_init();   //Zainicjuj u�ywany SPI (SERCOM1)
}
#endif /* SPI_H_ */