/*
 * ADS7843.h
 *
 * Created: 2014-04-21 12:34:25
 *  Author: tmf
 */ 


#ifndef ADS7843_H_
#define ADS7843_H_

#include <stdint.h>
#include "calibrate.h"

//Konfiguracja trybów oszczêdzania energii
typedef enum {PowerDown_BetweenConversions, PowerDown_PENIRQDisabled, PowerDown_Reserved, PowerDown_AlwaysPowered} ADS7843_PowerDown;
//Konfiguracja trybu pracy przetwornika
typedef enum {ADS7843_Mode_12bit, ADS7843_Mode_8bit} ADS7843_mode;
//Tryb ró¿nicowy lub z pojedynczym wejœciem
typedef enum {ADS7843_Differential, ADS7843_SingleEnded} ADS7843_InputMode;
//Konfiguracja multipleksera ADC
typedef enum {ADS7843_IN_X=0b101, ADS7843_IN_Y=0b001, ADS7843_IN_IN3=0b010, ADS7843_IN_IN4=0b110} ADS7843_MUX;

typedef union
{
	struct  
	{
		ADS7843_PowerDown PD   : 2;      //Tryb power down	
		ADS7843_InputMode SD   : 1;      //Tryb z wejœciem pojedynczym/ró¿nicowym
		ADS7843_mode      Mode : 1;      //Tryb 8/12 bitowy
		ADS7843_MUX       Mux  : 3;      //Konfiguracja multipleksera
		uint8_t S              : 1;      //Dla bajtu kontrolnego zawsze ma wartoœæ 1
	};
	uint8_t Byte;              //Wartoœæ binarna bajtu kontrolnego
} ADS7843_Cntrl;

//Sta³e definiuj¹ce pozycje
typedef enum {TouchPanel_CordX, TouchPanel_CordY} TouchPanel_Cord;

void Touch_Panel_Init();          //Zainicjuj TP

uint16_t TouchPanel_GetPosition(TouchPanel_Cord cord);  //Zwraca odczytan¹ pozycjê X lub Y
void TouchPanel_GetPositionXY(TP_Position *pos);        //Zwraca pozycjê XY naciœniêtego punktu

#endif /* ADS7843_H_ */