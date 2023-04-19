/*
 * FT5x06.c
 *
 * Created: 2016-01-24 13:39:15
 *  Author: tmf
 */ 

#include "FT5x06.h"
#include "Delay/delay.h"
#include <sam.h>
#include <stdint.h>
#include <stdbool.h>
#include "TWI/TWI.h"

#define CTP_SDA		PORT_PA08			//Pin I2C - SDA
#define CTP_SCL		PORT_PA09			//Pin I2C - SCL
#define CTP_RST		PORT_PA12			//Pin RESET
#define CTP_WAKE	PORT_PA10			//Pin Wake
#define CTP_INT		PORT_PA11			//Pin INT
#define FT5x06_TWI	SERCOM2->I2CM	//Wykorzystywany do komunikacji interfejs TWI
#define FT5x06_PORT	PORT->Group[0]  //Port IO sygna³ów steruj¹cych kontrolerem

void CTP_Init()
{
	TWI_Init(&FT5x06_TWI, TWI_BAUD_FROM_FREQ(100000));		//Baudrate=F_CPU / (2 * (5 + (BAUD))) - wykorzystujemy 100 kHz
	FT5x06_PORT.DIRSET.reg=CTP_RST | CTP_WAKE;		//Piny CTP_RST i CTP_WAKE
	FT5x06_PORT.OUTSET.reg=CTP_WAKE;
	FT5x06_PORT.OUTCLR.reg=CTP_RST;
	delay_ms(10);
	FT5x06_PORT.OUTSET.reg=CTP_RST;
	delay_ms(400);
	FT5x06_PORT.OUTCLR.reg=CTP_WAKE;
	delay_ms(10);
	FT5x06_PORT.OUTSET.reg=CTP_WAKE;
	delay_ms(400);
	
#ifdef FT_5x06_IRQEn
	FT5x06_PORT.DIRCLR.reg=CTP_INT; //Wejœcie INT TP
	FT5x06_PORT.WRCONFIG.reg=CTP_INT | PORT_WRCONFIG_WRPINCFG | PORT_WRCONFIG_WRPMUX | PORT_WRCONFIG_PMUX(0) | PORT_WRCONFIG_PMUXEN |PORT_WRCONFIG_INEN | PORT_WRCONFIG_PULLEN; //Odblokowujemy pin i w³¹czamy pullup oraz EXTINT10
	
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(GCLK_CLKCTRL_ID_EIC_Val) | //Generic Clock 0
	GCLK_CLKCTRL_GEN_GCLK0 |                            // jest Ÿród³em zegara dla EIC
	GCLK_CLKCTRL_CLKEN;
	
	while(GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY); //Zaczekaj na synchronizacjê
	
	EIC->CONFIG[1].bit.SENSE3=EIC_CONFIG_SENSE0_FALL_Val;  //Zbocze opadaj¹ce wywo³a przerwanie EXTINT11
	EIC->INTENSET.bit.EXTINT11=1;  //W³¹cz przerwanie EXTINT10
	EIC->CTRL.bit.ENABLE=1;
	NVIC_EnableIRQ(EIC_IRQn);   //Zezwól na przerwania EIC w NVIC
#endif
}

_Bool CTP_ReadRegister(uint8_t reg, uint8_t *value, uint8_t bytesno)
{
	uint8_t err=TWI_StartTransmission(&FT5x06_TWI, FT5206_Address, 100);
	if(err == TWI_ERROR_NoError)
	{
		TWI_SendByte(&FT5x06_TWI, reg);
		TWI_StopTransmission(&FT5x06_TWI);
		err=TWI_StartTransmission(&FT5x06_TWI, FT5206_Address | TWI_ADDRESS_READ, 100);
		if(err == TWI_ERROR_NoError)
		{
			bytesno--;
			uint8_t i;
			
			for(i=0; i < bytesno; i++)
			{
				err=!TWI_ReceiveByte(&FT5x06_TWI, &value[i], false);
			}
			err=!TWI_ReceiveByte(&FT5x06_TWI, &value[i], true);		//Ostatni bajt - NACK
		}
	}
	
	TWI_StopTransmission(&FT5x06_TWI);
	return (err == TWI_ERROR_NoError);    //Zwróæ true jeœli nie wyst¹pi³ ¿aden b³¹d	
}

_Bool CTP_WriteRegister(uint8_t reg, uint8_t *value, uint8_t bytesno)
{
	uint8_t err=TWI_StartTransmission(&FT5x06_TWI, FT5206_Address, 100);
	if(err == TWI_ERROR_NoError)
	{
		TWI_SendByte(&FT5x06_TWI, reg);
		for(uint8_t i=0; i < bytesno; i++)
		{
			err=!TWI_SendByte(&FT5x06_TWI, value[i]);
		}
		TWI_StopTransmission(&FT5x06_TWI);
	}
	return (err == TWI_ERROR_NoError);    //Zwróæ true jeœli nie wyst¹pi³ ¿aden b³¹d	
}