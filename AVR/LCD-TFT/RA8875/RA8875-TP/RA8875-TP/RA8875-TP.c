/*
 * RA8875_TP.c
 *
 * Created: 2016-01-09 22:35:39
 *  Author: tmf
 */ 

#include "RA8875-TP.h"
#include <stdbool.h>

void Touch_Panel_Init()
{
	RA_SendCmdWithData(RA_Touch_Panel_Control_Register0, (RS8875_TPCR0_Reg){.ADCClk=RA_TP_ADCCLKDiv128, .TPWakeUpEn=true,
		               .SampleTime=RA_TP_SampleTime32768, .TPEnable=true}.byte);
	RA_SendCmdWithData(RA_Touch_Panel_Control_Register1, (RS8875_TPCR1_Reg){.DeBounce=false, .ExternalVRef=false, .ManualMode=false, .ModeSel=RA_TP_ModeSel_LatchY}.byte);
}

uint16_t TouchPanel_GetPosition(TouchPanel_Cord cord)
{
	uint16_t result=result;		//Zapobiega komunikatowi, �e result mo�e by� niezainicjowany
	
	switch(cord)
	{
		case TouchPanel_CordX: result=RA_SendCmdReadData(RA_Touch_Panel_X_High_Byte_Data_Register) << 2;
							   result=result | (RA_SendCmdReadData(RA_Touch_Panel_X_Y_Low_Byte_Data_Register) & 0b0011);
		break;
		case TouchPanel_CordY: result=RA_SendCmdReadData(RA_Touch_Panel_X_High_Byte_Data_Register) << 2;
							   result=result | ((RA_SendCmdReadData(RA_Touch_Panel_X_Y_Low_Byte_Data_Register) & 0b1100) >> 2);
		break;
	}
	
	return result;
}

_Bool TouchPanel_GetPositionXY(TP_Position *pos)
{
	uint8_t HiByte=RA_SendCmdReadData(RA_Touch_Panel_X_Y_Low_Byte_Data_Register);
	pos->X=(RA_SendCmdReadData(RA_Touch_Panel_X_High_Byte_Data_Register) << 2) + (HiByte & 0b0011) ;            //Odbierz pozycję X
	pos->Y=(RA_SendCmdReadData(RA_Touch_Panel_Y_High_Byte_Data_Register) << 2) + ((HiByte & 0x0b1100) >> 2);    //Odbierz pozycję Y
	//_Bool tps=RA_SendCmdReadData(RA_Interrupt_Control_Register2) & (RS8875_INTC2_Reg){.TouchPanelFlag=true}.byte;
	_Bool tps=HiByte & 0x80;
	RA_SendCmdWithData(RA_Interrupt_Control_Register2, (RS8875_INTC2_Reg){.TouchPanelFlag=true}.byte);  //Wyzeruj przerwanie, bez tego nie działa auto mode
	return !tps;
}