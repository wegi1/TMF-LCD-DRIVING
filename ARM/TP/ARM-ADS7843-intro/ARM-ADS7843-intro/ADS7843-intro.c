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
#include "ADS7843/ADS7843.h"
#include "Fonts/Fonts.h"

void SPI_init()
{
	//Konfiguracja SPI
	REG_PM_APBCMASK|=PM_APBCMASK_SERCOM1;  //W��cz zegar dla SERCOM1
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(GCLK_CLKCTRL_ID_SERCOM1_CORE_Val) | //Generic Clock 0
										GCLK_CLKCTRL_GEN_GCLK0 |                            // jest �r�d�em zegara
										GCLK_CLKCTRL_CLKEN;
		
	while(GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY); //Zaczekaj na zynchronizacj�
	
	LCD_USART.CTRLA.bit.ENABLE=0;      //Wy��cz SERCOM1
	while(LCD_USART.SYNCBUSY.bit.ENABLE);
	LCD_USART.CTRLA.bit.SWRST=1;      //Zresetuj SERCOM1
	while(LCD_USART.CTRLA.bit.SWRST || LCD_USART.SYNCBUSY.bit.SWRST);
	
	SPI_SPICLKMAX();  //Zegar SPI max - 12 MHz
	LCD_USART.CTRLB.reg=SERCOM_SPI_CTRLB_RXEN; //Odblokuj odbiornik SPI, ramka 8 bitowa, programowa kontrola SS
	while(LCD_USART.SYNCBUSY.reg & SERCOM_SPI_SYNCBUSY_MASK);  //Zaczekaj na synchreonizacj� zapisu rejestr�w
	
	LCD_PORT.WRCONFIG.reg=PORT_WRCONFIG_HWSEL | PORT_WRCONFIG_WRPINCFG | PORT_WRCONFIG_WRPMUX | PORT_WRCONFIG_PMUX(2) | PORT_WRCONFIG_PMUXEN | 0b1101; //Wybierz funkcj� SERCOM1 dla PA16-19
	
	LCD_USART.CTRLA.reg=SERCOM_SPI_CTRLA_ENABLE | SERCOM_SPI_CTRLA_MODE_SPI_MASTER | SERCOM_SPI_CTRLA_DOPO(1) | SERCOM_SPI_CTRLA_RUNSTDBY; //Tryb master SPI, Mode 0, MSB, PAD0 - MISO
	while(LCD_USART.SYNCBUSY.reg & SERCOM_SPI_SYNCBUSY_MASK);  //Zaczekaj na synchreonizacj� zapisu rejestr�w
}

void LCD_Interface_Init()
{
	ssd2119_CS(false);  //Sygna� CS nieaktywny
	LCD_PORT.DIRSET.reg=LCD_CS | LCD_RS | LCD_RESET;	//Piny CS, RS i RESET jako wyj�cia
	SPI_init();   //Zainicjuj u�ywany SPI (SERCOM1)
}

int main(void)
{
	Set48MHzClk();
	delay_init();
	LCD_Interface_Init();
	LCD_Init262();
	Touch_Panel_Init();                  //Inicjalizacja obs�ugi TP

	char bufor[20];
	TP_Position XY;
	uint16_t touches=0;
	uint8_t prevTouch=0;

	LCD_Rect(0, 0, LCD_GetMaxX(), LCD_GetMaxY(), 0x000000);
	
	while(1)
	{
		TouchPanel_GetPositionXY(&XY);
		//XY.X=TouchPanel_GetPosition(TouchPanel_CordX);  //Mo�na u�y� alternatywnie dla sprawdzenia innych sposob�w komunikacji
		//XY.Y=TouchPanel_GetPosition(TouchPanel_CordY);
		sprintf(bufor, "X=%05d, Y=%05d", XY.X, XY.Y);
		LCD_SetTextAA(1, 10, bufor, Times16AA332_array, 0xffff00,0x000000);
		
		if((LCD_PORT.IN.reg & TP_INT) == 0)
		{
			if(prevTouch == 0) touches++;
			prevTouch=1;
			LCD_SetTextAA(1, 30, "Pressed  ", Times16AA332_array, 0xff0000,0x000000);
			sprintf(bufor, "Touches=%05d", touches);
			LCD_SetTextAA(1, 62, bufor, Times16AA332_array, 0xff0000,0x000000);
			
		} else
		{
			LCD_SetTextAA(1, 30, "Released", Times16AA332_array, 0x00ff00,0x000000);
			prevTouch=0;
		}
		delay_ms(500);  //Op�nienie dla pokazania rozrzutu wynik�w pr�bkowania
	}



	while(1);
}
