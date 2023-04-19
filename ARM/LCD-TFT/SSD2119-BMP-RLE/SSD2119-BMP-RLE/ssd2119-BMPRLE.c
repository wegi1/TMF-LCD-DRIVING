/*
 * ssd2119_DMA.c
 *
 * Created: 2014-11-09 16:11:46
 *  Author: tmf
 */ 

#include "sam.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "Clk/SetClk.h"
#include "Delay/delay.h"
#include "SPI/spi.h"
#include "ssd2119.h"
#include "GFXDrv.h"
#include "Fonts/Fonts.h"
#include "Icons.h"
#include "BMPLib.h"

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

void LCD_Interface_Init()
{
	ssd2119_CS(false);  //Sygna³ CS nieaktywny
	LCD_PORT.DIRSET.reg=LCD_CS | LCD_RS | LCD_RESET;	//Piny CS, RS i RESET jako wyjœcia
	SPI_init();   //Zainicjuj u¿ywany SPI (SERCOM1)
}

volatile uint32_t counter;
volatile _Bool counterrdy;

//Przerwanie compare match TC6
void TC6_Handler()
{
	REG_TC6_INTFLAG=TC_INTFLAG_MC0;   //Skasuj flagê
	REG_TC6_COUNT32_COUNT=0;
	counterrdy=true;
}

void CountSCK_Init()
{
	REG_PM_APBCMASK|=PM_APBCMASK_TC6 | PM_APBCMASK_TC7;  //W³¹cz zegar dla TC6 i 7
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN(0) | GCLK_CLKCTRL_ID_TC6_TC7;

	REG_TC6_INTENSET=TC_INTFLAG_MC0;  //Odblokuj przerwanie compare match
	REG_TC6_CTRLA=TC_CTRLA_ENABLE | TC_CTRLA_MODE_COUNT32 | TC_CTRLA_PRESCALER_DIV1 | TC_CTRLA_WAVEGEN_NFRQ;
	REG_TC6_COUNT32_CC0=SystemCoreClock;  //Zg³oœ przerwanie co jedn¹ sekundê
	NVIC_EnableIRQ(TC6_IRQn);
}

void Text_Demo()
{
	char bufor[50];

	uint16_t ile=900; counter=0; REG_TC6_COUNT32_COUNT=0;
	while(--ile)
	{
		uint16_t x1=rand() % LCD_GetMaxX();   //Maksymalna wspó³rzêdna x
		uint8_t  y1=rand() % (LCD_GetMaxY()-16);   //Maksymalna wspó³rzêdna y
		uint32_t color=rand() + (((uint32_t)rand()) << 8) + (((uint32_t)rand()) << 16); //Wybierz kolor
		LCD_SetTextAA(x1, y1+16, "ARM&AVR. LCD-TFT", Times16WA_array, color, 0x000000ul);
		//LCD_SetAASubTextOpaque(x1, y1+16, "ARM&AVR. LCD-TFT", Times16AA332_array, color, 0x000000ul);
		//LCD_SetAATextOpaque(x1, y1+16, "ARM&AVR. LCD-TFT", Times16AABW_array, color, 0x000000ul);

		if(counterrdy)
		{
			sprintf(bufor, "%7lu b/s, %6lu pix/s", counter, counter/10);
			LCD_SetTextAA(0, 0, bufor, system16_array, 0xfffffful, 0x000000ul);
			counter=0; counterrdy=false;
		}
	}
}

void Draw_Text()
{
	LCD_Rect(0, 0, 319, 239, 0x000000ul);  //Skasuj ekran i przy okazji ustaw okno dostêpu do GRAM
	LCD_SetTextAA(1 ,0, "Test fontu z antyaliasingiem 2 bitowym", Times16AA2bit, 0xffff00, 0x000000);
	LCD_SetTextAA(1, 20, "ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]", Times16AA2bit, 0xffff00, 0x000000);
	
	LCD_SetTextAA(1, 40, "Test fontu antyaliasingiem 8 bitowym monochrom.", Times16AA8bitmono, 0xffff00,0x000000);
	LCD_SetTextAA(1, 60, "ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]", Times16AA8bitmono, 0xffff00,0x000000);

	LCD_SetTextAA(1, 80, "Test fontu z antyaliasingiem subpikselowym 3-3-2", Times16AA332_array, 0xffff00,0x000000);
	LCD_SetTextAA(1, 100, "ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]", Times16AA332_array, 0xffff00,0x000000);

	LCD_SetTextAA(1, 120, "Test fontu bez antyaliasingu", Times16WA_array, 0xffff00,0x000000);
	LCD_SetTextAA(1, 140, "ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]", Times16WA_array, 0xffff00,0x000000);
}

void DrawLineAA()
{
	LCD_Rect(0, 0, 319, 239, 0x000000ul);  //Skasuj ekran i przy okazji ustaw okno dostêpu do GRAM
	for(uint16_t xx=0; xx<320; xx+=8)
	{
		LCD_LineToAA(0, 0, xx, 239, 0xff0000, 0x000000);
		LCD_LineToAA(319, 0, xx, 239, 0x0000ff, 0x000000);
		LCD_LineToAA(0, 239, xx, 0, 0x00ff00, 0x000000);
	}
}

void DrawLine()
{
	LCD_Rect(0, 0, 319, 239, 0x000000ul);  //Skasuj ekran i przy okazji ustaw okno dostêpu do GRAM
	for(uint16_t xx=0; xx<320; xx+=8)
	{
		LCD_LineTo(0, 0, xx, 239, 0xff0000);
		LCD_LineTo(319, 0, xx, 239, 0x0000ff);
		LCD_LineTo(0, 239, xx, 0, 0x00ff00);
	}
}

void DrawCircles()
{
	LCD_Rect(0, 0, 319, 239, 0x000000ul);  //Skasuj ekran i przy okazji ustaw okno dostêpu do GRAM
	for(uint8_t r=10; r<110; r+=10)
	{
		LCD_CircleAA(160, 120, r, 0xffffff, 0x000000);
		LCD_Circle(160, 120, r+5, false, 0xffffff);
	}
}


int main(void)
{
	Set48MHzClk();
	delay_init();
	LCD_Interface_Init();
	LCD_Init262();

	while(1)
	{
		LCD_Rect(0, 0, 319, 239, 0x000000ul);  //Skasuj ekran i przy okazji ustaw okno dostêpu do GRAM
		LCD_BMP(&_binary____Graph_bmp_start, 0, 0);  //Indeksowany, 8bpp, bez RLE
		delay_ms(1000);
		
		LCD_Rect(0, 0, 319, 239, 0x000000ul);  //Skasuj ekran i przy okazji ustaw okno dostêpu do GRAM
		LCD_BMP(&_binary____Weather_bmp_start, 0, 0); //Indeksowany, 8bpp, z RLE
		delay_ms(1000);

		LCD_Rect(0, 0, 319, 239, 0x000000ul);  //Skasuj ekran i przy okazji ustaw okno dostêpu do GRAM
		LCD_BMP(&_binary____Maps_bmp_start, 0, 0); //Indeksowany, 4bpp, z RLE
		delay_ms(1000);
	}
}
