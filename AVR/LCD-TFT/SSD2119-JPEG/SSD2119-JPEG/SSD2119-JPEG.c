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
#include <string.h>

#include "GFXDrv.h"
#include "SPI/spi.h"
#include "tjpgd.h"
#include "Clk/Clk.h"

#define	IMPORT_BIN(file, symbol) asm (\
                                      ".section \".progmemx.data.\"\n"\
                                      ".balign 4\n"\
                                      ".global "#symbol"_start\n"\
                                      #symbol"_start:\n"\
                                      ".incbin \"" file "\"\n"\
                                      ".global "#symbol"_end\n"\
                                      #symbol"_end:\n"\
                                      ".balign 4\n"\
                                      ".section \".text\"\n")
									   
IMPORT_BIN("../tiger.jpg", tiger);

extern __memx const uint8_t tiger_start;
extern __memx const uint8_t tiger_end;

typedef struct
{
	const __memx uint8_t *ptr;     //Adres odczytywanych danych
	const __memx uint8_t *ptrend;  //Adres koñca odczytywanych danych
	uint16_t x;                    //Wspó³rzêdne (x,y) wyœwietlanego obrazu
	uint8_t y;
} FLASHIODEV;

UINT in_FLASH_func (JDEC* jd, BYTE* buff, UINT nbyte)
{
	FLASHIODEV *img = (FLASHIODEV*)jd->device;   // Identyfikator bitmapy

	if(buff)
	{   //Odczytaj bajty z pamiêci do bufora dekompresji
		nbyte=((img->ptrend - img->ptr) > nbyte) ? nbyte : img->ptrend - img->ptr;
		for(UINT cnt=0; cnt < nbyte; cnt++) buff[cnt]=*img->ptr++;  //Skopiuj dane z FLASH do bufora
		return nbyte;
	} else
	{   //Pomiñ wskazan¹ liczbê bajtów z pliku JPEG
		img->ptr+=nbyte;
		return (img->ptr > img->ptrend) ? 0 : nbyte;
	}
}

UINT out_LCD_func (JDEC* jd, void* bitmap, JRECT* rect)
{
	FLASHIODEV *img = (FLASHIODEV*)jd->device;   // Identyfikator bitmapy
	if((img->y + rect->top) >= LCD_GetMaxY()) return 1; //Jesteœmy poza obszarem kreœlenia
	if((img->x + rect->left) >= LCD_GetMaxX()) return 1;
	LCD_CpyBitmap_RGB(rect->left + img->x, rect->top + img->y, rect->right - rect->left + 1,  rect->bottom - rect->top + 1, (uint8_t*)bitmap);
	return 1;
}

int main(void)
{
	Set32MHzClk();
	PMIC_CTRL=PMIC_LOLVLEN_bm;           //Odblokuj przerwania niskiego poziomu
	sei();

	LCD_Interface_Init();                //Inicjalizacja inerfejsu ³¹cz¹cego z LCD
	LCD_Init262();                       //Inicjalizacja LCD
	void *work=malloc(3100);             //Pamiêæ na bufor
	
    LCD_Rect(0, 0, 319, 239, 0xffffff);
    FLASHIODEV jpegimg={.ptr=&tiger_start, .ptrend=&tiger_end, .x=0, .y=0}; //Adres zdjêcia
	JDEC jdec;                           //Uchwyt dekompresowanego obiektu
	JRESULT res;                         //Wynik przeprowadzonej operacji

	res=jd_prepare(&jdec, in_FLASH_func, work, 3100, &jpegimg);
	if (res == JDR_OK)
	{
		res = jd_decomp(&jdec, out_LCD_func, 0);
	}

	jpegimg=(FLASHIODEV){.ptr=&tiger_start, .ptrend=&tiger_end, .x=0, .y=0};  //Rysunek pomniejszony 4-krotnie
	res=jd_prepare(&jdec, in_FLASH_func, work, 3100, &jpegimg);
	if (res == JDR_OK)
	{
		res = jd_decomp(&jdec, out_LCD_func, 1);
	}
	
	jpegimg=(FLASHIODEV){.ptr=&tiger_start, .ptrend=&tiger_end, .x=0, .y=0};  //Rysunek pomniejszony 16-krotnie
	res=jd_prepare(&jdec, in_FLASH_func, work, 3100, &jpegimg);
	if (res == JDR_OK)
	{
		res = jd_decomp(&jdec, out_LCD_func, 2);
	}
	
	jpegimg=(FLASHIODEV){.ptr=&tiger_start, .ptrend=&tiger_end, .x=0, .y=0};  //Rysunek pomniejszony 64-krotnie
	res=jd_prepare(&jdec, in_FLASH_func, work, 3100, &jpegimg);
	if (res == JDR_OK)
	{
		res = jd_decomp(&jdec, out_LCD_func, 3);
	}

	free(work);

	while(1);
}