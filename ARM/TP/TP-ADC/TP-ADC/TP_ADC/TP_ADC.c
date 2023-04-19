/*
 * TP_ADC.c
 *
 * Created: 2015-09-27 15:44:23
 *  Author: tmf
 */ 

#include "../TP_ADC/TP_ADC.h"
#include <stddef.h>
#include <stdlib.h>
#include <sam.h>

//Pod³¹czenie TP
//PA4 - AIN4 - X+
//PA5 - AIN5 - X-
//PA6 - AIN6 - Y+
//PA7 - AIN7 - Y-

void Touch_Panel_Rest()
{
	PORT->Group[0].WRCONFIG.reg=PORT_WRCONFIG_WRPINCFG | PORT_WRCONFIG_WRPMUX | 0b11110000; //Wy³¹cz mux dla pinów
	PORT->Group[0].WRCONFIG.reg=PORT_WRCONFIG_WRPINCFG | PORT_WRCONFIG_PULLEN | PORT_WRCONFIG_INEN | 0b00010000; //Wy³¹cz pullup dla PA4
	PORT->Group[0].DIRSET.reg=0b11000000;  //PA 6-7 s¹ wyjœciami, PA4,5 wejœcie
	PORT->Group[0].OUTCLR.reg=0b11000000;  //Y+ i Y- - w stanie niskim
	PORT->Group[0].DIRCLR.reg=0b00110000;  //X+ i X- - wejœcia z podci¹ganiem do 1 - dziêki temu wykryjemy naciœniêcie panela
	PORT->Group[0].OUTSET.reg=0b00110000;
}

void Touch_Panel_Init()
{
	REG_PM_APBCMASK|=PM_APBCMASK_ADC;  //W³¹cz zegar dla ADC
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(GCLK_CLKCTRL_ID_ADC_Val) | //Generic Clock 0
	GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_CLKEN;                   // jest Ÿród³em zegara
	
	REG_ADC_CTRLA=ADC_CTRLA_ENABLE;
	REG_ADC_REFCTRL=ADC_REFCTRL_REFSEL_INTVCC1;  //Vref=Vcc/2
	REG_ADC_SAMPCTRL=63;  //Czas samplowania wejœcia
	REG_ADC_CTRLB=ADC_CTRLB_PRESCALER_DIV128 | ADC_CTRLB_RESSEL_12BIT | ADC_CTRLB_DIFFMODE;  //Tryb 12-bitowy, ró¿nicowy, preskaler 128	
	
	Touch_Panel_Rest();
}

static uint16_t TP_ADCResult(TouchPanel_Cord ctrl_byte)
{
	void PinCfgX()
	{
		PORT->Group[0].OUTCLR.reg=PORT_PA07;
		PORT->Group[0].OUTSET.reg=PORT_PA06;
		PORT->Group[0].DIRCLR.reg=PORT_PA04 | PORT_PA05;
		PORT->Group[0].DIRSET.reg=PORT_PA06 | PORT_PA07;
		PORT->Group[0].WRCONFIG.reg=PORT_WRCONFIG_WRPINCFG | PORT_WRCONFIG_WRPMUX | PORT_WRCONFIG_PMUX(1) | PORT_WRCONFIG_PMUXEN | 0b00010000;
		REG_ADC_INPUTCTRL=ADC_INPUTCTRL_GAIN_DIV2 | ADC_INPUTCTRL_MUXPOS_PIN4 | ADC_INPUTCTRL_MUXNEG_IOGND; //Mierzymy X+
	}
	
	void PinCfgY()
	{
		PORT->Group[0].OUTCLR.reg=PORT_PA05;
		PORT->Group[0].OUTSET.reg=PORT_PA04;
		PORT->Group[0].DIRCLR.reg=PORT_PA06 | PORT_PA07;
		PORT->Group[0].DIRSET.reg=PORT_PA04 | PORT_PA05;
		PORT->Group[0].WRCONFIG.reg=PORT_WRCONFIG_WRPINCFG | PORT_WRCONFIG_WRPMUX | PORT_WRCONFIG_PMUX(1) | PORT_WRCONFIG_PMUXEN | 0b01000000;
		REG_ADC_INPUTCTRL=ADC_INPUTCTRL_GAIN_DIV2 | ADC_INPUTCTRL_MUXPOS_PIN6 | ADC_INPUTCTRL_MUXNEG_IOGND; //Mierzymy Y+
	}
	
	int16_t result, tmpres=0xffff;

	switch(ctrl_byte){
		case TouchPanel_CordX:  PinCfgX(); break;
		case TouchPanel_CordY:  PinCfgY(); break;
	}

	REG_ADC_INTFLAG=ADC_INTFLAG_RESRDY;   //Kasujemy flagê na wszelki wypadek
	do{
		REG_ADC_SWTRIG=ADC_SWTRIG_START; //Wyzwolenie konwersji ADC
		while(!(REG_ADC_INTFLAG & ADC_INTFLAG_RESRDY));  //Zaczekaj na koniec konwersji
		result=tmpres;
		tmpres=REG_ADC_RESULT;               //Odczytaj wynik konwersji
		REG_ADC_INTFLAG=ADC_INTFLAG_RESRDY;  //Kasujemy flagê informuj¹c¹ o zakoñczeniu konwersji
	}while((abs(tmpres-result) > ADC_Noise) || (result > 0x1000));	  //Czekaj na dwa identyczne wyniki

	Touch_Panel_Rest();  //Przywróæ domyœlny stan - detekcji dotyku
	return result;
}

uint16_t TouchPanel_GetPosition(TouchPanel_Cord cord)
{
	return TP_ADCResult(cord);  //Odczytaj ADC
}

void TouchPanel_GetPositionXY(TP_Position *pos)
{
	pos->X=TP_ADCResult(TouchPanel_CordX);  //Najpierw odczytujemy pozycjê X
	pos->Y=TP_ADCResult(TouchPanel_CordY);  //Odbierz pozycjê Y
}