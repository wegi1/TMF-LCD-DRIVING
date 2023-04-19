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

#include "Clk/Clk.h"
#include "GFXDrv.h"
#include "SPI/spi.h"
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
		LCD_SetTextOpaque(x1, y1+16, PSTR("ARM&AVR. LCD-TFT"), system16_array, color, 0x000000ul);

		if(counterrdy)
		{
			sprintf_P(bufor, PSTR("%7lu b/s, %6lu pix/s"), counter/8, counter/8/10);
			LCD_SetTextOpaque(0, 0, bufor, system16_array, 0xfffffful, 0x000000ul);
			Reset_Counter();
		}
	}
}

void Line_Demo()
{
	char bufor[50];

	uint16_t ile=6000;
	while(--ile)
	{
		uint16_t x1=rand() % LCD_GetMaxX();   //Maksymalna wspó³rzêdna x
		uint16_t x2=rand() % LCD_GetMaxX();
		uint8_t  y1=rand() % (LCD_GetMaxY()-16);   //Maksymalna wspó³rzêdna y minus szerokoœæ linii tekstu
		uint8_t  y2=rand() % (LCD_GetMaxY()-16);
		__uint24 color=rand() + (((__uint24)rand()) << 8) + (((__uint24)rand()) << 16); //Wybierz kolor
		LCD_LineTo(x1, y1+16, x2, y2+16, color);

		if(counterrdy)
		{
			sprintf_P(bufor, PSTR("%7lu b/s, %7lu pix/s"), counter/8, counter/8/10);
			LCD_SetTextOpaque(0, 0, bufor, system16_array, 0xfffffful, 0x000000ul);
			Reset_Counter();
		}
	}
}

void Bitmap_Demo()
{
	char bufor[50];

	uint16_t x,y;
	LCD_GetBitmapSize(&x, &y, image_data_Browser48x48);  //Rozmiar bitmapy

	uint16_t ile=3000;
	while(--ile)
	{
		uint16_t x1=rand() % (LCD_GetMaxX() - x);        //Maksymalna wspó³rzêdna x
		uint8_t  y1=rand() % (LCD_GetMaxY() - y - 16);   //Maksymalna wspó³rzêdna y minus szerokoœæ linii tekstu i bitmapy

		LCD_DrawBitmap_565(x1, y1 + 16, image_data_Browser48x48);

		if(counterrdy)
		{
			sprintf_P(bufor, PSTR("%5lu b/s, %5lu pix/s"), counter/8, counter/8/2);
			LCD_SetTextOpaque(0, 0, bufor, system16_array, 0xfffffful, 0x000000ul);
			Reset_Counter();
		}
	}
}

void Bitmap_Mono_Demo()
{
	char bufor[50];

	uint16_t x,y;
	LCD_GetBitmapSize(&x, &y, image_data_MGlass_ico);  //Rozmiar bitmapy
	uint16_t ile=3000;
	while(--ile)
	{
		uint16_t x1=rand() % (LCD_GetMaxX() - x);        //Maksymalna wspó³rzêdna x
		uint8_t  y1=rand() % (LCD_GetMaxY() - y - 16);   //Maksymalna wspó³rzêdna y minus szerokoœæ linii tekstu i bitmapy
		uint16_t col=rand() + (((__uint24)rand()) << 8);
		uint16_t colbkg=rand() + (((__uint24)rand()) << 8);

		LCD_DrawBitmap_Mono(x1, y1 + 16,col, colbkg, image_data_MGlass_ico);

		if(counterrdy)
		{
			sprintf_P(bufor, PSTR("%5lu b/s, %5lu pix/s"), counter/8, counter/8/2);
			LCD_SetTextOpaque(0, 0, bufor, system16_array, 0xfffffful, 0x000000ul);
			Reset_Counter();
		}
	}
}

void Circle_Demo()
{
	char bufor[50];

	uint16_t ile=700;
	while(--ile)
	{
		uint8_t radius=rand() % 100;
		uint16_t x1=rand() % (LCD_GetMaxX() - 2*radius);        //Maksymalna wspó³rzêdna x
		uint8_t  y1=rand() % (LCD_GetMaxY() - 2*radius - 16);   //Maksymalna wspó³rzêdna y minus szerokoœæ linii tekstu i bitmapy
		__uint24 color=rand() + (((__uint24)rand()) << 8) + (((__uint24)rand()) << 16);

		LCD_Circle(x1 + radius, y1 + radius + 16, radius, true, color);

		if(counterrdy)
		{
			sprintf_P(bufor, PSTR("%5lu b/s, %5lu pix/s"), counter/8, counter/8/10);
			LCD_SetTextOpaque(0, 0, bufor, system16_array, 0xfffffful, 0x000000ul);
			Reset_Counter();
		}
	}
}

int main(void)
{
	Set48MHzClk();
	PMIC_CTRL=PMIC_LOLVLEN_bm;           //Odblokuj przerwania niskiego poziomu
	sei();

	LCD_Interface_Init();                //Inicjalizacja inerfejsu ³¹cz¹cego z LCD
	LCD_Init262();                       //Inicjalizacja LCD

	CountSCK_Init();                     //Inicjalizacja pomiaru szybkoœci transferu

	while(1)
	{
		LCD_Rect(0, 0, 319, 239, 0x000000ul);  //Skasuj ekran i przy okazji ustaw okno dostêpu do GRAM
		Text_Demo();
		LCD_Rect(0, 0, 319, 239, 0x000000ul);
		Line_Demo();
		LCD_Rect(0, 0, 319, 239, 0x000000ul);
		Bitmap_Demo();
		LCD_Rect(0, 0, 319, 239, 0x000000ul);
		Bitmap_Mono_Demo();
		LCD_Rect(0, 0, 319, 239, 0x000000ul);
		Circle_Demo();
	}
}