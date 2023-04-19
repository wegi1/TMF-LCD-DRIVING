/*
 * RA8875_Kbd.c
 *
 * Created: 2016-01-23 13:19:33
 *  Author: tmf
 */ 

#include "RA8875-Kbd.h"
#include "RA8875.h"
#include <stdbool.h>

void RA_KBDInit()
{
	RA_SendCmdWithData(RA_Key_Scan_Control_Register1, (RS8875_KSCR1_Reg){.ScanFreq=4, .DebounceSampleTime=RA_KBDDebounce32,
						.LongKeyEn=true, .KeyScanEn=true}.byte);
	RA_SendCmdWithData(RA_Key_Scan_Control_Register2, (RS8875_KSCR2_Reg){.LongKeyTimeAdj=3}.byte);
}

uint8_t RA_GetKeys(uint8_t *k1, uint8_t *k2, uint8_t *k3)
{
	uint8_t KeyNo=KeyNo;
	if(RA_SendCmdReadData(RA_Interrupt_Control_Register2) & ((RS8875_INTC2_Reg){.KeyScanStatus=1}.byte))
		{
			KeyNo=RA_SendCmdReadData(RA_Key_Scan_Control_Register2) & ((RS8875_KSCR2_Reg){.KBDHitNo=-1}.byte);
			*k1=RA_SendCmdReadData(RA_Key_Scan_Data_Register0);
			if(k2) *k2=RA_SendCmdReadData(RA_Key_Scan_Data_Register1);  //Uzupe³niamy kolejne argumenty tylko jeœli nie s¹ NULL
			if(k3) *k3=RA_SendCmdReadData(RA_Key_Scan_Data_Register2);
		}
	RA_SendCmdWithData(RA_Interrupt_Control_Register2, (RS8875_INTC2_Reg){.KeyScanStatus=1}.byte);  //Musimy skasowaæ flagê przerwania
	return KeyNo;
}