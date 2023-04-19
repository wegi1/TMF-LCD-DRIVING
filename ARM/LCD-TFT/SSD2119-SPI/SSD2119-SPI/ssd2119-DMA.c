/*
 * ssd2119_DMA.c
 *
 * Created: 2014-11-09 16:11:46
 *  Author: tmf
 */ 

#include "sam.h"
#include <stdbool.h>
#include "Clk/SetClk.h"
#include "Delay/delay.h"
#include "SPI/spi.h"
#include "ssd2119.h"

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

typedef struct
{
	uint8_t reg;        //Numer rejestru uk³¹du SSD
	uint16_t value;     //Wartoœæ rejestru
} SSD_Regs;

const SSD_Regs SSD_Init262[] = {{0x28, 0x0006}, {0x00, 0x0001}, {0x10, 0x0000}, {0x01, 0x72EF}, {0x02, 0x0600}, {0x03, 0x6A38}, {0x11, 0x4870},
{0x0F, 0x0000}, {0x0B, 0x5308}, {0x0C, 0x0003}, {0x0D, 0x000A}, {0x0E, 0x2E00}, {0x1E, 0x00BE}, {0x25, 0x8000},
{0x26, 0x7800}, {0x4E, 0x0000}, {0x4F, 0x0000}, {0x12, 0x08D9}, {0x30, 0x0000}, {0x31, 0x0104}, {0x32, 0x0100},
{0x33, 0x0305}, {0x34, 0x0505}, {0x35, 0x0305}, {0x36, 0x0707}, {0x37, 0x0300}, {0x3A, 0x1200}, {0x3B, 0x0800},
{0x07, 0x0033}};

const SSD_Regs SSD_Init65[] = {{0x28, 0x0006}, {0x00, 0x0001}, {0x10, 0x0000}, {0x01, 0x72EF}, {0x02, 0x0600}, {0x03, 0x6A38}, {0x11, 0x6870},
{0x0F, 0x0000}, {0x0B, 0x5308}, {0x0C, 0x0003}, {0x0D, 0x000A}, {0x0E, 0x2E00}, {0x1E, 0x00BE}, {0x25, 0x8000},
{0x26, 0x7800}, {0x4E, 0x0000}, {0x4F, 0x0000}, {0x12, 0x08D9}, {0x30, 0x0000}, {0x31, 0x0104}, {0x32, 0x0100},
{0x33, 0x0305}, {0x34, 0x0505}, {0x35, 0x0305}, {0x36, 0x0707}, {0x37, 0x0300}, {0x3A, 0x1200}, {0x3B, 0x0800},
{0x07, 0x0033}};

void LCD_Init262()
{
	ssd2119_RESET();   //Zresetuj kontroler

	for(uint8_t indeks=0; indeks < sizeof(SSD_Init262)/sizeof(SSD_Init262[0]); indeks++)
	{
		ssd2119_SendCmd(SSD_Init262[indeks].reg);
		ssd2119_SendDataWord(SSD_Init262[indeks].value);
	}
}

void LCD_Init65()
{
	ssd2119_RESET();   //Zresetuj kontroler

	for(uint8_t indeks=0; indeks < sizeof(SSD_Init65)/sizeof(SSD_Init65[0]); indeks++)
	{
		ssd2119_SendCmd(SSD_Init65[indeks].reg);
		ssd2119_SendDataWord(SSD_Init65[indeks].value);
	}
}

const uint8_t tryby[] = {0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff};

void LCD_Test262()
{
	uint8_t tryb=0;

	ssd2119_SendCmd(0x4E);    // Pozycja X piksela
	ssd2119_SendDataWord(0x0000);
	ssd2119_SendCmd(0x4F);    // Pozycja Y piksela
	ssd2119_SendDataWord(0x0000);

	ssd2119_SendCmd(0x44);
	ssd2119_SendDataWord(0xEF00);
	ssd2119_SendCmd(0x45);
	ssd2119_SendDataWord(0x0000);
	ssd2119_SendCmd(0x46);
	ssd2119_SendDataWord(0x013F);

	ssd2119_SendCmd(0x22);    // Zapis pod wskazan¹ pozycjê

	while(1)
	{
		uint8_t r=tryby[tryb];
		uint8_t g=tryby[tryb+1];
		uint8_t b=tryby[tryb+2];
		ssd2119_CS(0);
		for(uint8_t y=0; y<240; y++)
		for(uint16_t x=0; x<320; x++)
		{
			ssd2119_SendDataByte(r);  //Red
			ssd2119_SendDataByte(g);  //Green
			ssd2119_SendDataByte(b);  //Blue
		}
		tryb+=3;
		tryb%=sizeof(tryby);
	}
}

typedef union
{
	struct
	{
		uint8_t blue  : 5;
		uint8_t green : 6;
		uint8_t red   : 5;
	};
	uint8_t byte[2];
} RGB565;

void LCD_Test65()
{
	uint8_t tryb=0;

	ssd2119_SendCmd(0x4E);
	ssd2119_SendDataWord(0x0000);
	ssd2119_SendCmd(0x4F);
	ssd2119_SendDataWord(0x0000);

	ssd2119_SendCmd(0x44);
	ssd2119_SendDataWord(0xEF00);
	ssd2119_SendCmd(0x45);
	ssd2119_SendDataWord(0x0000);
	ssd2119_SendCmd(0x46);
	ssd2119_SendDataWord(0x013F);

	ssd2119_SendCmd(0x22);

	while(1)
	{
		RGB565 rgb;
		rgb.red=tryby[tryb];
		rgb.green=tryby[tryb+1];
		rgb.blue=tryby[tryb+2];
		ssd2119_CS(0);
		for(uint8_t y=0; y<240; y++)
		for(uint16_t x=0; x<320; x++)
		{
			ssd2119_SendDataByte(rgb.byte[1]);
			ssd2119_SendDataByte(rgb.byte[0]);
		}
		tryb+=3;
		tryb%=sizeof(tryby);
	}
}

int main(void)
{
	Set48MHzClk();
	
	LCD_Interface_Init();
	//LCD_Init262();
	//LCD_Test262();
	LCD_Init65();
	LCD_Test65();
	while(1)
	{
		//TODO:: Please write your application code
	}
}
