/*
             LUFA Library
     Copyright (C) Dean Camera, 2015.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
  Copyright 2015  Dean Camera (dean [at] fourwalledcubicle [dot] com)

  Permission to use, copy, modify, distribute, and sell this
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in
  all copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaims all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

#define  __INCLUDE_FROM_TWI_C
#include "TWI.h"
#include "../Delay/delay.h"

static void pin_set_peripheral_function(uint32_t pinmux)
{
	uint8_t	port=(uint8_t)((pinmux	>>	16)/32);
	PORT->Group[port].PMUX[((pinmux	>>	16)	- (port*32))/2].reg	&=~(0xF	<<	(4*((pinmux >> 16) & 0x01u)));
	PORT->Group[port].PMUX[((pinmux >> 16) - (port*32))/2].reg |= (uint8_t)((pinmux & 0x0000ffff) << (4* ((pinmux >> 16) & 0x01u)));
	PORT->Group[port].PINCFG[((pinmux	>>	16)	- (port*32))].bit.PMUXEN=1;
}

void TWI_Init(SercomI2cm* const TWI, const uint32_t Baud)
{
	//Konfiguracja SPI
	REG_PM_APBCMASK|=PM_APBCMASK_SERCOM2;  //W³¹cz zegar dla SERCOM1
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(GCLK_CLKCTRL_ID_SERCOM2_CORE_Val) | //Generic Clock 0
	GCLK_CLKCTRL_GEN_GCLK0 |                            // jest Ÿród³em zegara
	GCLK_CLKCTRL_CLKEN;
	
	while(GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY); //Zaczekaj na zynchronizacjê
	
	pin_set_peripheral_function(PINMUX_PA08D_SERCOM2_PAD0);	// SDA
	pin_set_peripheral_function(PINMUX_PA09D_SERCOM2_PAD1);	// SCL
	
	TWI->CTRLA.reg=SERCOM_I2CM_CTRLA_SPEED(0) | SERCOM_I2CM_CTRLA_SDAHOLD(0x02) | SERCOM_I2CM_CTRLA_RUNSTDBY | SERCOM_I2CM_CTRLA_MODE_I2C_MASTER;
	TWI->CTRLB.reg=SERCOM_I2CM_CTRLB_SMEN;
	while(TWI->SYNCBUSY.bit.SYSOP);
				
	TWI->BAUD.reg=Baud;
	while(TWI->SYNCBUSY.bit.SYSOP);
	
	TWI->CTRLA.reg|=SERCOM_I2CM_CTRLA_ENABLE;
	
	while(TWI->SYNCBUSY.reg & SERCOM_I2CM_SYNCBUSY_ENABLE);
				
	TWI->STATUS.bit.BUSSTATE=0x01;   //Idle
	while(TWI->SYNCBUSY.bit.SYSOP);
}

uint8_t TWI_StartTransmission(SercomI2cm* const TWI,
                              const uint8_t SlaveAddress,
                              const uint8_t TimeoutMS)
{
	uint16_t TimeoutRemaining;

	TWI->ADDR.bit.ADDR = SlaveAddress;

	TimeoutRemaining = (TimeoutMS * 100);
	while (TimeoutRemaining)
	{
		if(TWI->INTFLAG.reg & SERCOM_I2CM_INTFLAG_MB)  //Czy adres zosta³ nadany?
		{
			uint8_t status = TWI->STATUS.reg;
			
			if ((status & SERCOM_I2CM_STATUS_ARBLOST) == SERCOM_I2CM_STATUS_ARBLOST)
			{
				TWI->ADDR.bit.ADDR = SlaveAddress;
			}
			else if ((status & SERCOM_I2CM_STATUS_RXNACK) == SERCOM_I2CM_STATUS_RXNACK)  //brak NACK
			{
				TWI_StopTransmission(TWI);
				return TWI_ERROR_SlaveResponseTimeout;
			}
			else if ((status & SERCOM_I2CM_STATUS_BUSERR) == 0)  //Wszystko ok
			{
				return TWI_ERROR_NoError;
			}
		}

			delay_us(10);
			TimeoutRemaining--;
	}

	if (!(TimeoutRemaining)) {
		if (TWI->STATUS.reg & SERCOM_I2CM_STATUS_CLKHOLD) {
			TWI_StopTransmission(TWI);
		}
	}

	return TWI_ERROR_BusCaptureTimeout;
}

bool TWI_SendByte(SercomI2cm* const TWI, const uint8_t Byte)
{
	TWI->DATA.reg = Byte;
	
	while(!TWI->INTFLAG.bit.MB) {

		// If a bus error occurs, the MB bit may never be set.
		// Check the bus error bit and bail if it's set.
		if (TWI->STATUS.bit.BUSERR) {
			return false;
		}
		
		if(TWI->STATUS.bit.RXNACK) return false;
		else return true;
	}
}

bool TWI_ReceiveByte(SercomI2cm* const TWI, uint8_t* const Byte, const bool LastByte)
{
	if (TWI->STATUS.reg & (SERCOM_I2CM_STATUS_BUSERR | SERCOM_I2CM_STATUS_ARBLOST)) return false;

	while (!TWI->INTFLAG.bit.SB);

	*Byte = TWI->DATA.reg;

	if (LastByte)
	TWI->CTRLB.reg=SERCOM_I2CM_CTRLB_ACKACT | SERCOM_I2CM_CTRLB_CMD(0x03);    //Stop condition with NACK
		else
	TWI->CTRLB.reg=SERCOM_I2CM_CTRLB_CMD(0x03);    //Stop condition with ACK
	return true;
}

uint8_t TWI_ReadPacket(SercomI2cm* const TWI, const uint8_t SlaveAddress, const uint8_t TimeoutMS, const uint8_t* InternalAddress, uint8_t InternalAddressLen, uint8_t* Buffer, uint16_t Length)
{
	uint8_t ErrorCode;

	if ((ErrorCode = TWI_StartTransmission(TWI, (SlaveAddress & TWI_DEVICE_ADDRESS_MASK) | TWI_ADDRESS_WRITE,
	TimeoutMS)) == TWI_ERROR_NoError)
	{
		while (InternalAddressLen--)
		{
			if (!(TWI_SendByte(TWI, *(InternalAddress++))))
				{
					ErrorCode = TWI_ERROR_SlaveNAK;
					break;
				}
		}

		if ((ErrorCode = TWI_StartTransmission(TWI, (SlaveAddress & TWI_DEVICE_ADDRESS_MASK) | TWI_ADDRESS_READ,
			TimeoutMS)) == TWI_ERROR_NoError)
		{
			while (Length--)
			{
				if (!(TWI_ReceiveByte(TWI, Buffer++, (Length == 0))))
				{
					ErrorCode = TWI_ERROR_SlaveNAK;
					break;
				}
			}
		}

		TWI_StopTransmission(TWI);
	}

	return ErrorCode;
}

uint8_t TWI_WritePacket(SercomI2cm* const TWI, const uint8_t SlaveAddress, const uint8_t TimeoutMS, const uint8_t* InternalAddress, uint8_t InternalAddressLen, const uint8_t* Buffer, uint16_t Length)
{
	uint8_t ErrorCode;

	if ((ErrorCode = TWI_StartTransmission(TWI, (SlaveAddress & TWI_DEVICE_ADDRESS_MASK) | TWI_ADDRESS_WRITE,
	TimeoutMS)) == TWI_ERROR_NoError)
	{
		while (InternalAddressLen--)
		{
			if (!(TWI_SendByte(TWI, *(InternalAddress++))))
			{
				ErrorCode = TWI_ERROR_SlaveNAK;
				break;
			}
		}

		while (Length--)
		{
			if (!(TWI_SendByte(TWI, *(Buffer++))))
			{
				ErrorCode = TWI_ERROR_SlaveNAK;
				break;
			}
		}

		TWI_StopTransmission(TWI);
	}

	return ErrorCode;
}
