/*
 * RA8875_TP.h
 *
 * Created: 2016-01-09 12:12:58
 *  Author: tmf
 */ 


#ifndef RA8875_TP_H_
#define RA8875_TP_H_

#include "RA8875.h"

//Rejestr kontrolny panela dotykowego nr 0

typedef enum {RA_TP_SampleTime512=0, RA_TP_SampleTime1024, RA_TP_SampleTime2048, RA_TP_SampleTime4096, RA_TP_SampleTime8192,
	          RA_TP_SampleTime16384, RA_TP_SampleTime32768, RA_TP_SampleTime65536} RA_TP_SampleTime;
			  
typedef enum {RA_TP_ADCCLKNoDiv=0, RA_TP_ADCCLKDiv2, RA_TP_ADCCLKDiv4, RA_TP_ADCCLKDiv8, RA_TP_ADCCLKDiv16,
	          RA_TP_ADCCLKDiv32, RA_TP_ADCCLKDiv64, RA_TP_ADCCLKDiv128} RA_TP_ADCClk;

typedef union
{
	struct
	{
		RA_TP_ADCClk ADCClk		    : 3;
		_Bool TPWakeUpEn		    : 1;
		RA_TP_SampleTime SampleTime : 3;
		_Bool TPEnable              : 1;
	};
	uint8_t byte;
} RS8875_TPCR0_Reg;

typedef enum {RA_TP_ModeSel_Idle=0, RA_TP_ModeSel_WaitForTP, RA_TP_ModeSel_LatchX, RA_TP_ModeSel_LatchY} RA_TP_ModeSel;

typedef union
{
	struct
	{
		RA_TP_ModeSel ModeSel	    : 2;
		_Bool DeBounce			    : 1;
		uint8_t IB00			    : 2;
		_Bool ExternalVRef		    : 1;
		_Bool ManualMode            : 1;
		_Bool IB01				    : 1;
	};
	uint8_t byte;
} RS8875_TPCR1_Reg;

//Rejestry konfiguracji panela dotykowego
#define RA_Touch_Panel_Control_Register0								0x70
#define RA_Touch_Panel_Control_Register1								0x71
#define RA_Touch_Panel_X_High_Byte_Data_Register						0x72
#define RA_Touch_Panel_Y_High_Byte_Data_Register						0x73
#define RA_Touch_Panel_X_Y_Low_Byte_Data_Register						0x74

//Zwr�� pozycj� XY punktu
typedef struct
{
	uint16_t X;       //Pozycja X
	uint16_t Y;       //Pozycja Y
} TP_Position;

//Sta�e definiuj�ce pozycje
typedef enum {TouchPanel_CordX, TouchPanel_CordY} TouchPanel_Cord;

void Touch_Panel_Init();          //Zainicjuj TP

uint16_t TouchPanel_GetPosition(TouchPanel_Cord cord);  //Zwraca odczytan� pozycj� X lub Y
_Bool TouchPanel_GetPositionXY(TP_Position *pos);       //Zwraca pozycj� XY naci�ni�tego punktu, lub false, je�li TP nie jest naci�ni�ty


#endif /* RA8875-TP_H_ */