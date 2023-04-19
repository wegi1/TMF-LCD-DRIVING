/*
 * SSD2119_prymitywy_DMA.c
 *
 * Created: 2014-03-08 10:30:10
 *  Author: tmf
 */


#include <avr/io.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <util/delay.h>

#include "GFXDrv.h"
#include "spi.h"
#include "Fonts/Fonts.h"
#include "Icons.h"
#include "ADS7843/ADS7843.h"
#include "ADS7843/calibrate.h"

_Bool OSC_wait_for_rdy(uint8_t clk)
{
	uint8_t czas=255;
	while ((!(OSC.STATUS & clk)) && (--czas)) // Czekaj na ustabilizowanie si� generatora
	_delay_ms(1);
	return czas;   //false je�li generator nie wystartowa�, true je�li jest ok
}

void SelectPLL(OSC_PLLSRC_t src, uint8_t mult)
{
	mult&=OSC_PLLFAC_gm;
	OSC.PLLCTRL=src | mult;              //Ustaw �r�d�o i mno�nik PLL
	OSC.CTRL|=OSC_PLLEN_bm;				 //W��cz uk�ad PLL
	OSC_wait_for_rdy(OSC_PLLRDY_bm);     //Poczekaj na ustabilizowanie si� PLL
}

void TouchPanel_Calibrate()
{	
	TP_Position perfectDisplaySample[3] = {  //Punkty kalibracyjne na TP - u�ywamy tylko trzech		{50, 40},		{100, 200},		{300, 120}	};
	
	TP_Position ScreenSample[3];  //Pobrane pr�bki z panela

	void GetCalPoint(uint8_t sample)
	{
		TP_Position pos;
		LCD_Circle(perfectDisplaySample[sample].X, perfectDisplaySample[sample].Y, 5, true, 0x00ff00);
		
		while((VPORT0_IN & TP_INT) != 0);  //Poczekaj na dotyk
		_delay_ms(20);    //Zaczekaj na koniec drga�
		
		TouchPanel_GetPositionXY(&pos); 	//Pobierz miejsce dotkni�cia

		while((VPORT0_IN & TP_INT) == 0);   //Poczekaj na zwolnienie panela

		_delay_ms(100);	//Odczekaj chwil�, aby wyeliminowa� ew. drgania
		
		LCD_Circle(perfectDisplaySample[sample].X, perfectDisplaySample[sample].Y, 5, true, 0x000000);
		
		ScreenSample[sample].X=pos.X>>2; ScreenSample[sample].Y=pos.Y>>2;
	}
	
	for(uint8_t i=0; i<sizeof(perfectDisplaySample)/sizeof(perfectDisplaySample[0]); i++) GetCalPoint(i);	//Pobierz kolejne pr�bki z ADC
	
	setCalibrationMatrix(&perfectDisplaySample[0], &ScreenSample[0], &TP_matrix);  //Policz macierz
}

int main(void)
{
	SelectPLL(OSC_PLLSRC_RC2M_gc, 16);   //PLL 16x - na wyj�ciu 32 MHz
	CPU_CCP=CCP_IOREG_gc;
	CLK.CTRL=CLK_SCLKSEL_PLL_gc;         //Wybierz wyj�cie PLL, czyli zegar 32 MHz

	LCD_Interface_Init();                //Inicjalizacja interfejsu ��cz�cego z LCD
	LCD_Init262();                       //Inicjalizacja LCD
	
	LCD_Rect(0, 0, LCD_GetMaxX()-1, LCD_GetMaxY()-1, 0x000000);   //Wyczy�� ekran
	
	Touch_Panel_Init();                  //Inicjalizacja obs�ugi TP

	TP_Position XY, dis_point, scr;
	
	TouchPanel_Calibrate();   //Skalibruj panel dotykowy
	
	
	while(1)
	{
		TouchPanel_GetPositionXY(&XY);
		dis_point.X=XY.X>>2; dis_point.Y=XY.Y>>2;   //Potrzebujemy tylko 10-bit�w z ADC, wi�cej bit�w, to konieczno�� u�ycia szerszego typu do oblicze�
		getDisplayPoint(&scr, &dis_point, &TP_matrix);
		
		if((VPORT0_IN & TP_INT) == 0)
		{		
			LCD_SetPixel(scr.X, scr.Y, 0xff0000);
		} 
	}  
}