/*
 * SSD2119_RECT_DMA.c
 *
 * Created: 2014-02-23 18:24:35
 *  Author: tmf
 */


#include <stdbool.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include "ssd2119.h"
#include "Clk/Clk.h"
#include "SPI/spi.h"
#include <avr/io.h>
#include <util/delay.h>

__attribute__ ((flatten)) void ssd2119_SendCmdWithData(uint8_t cmd, uint16_t data)
{
	ssd2119_SendCmd(cmd);
	ssd2119_SendDataWord(data);
}

typedef struct
{
	uint8_t reg;        //Numer rejestru uk³¹du SSD
	uint16_t value;     //Wartoœæ rejestru
} SSD_Regs;

const SSD_Regs __flash SSD_Init262[] = {{0x28, 0x0006}, {0x00, 0x0001}, {0x10, 0x0000}, {0x01, 0x32EF}, {0x02, 0x0600}, {0x03, 0x6A38}, {0x11, 0x4870},
{0x0F, 0x0000}, {0x0B, 0x5308}, {0x0C, 0x0003}, {0x0D, 0x000A}, {0x0E, 0x2E00}, {0x1E, 0x00BE}, {0x25, 0x8000},
{0x26, 0x7800}, {0x4E, 0x0000}, {0x4F, 0x0000}, {0x12, 0x08D9}, {0x30, 0x0000}, {0x31, 0x0104}, {0x32, 0x0100},
{0x33, 0x0305}, {0x34, 0x0505}, {0x35, 0x0305}, {0x36, 0x0707}, {0x37, 0x0300}, {0x3A, 0x1200}, {0x3B, 0x0800},
{0x07, 0x0033}};

