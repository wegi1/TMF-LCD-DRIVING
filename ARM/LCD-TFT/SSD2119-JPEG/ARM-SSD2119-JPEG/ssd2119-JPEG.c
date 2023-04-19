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
#include "tjpgd.h"

#define	IMPORT_BIN(file, symbol) asm (\
".section \".rodata.\"\n"\
".balign 4\n"\
".global "#symbol"_start\n"\
#symbol"_start:\n"\
".incbin \"" file "\"\n"\
".global "#symbol"_end\n"\
#symbol"_end:\n"\
".balign 4\n"\
".section \".text\"\n")

IMPORT_BIN("../tiger.jpg", tiger);

extern const uint8_t tiger_start;
extern const uint8_t tiger_end;

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

typedef struct
{
	const uint8_t *ptr;     //Adres odczytywanych danych
	const uint8_t *ptrend;  //Adres koñca odczytywanych danych
	uint16_t x;             //Wspó³rzêdne (x,y) wyœwietlanego obrazu
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
	Set48MHzClk();
	delay_init();
	LCD_Interface_Init();
	LCD_Init262();

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
