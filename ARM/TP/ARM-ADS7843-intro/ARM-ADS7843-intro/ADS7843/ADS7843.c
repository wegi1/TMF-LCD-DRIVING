/*
 * ADS7843.c
 *
 * Created: 2014-04-21 15:49:10
 *  Author: tmf
 */ 

#include "ADS7843.h"
#include "../SPI/spi.h"
#include "../Delay/delay.h"
#include <stdbool.h>

static __attribute__((always_inline)) inline void ADS7843_CS(_Bool state)
{
	if(state) LCD_PORT.OUTSET.reg=TP_CS;
	else LCD_PORT.OUTCLR.reg=TP_CS;
}

void Touch_Panel_Init()
{
	ADS7843_CS(true);  //Deaktywuj TP CS
	LCD_PORT.DIRSET.reg=TP_CS;  //TP CS jest wyjœciem
	LCD_PORT.DIRCLR.reg=TP_INT; //Wejœcie INT TP
	LCD_PORT.WRCONFIG.reg=TP_INT | PORT_WRCONFIG_WRPINCFG | PORT_WRCONFIG_INEN | PORT_WRCONFIG_PULLEN; //Odblokowujemy pin i w³¹czamy pullup
}

uint16_t TouchPanel_GetPosition(TouchPanel_Cord cord)
{
	ADS7843_Cntrl ctrl={.Mode=ADS7843_Mode_12bit, .PD=PowerDown_BetweenConversions, .S=1, .SD=ADS7843_Differential};
	uint16_t result;
	
	switch(cord)
	{
		case TouchPanel_CordX: ctrl.Mux=ADS7843_IN_X;
		                       break;
		case TouchPanel_CordY: ctrl.Mux=ADS7843_IN_Y;
		                       break;
	}
	
	LCD_USART.DATA;    //Wyczyœæ bufor odbiornika
	LCD_USART.DATA;
	LCD_USART.DATA;
	
	SPI_Speed_120kHz();    //Uk³ady ADS maj¹ ograniczon¹ do ok. 120 kHz szybkoœæ taktowania ADC
	ADS7843_CS(false);     //Wybierz uk³ad kontrolera
	delay_us(1);
	
	SPI_RW(ctrl.Byte);          //Wyœlij bajt kontrolny
	SPI_RW(0x00);
	
	SPI_RW(ctrl.Byte);         //Wyœlij bajt kontrolny
	result=SPI_RW(0x00) << 8;  //Odbierz bardziej znacz¹cy bajt wyniku
	result|=SPI_RW(0x00);      //Odbierz mniej znacz¹cy bajt wyniku
	result>>=3;						//Tylko 12 bitów zawiera wynik, cztery najm³odsze maj¹ wartoœæ 0
	
	ADS7843_CS(true);  //Deaktywuj uk³ad kontrolera
	SPI_SPICLKMAX();
	return result;
}

void TouchPanel_GetPositionXY(TP_Position *pos)
{
	//ADS7843_Cntrl ctrl={.Mode=ADS7843_Mode_12bit, .PD=PowerDown_BetweenConversions, .S=1, .SD=ADS7843_SingleEnded};
	ADS7843_Cntrl ctrl={.Mode=ADS7843_Mode_12bit, .PD=PowerDown_BetweenConversions, .S=1, .SD=ADS7843_Differential};
	
	SPI_Speed_120kHz();     //Uk³ady ADS maj¹ ograniczon¹ do ok. 120 kHz szybkoœæ taktowania ADC
	ADS7843_CS(false);      //Wybierz uk³ad kontrolera
	
	ctrl.Mux=ADS7843_IN_X;  //Najpierw odczytujemy pozycjê X
	
	SPI_RW(ctrl.Byte);         //Wyœlij bajt kontrolny
	pos->X=SPI_RW(0x00) << 8;  //Odbierz bardziej znacz¹cy bajt wyniku
	
	ctrl.Mux=ADS7843_IN_Y;          //W trakcie odbierania poprzedniej pozycji mo¿emy wys³¹æ nowe polecenie konwersji
	pos->X|=SPI_RW(ctrl.Byte); //Odbierz mniej znacz¹cy bajt wyniku
	pos->X>>=3;
	
	pos->Y=SPI_RW(0x00) << 8;  //Odbierz pozycjê Y
	pos->Y|=SPI_RW(0x00);
	pos->Y>>=3;
	
	ADS7843_CS(true);  //Deaktywuj uk³ad kontrolera
	SPI_SPICLKMAX();
}