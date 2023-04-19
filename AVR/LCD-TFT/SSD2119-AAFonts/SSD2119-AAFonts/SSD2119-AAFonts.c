/*
 * SSD2119_prymitywy_DMA.c
 *
 * Created: 2014-03-08 10:30:10
 *  Author: tmf
 */


#include <avr/io.h>
#include <stdbool.h>
#include <stdlib.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <stdio.h>

#include "GFXDrv.h"
#include "spi/spi.h"
#include "Clk/clk.h"
#include "Fonts/Fonts.h"
#include "Icons.h"

volatile uint32_t counter;
volatile _Bool counterrdy;

ISR(TCE0_OVF_vect)
{
	counter=(uint32_t)TCD1_CCA << 16 | TCD0_CCA;  //Przechwyæ stan timerów odpowiadaj¹cy zliczonej liczbie impulsw SCK
	counterrdy=true;
}

void CountSCK_Init()
{
	PORTC_PIN1CTRL=PORT_ISC_RISING_gc;             //Zbocze narastaj¹ce wyzwoli zdarzenie
	EVSYS_CH7MUX=EVSYS_CHMUX_PORTC_PIN1_gc;        //Zdarzenia z PC1 (SCK) bêd¹ taktowaæ timer
	TCD0.CTRLB=TC_WGMODE_NORMAL_gc | TC0_CCAEN_bm; //Przechwytujemy zdarzenie do kana³u CCA
	TCD0.CTRLD=TC_EVACT_CAPT_gc | TC_EVSEL_CH5_gc; //Skonfiguruj sposób przechwytywania zdarzeñ
	TCD0.CTRLA=TC_CLKSEL_EVCH7_gc;
	EVSYS_CH6MUX=EVSYS_CHMUX_TCD0_OVF_gc;          //Zdarzenie nadmiaru z TCD0 bêdzie taktowa³o TCD1
	TCD1.CTRLB=TC_WGMODE_NORMAL_gc | TC0_CCAEN_bm; //Przechwytujemy zdarzenie do kana³u CCA
	TCD1.CTRLA=TC_CLKSEL_EVCH6_gc;
	TCD1.CTRLD=TC_EVACT_CAPT_gc | TC_EVSEL_CH5_gc | TC0_EVDLY_bm; //Skonfiguruj sposób przechwytywania zdarzeñ

	TCE0.PER=F_CPU/1024 - 1;                  //Dla 32 MHz zlicz do 31250 co da czas 1 sekundy
	TCE0.INTCTRLA=TC_OVFINTLVL_LO_gc;         //Odblokuj przerwania nadmiaru timera
	TCE0.CTRLA=TC_CLKSEL_DIV1024_gc;          //Timer odmierzaj¹cy sekundê
	EVSYS_CH5MUX=EVSYS_CHMUX_TCE0_OVF_gc;     //Przepe³nienie timera wywo³a capture event dla timerów TCD0 i TCD1
}

void Reset_Counter()
{
	counterrdy=false;
	TCE0_CTRLA=TC_CLKSEL_OFF_gc;
	TCD0_CTRLA=TC_CLKSEL_OFF_gc;
	TCD0_CNT=0;
	TCD1_CNT=0;
	TCE0_CNT=0;
	TCE0_CTRLA=TC_CLKSEL_DIV1024_gc;
	TCD0_CTRLA=TC_CLKSEL_EVCH7_gc;
}

void Text_Demo()
{
	char bufor[50];

	uint16_t ile=900;
	while(--ile)
	{
		uint16_t x1=rand() % LCD_GetMaxX();   //Maksymalna wspó³rzêdna x
		uint8_t  y1=rand() % (LCD_GetMaxY()-16);   //Maksymalna wspó³rzêdna y
		__uint24 color=rand() + (((__uint24)rand()) << 8) + (((__uint24)rand()) << 16); //Wybierz kolor
		LCD_SetTextAA(x1, y1+16, PSTR("ARM&AVR. LCD-TFT"), Times16WA_array, color, 0x000000ul);
		//LCD_SetAASubTextOpaque(x1, y1+16, PSTR("ARM&AVR. LCD-TFT"), Times16AA332_array, color, 0x000000ul);
		//LCD_SetAATextOpaque(x1, y1+16, PSTR("ARM&AVR. LCD-TFT"), Times16AABW_array, color, 0x000000ul);

		if(counterrdy)
		{
			sprintf_P(bufor, PSTR("%7lu b/s, %6lu pix/s"), counter/8, counter/8/10);
			LCD_SetTextAA(0, 0, bufor, system16_array, 0xfffffful, 0x000000ul);
			Reset_Counter();
		}
	}
}

void Draw_Text()
{
	LCD_Rect(0, 0, 319, 239, 0x000000ul);  //Skasuj ekran i przy okazji ustaw okno dostêpu do GRAM
	LCD_SetTextAA(1 ,0, PSTR("Test fontu z antyaliasingiem 2 bitowym"), Times16AA2bit, 0xffff00, 0x000000);
	LCD_SetTextAA(1, 20, PSTR("ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]"), Times16AA2bit, 0xffff00, 0x000000);
	
	LCD_SetTextAA(1, 40, PSTR("Test fontu antyaliasingiem 8 bitowym monochrom."), Times16AA8bitmono, 0xffff00,0x000000);
	LCD_SetTextAA(1, 60, PSTR("ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]"), Times16AA8bitmono, 0xffff00,0x000000);

	LCD_SetTextAA(1, 80, PSTR("Test fontu z antyaliasingiem subpikselowym 3-3-2"), Times16AA332_array, 0xffff00,0x000000);
	LCD_SetTextAA(1, 100, PSTR("ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]"), Times16AA332_array, 0xffff00,0x000000);

	LCD_SetTextAA(1, 120, PSTR("Test fontu bez antyaliasingu"), Times16WA_array, 0xffff00,0x000000);
	LCD_SetTextAA(1, 140, PSTR("ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]"), Times16WA_array, 0xffff00,0x000000);	
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
	Set32MHzClk();
	PMIC_CTRL=PMIC_LOLVLEN_bm;           //Odblokuj przerwania niskiego poziomu
	sei();

	LCD_Interface_Init();                //Inicjalizacja inerfejsu ³¹cz¹cego z LCD
	LCD_Init262();                       //Inicjalizacja LCD

	CountSCK_Init();                     //Inicjalizacja pomiaru szybkoœci transferu

	while(1)
	{
		Draw_Text();
		_delay_ms(5000);
		DrawLineAA();
		_delay_ms(5000);
		DrawLine();
		_delay_ms(5000);
		DrawCircles();
		_delay_ms(5000);
	}
}