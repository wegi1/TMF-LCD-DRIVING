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