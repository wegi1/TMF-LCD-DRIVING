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
#include "TP_ADC/TP_ADC.h"
#include "Fonts/Fonts.h"

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

void TouchPanel_Calibrate()
{
	TP_Position perfectDisplaySample[3] = {  //Punkty kalibracyjne na TP - u¿ywamy tylko trzech		{50, 40},		{100, 200},		{300, 120}	};
	
	TP_Position ScreenSample[3];  //Pobrane próbki z panela

	void GetCalPoint(uint8_t sample)
	{
		TP_Position pos;
		LCD_Circle(perfectDisplaySample[sample].X, perfectDisplaySample[sample].Y, 5, true, 0x00ff00);
		
		while((LCD_PORT.IN.reg & TP_INT) != 0);  //Poczekaj na dotyk
		delay_ms(20);    //Zaczekaj na koniec drgañ
		
		TouchPanel_GetPositionXY(&pos); 	//Pobierz miejsce dotkniêcia

		while((LCD_PORT.IN.reg & TP_INT) == 0);   //Poczekaj na zwolnienie panela

		delay_ms(100);	//Odczekaj chwilê, aby wyeliminowaæ ew. drgania
		
		LCD_Circle(perfectDisplaySample[sample].X, perfectDisplaySample[sample].Y, 5, true, 0x000000);
		
		ScreenSample[sample].X=pos.X>>2; ScreenSample[sample].Y=pos.Y>>2;
	}
	
	for(uint8_t i=0; i<sizeof(perfectDisplaySample)/sizeof(perfectDisplaySample[0]); i++) GetCalPoint(i);	//Pobierz kolejne próbki z ADC
	
	setCalibrationMatrix(&perfectDisplaySample[0], &ScreenSample[0], &TP_matrix);  //Policz macierz
}

int main(void)
{
	Set48MHzClk();
	delay_init();
	LCD_Interface_Init();
	LCD_Init262();
	Touch_Panel_Init();                  //Inicjalizacja obs³ugi TP

	LCD_Rect(0, 0, LCD_GetMaxX(), LCD_GetMaxY(), 0x000000);
	
	TP_Position XY, dis_point, scr;
	
	//TouchPanel_Calibrate();   //Skalibruj panel dotykowy
	
	char bufor[20];
	
	while(1)
	{
		TouchPanel_GetPositionXY(&XY);
		//dis_point.X=XY.X>>2; dis_point.Y=XY.Y>>2;   //Potrzebujemy tylko 10-bitów z ADC, wiêcej bitów, to koniecznoœæ u¿ycia szerszego typu do obliczeñ
		//getDisplayPoint(&scr, &dis_point, &TP_matrix);
		
		sprintf(bufor, "X=%05d, Y=%05d", XY.X, XY.Y);
		LCD_SetTextAA(1, 10, bufor, Times16AA332_array, 0xffff00,0x000000);
		
		if((PORT->Group[0].IN.reg & PORT_PA04) == 0)
		{
			LCD_SetTextAA(1, 30, "Pressed  ", Times16AA332_array, 0xff0000,0x000000);
			//LCD_SetPixel(scr.X, scr.Y, 0xff0000);
		} else LCD_SetTextAA(1, 30, "Released", Times16AA332_array, 0x00ff00,0x000000);

	}
}
