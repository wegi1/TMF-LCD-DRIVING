/*
 * TP_ADC.h
 *
 * Created: 2015-09-27 15:42:20
 *  Author: tmf
 */ 


#ifndef TP_ADC_H_
#define TP_ADC_H_

#include "../TP_ADC/calibrate.h"

//Sta�e definiuj�ce pozycje
typedef enum {TouchPanel_CordX, TouchPanel_CordY, TouchPanel_Z1, TouchPanel_Z2} TouchPanel_Cord;

static const uint8_t ADC_Noise=12;      //Dopuszczalna r�nica kolejnych odczyt�w ADC, aby uzna� je za poprawne

void Touch_Panel_Init();          //Zainicjuj TP

uint16_t TouchPanel_GetPosition(TouchPanel_Cord cord);  //Zwraca odczytan� pozycj� X lub Y
void TouchPanel_GetPositionXY(TP_Position *pos);        //Zwraca pozycj� XY naci�ni�tego punktu

uint16_t TouchPanel_GetPositionAndTouchXY(TP_Position *pos);   //Zwraca pozycj� XY naci�ni�tego punktu oraz si�� nacisku

#endif /* TP_ADC_H_ */