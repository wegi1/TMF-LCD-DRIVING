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

#include <util/delay.h>
#include "GFXDrv.h"
#include "Clk/Clk.h"
#include "SPI/spi.h"
#include "Fonts/Fonts.h"
#include "Icons.h"

int main(void)
{
	Set32MHzClk();
	PMIC_CTRL=PMIC_LOLVLEN_bm;           //Odblokuj przerwania niskiego poziomu
	sei();

	LCD_Interface_Init();                //Inicjalizacja inerfejsu ³¹cz¹cego z LCD
	LCD_Init262();                       //Inicjalizacja LCD

	while(1)
	{
		LCD_Rect(0, 0, 319, 239, 0x000000ul);  //Skasuj ekran i przy okazji ustaw okno dostêpu do GRAM
		LCD_DrawBitmapRLE_565(96, 56, image_data_Calculatorsmall);
		_delay_ms(2000);
		LCD_Rect(0, 0, 319, 239, 0x000000ul);
		LCD_DrawBitmapRLE_Mono(80, 40, 0xffff, 0x0000, image_data_rocketbw);
		_delay_ms(2000);
	};
}