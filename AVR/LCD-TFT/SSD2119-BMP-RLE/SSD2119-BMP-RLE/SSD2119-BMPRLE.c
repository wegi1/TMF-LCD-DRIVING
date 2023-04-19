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
#include "SPI/spi.h"
#include "Clk/Clk.h"
#include "Fonts/Fonts.h"
#include "Icons.h"
#include "BMPLib.h"



int main(void)
{
	Set48MHzClk();
	PMIC_CTRL=PMIC_LOLVLEN_bm;           //Odblokuj przerwania niskiego poziomu
	sei();

	LCD_Interface_Init();                //Inicjalizacja inerfejsu ³¹cz¹cego z LCD
	LCD_Init262();                       //Inicjalizacja LCD
	
	while(1)
	{
		LCD_Rect(0, 0, 319, 239, 0x000000ul);  //Skasuj ekran i przy okazji ustaw okno dostêpu do GRAM
		LCD_BMP(&_binary____Graph_bmp_start, 0, 0);  //Indeksowany, 8bpp, bez RLE
		_delay_ms(1000);
	
		LCD_Rect(0, 0, 319, 239, 0x000000ul);  //Skasuj ekran i przy okazji ustaw okno dostêpu do GRAM
		LCD_BMP(&_binary____Weather_bmp_start, 0, 0); //Indeksowany, 8bpp, z RLE
		_delay_ms(1000);

		LCD_Rect(0, 0, 319, 239, 0x000000ul);  //Skasuj ekran i przy okazji ustaw okno dostêpu do GRAM
		LCD_BMP(&_binary____Maps_bmp_start, 0, 0); //Indeksowany, 4bpp, z RLE
		_delay_ms(1000);
	}
}