const SSD_Regs __flash SSD_Init65[] = {{0x28, 0x0006}, {0x00, 0x0001}, {0x10, 0x0000}, {0x01, 0x32EF}, {0x02, 0x0600}, {0x03, 0x6A38}, {0x11, 0x6870},
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

const __flash uint8_t tryby[] = {0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff};

typedef union
{
	struct
	{
		uint8_t blue  : 5;
		uint8_t green : 6;
		uint8_t red   : 5;
	};
	uint8_t byte[2];
	uint16_t word;
} RGB565;

volatile __uint24 DMA_bytestotransfer;
volatile _Bool DMA_busy=false;

ISR(DMA_CH3_vect)
{
	DMA.CH3.CTRLB|=DMA_CH_TRNIF_bm;     //Skasuj flagê przerwania
	if(DMA_bytestotransfer)
	{
		uint16_t bytes;
		if(DMA_bytestotransfer > 765) bytes=765; else bytes=DMA_bytestotransfer;
		DMA.CH3.REPCNT=bytes / 3;
		DMA_bytestotransfer-=bytes;
		DMA.CH3.CTRLA=DMA_CH_ENABLE_bm | DMA_CH_BURSTLEN_1BYTE_gc | DMA_CH_SINGLE_bm | DMA_CH_REPEAT_bm; //Przetransferuj kolejny blok danych
	} else
	{
		DMA_busy=false;
		while(!(LCD_USART.STATUS & USART_DREIF_bm));  //Zaczekaj na zakoñczenie ostatniego transferu
		ssd2119_CS(1);  //Koniec transferu            //Deaktywuj kontroler LCD
	}
}

void DMA_Fill_Init(__uint24 color, __uint24 pixelsno)
{
	volatile static __uint24 data;  //Dostêp do zmiennej odbywa siê za pomoc¹ zasobu sprzêtowego

	if(pixelsno == 0) return;       //¯adnych pikseli nie zniemiany wiêc koniec funkcji
	while(DMA_busy);				//Zaczekaj na koniec transakcji
	//Jeœli trwa transfer to nie mo¿na zmnieniaæ data i DMA_bytestotransfer
	DMA_busy=true;

	data=color;                     //Przepisz dane o kolorze do bufora
	DMA_bytestotransfer=pixelsno * 3;  //Transferujemy trzykrotnie wiêcej bajtów ni¿ pikseli

	DMA.CTRL|=DMA_ENABLE_bm;   //Odblokuj kontroler DMA z domyœlnymi ustawieniami
	DMA.CH3.SRCADDR0=(uint16_t)&data & 0xff;
	DMA.CH3.SRCADDR1=((uint16_t)&data >> 8) & 0xff;
	DMA.CH3.SRCADDR2=0;
	DMA.CH3.DESTADDR0=(uint16_t)&LCD_USART.DATA & 0xff;
	DMA.CH3.DESTADDR1=((uint16_t)&LCD_USART.DATA >> 8) & 0xff;
	DMA.CH3.DESTADDR2=0;

	DMA.CH3.ADDRCTRL=DMA_CH_SRCRELOAD_BLOCK_gc | DMA_CH_SRCDIR_INC_gc | DMA_CH_DESTDIR_FIXED_gc;
	DMA.CH3.TRFCNT=sizeof(data);      //Wielkoœæ transferowanego bloku
	uint16_t bytes;
	if(DMA_bytestotransfer > 765) bytes=765; else bytes=DMA_bytestotransfer;
	DMA.CH3.REPCNT=bytes  / 3;
	DMA_bytestotransfer-=bytes;
	DMA.CH3.TRIGSRC=LCD_DMA_TRIG_SRC; //Wyzwalacz DMA
	DMA.CH3.CTRLB=DMA_CH_TRNINTLVL_LO_gc;  //W³¹cz przerwanie koñca transakcji
	DMA.CH3.CTRLA=DMA_CH_ENABLE_bm | DMA_CH_BURSTLEN_1BYTE_gc | DMA_CH_SINGLE_bm | DMA_CH_REPEAT_bm;
}

void SSD2119_Rect(uint16_t x1, uint8_t y1, uint16_t x2, uint8_t y2, __uint24 color)
{
	while(DMA_busy);  //Zanim wyœlemy polecenia do kontrolera musimy mieæ pewnoœæ, ¿e nie trwa ¿aden transfer DMA

	ssd2119_SendCmdWithData(ssd2119_Vertical_RAM_address_position, ((uint16_t)y1) | (((uint16_t)y2) << 8)); //Górny i dolny brzeg obszaru
	ssd2119_SendCmdWithData(ssd2119_Horizontal_RAM_address_start_position, x1);    //Lewy brzeg obszaru
	ssd2119_SendCmdWithData(ssd2119_Horizontal_RAM_address_end_position, x2);    //Prawy brzeg obszaru

	ssd2119_SendCmdWithData(ssd2119_Set_GDDRAM_X_address_counter, x1);    // Pozycja X piksela
	ssd2119_SendCmdWithData(ssd2119_Set_GDDRAM_Y_address_counter, y1);    // Pozycja Y piksela

	ssd2119_SendCmd(0x22);    // Zapis pod wskazan¹ pozycjê

	ssd2119_CS(0);            //Aktywuj kontroler - DMA przeœle dane do GRAM i automatycznie zdezaktywuje sygna³ CS
	__uint24 val=(1+y2-y1)*(__uint24)(1+x2-x1);  //Policz z ilu pikseli sk³ada siê prostok¹t
	DMA_Fill_Init(color, val);               //Wyœlij dane
}

int main(void)
{
	Set32MHzClk();
	PMIC_CTRL=PMIC_LOLVLEN_bm;           //Odblokuj przerwania niskiego poziomu
	sei();

	LCD_Interface_Init();
	LCD_Init262();

    while(1)
    {
	    uint16_t x1=rand() % 320;   //Maksymalna wspó³rzêdna x
	    uint16_t x2=rand() % 320;
	    if(x1>x2)                   //x2 nie mo¿e byæ mniejsze ni¿ x1
	    {
		    uint16_t x=x2;
		    x2=x1;
		    x1=x;
	    }
	    uint8_t  y1=rand() % 240;   //Maksymalna wspó³rzêdna y
	    uint8_t  y2=rand() % 240;
	    if(y1>y2)                   //y2 nie mo¿e byæ mniejsze ni¿ y1
	    {
		    uint8_t y=y2;
		    y2=y1;
		    y1=y;
	    }
	    __uint24 color=rand() | (((__uint24)rand()) << 8) | (((__uint24)rand()) << 16); //Wybierz kolor
	    SSD2119_Rect(x1, y1, x2, y2, color);
    }
}