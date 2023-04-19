/*
 * TP_ADC.c
 *
 * Created: 2015-09-27 15:44:23
 *  Author: tmf
 */ 

#include "TP_ADC/TP_ADC.h"
#include <avr/pgmspace.h>
#include <stddef.h>
#include <stdlib.h>

typedef struct  
{
	uint8_t ADCMUX;     //Rejestr multipleksera ADC
	uint8_t PORTSET;    //Rejestr ustawiaj�cy piny IO w stanie 1
	uint8_t PORTCLR;    //Rejestr ustawiaj�cy piny IO w stanie 0
} TP_IO_Cfg;

//Konfiguracja multipleksera ADC dla odczytu wsp. X i Y oraz konfiguracja pin�w IO polaryzuj�cych elektrody panela
static const __flash TP_IO_Cfg Touch_IO_Cfg[]={{.ADCMUX=ADC_CH_MUXPOS_PIN4_gc | ADC_CH_MUXNEG_PIN7_gc, .PORTSET=PIN5_bm, .PORTCLR=PIN7_bm},
	                                           {.ADCMUX=ADC_CH_MUXPOS_PIN5_gc | ADC_CH_MUXNEG_PIN6_gc, .PORTSET=PIN4_bm, .PORTCLR=PIN6_bm},
											   {.PORTCLR=PIN7_bm}};


static uint8_t ReadCalibrationByte(uint8_t index)
{
	uint8_t result;

	NVM_CMD=NVM_CMD_READ_CALIB_ROW_gc; //Odczytaj sygnatur� produkcyjn�
	result=pgm_read_byte(index);

	NVM_CMD=NVM_CMD_NO_OPERATION_gc;   //Przywr�� normalne dzia�anie NVM
	return result;
}

void Touch_Panel_Init()
{
	PORTA_DIRCLR=0b11110000;    //Ustaw jako wej�cia piny ADC PA4-7
	
	ADCA.CTRLB=ADC_IMPMODE_bm | ADC_CURRLIMIT_NO_gc | ADC_CONMODE_bm | ADC_RESOLUTION_12BIT_gc; //Tryb signed, 12-bit
	ADCA.REFCTRL=ADC_REFSEL_INTVCC_gc;                 //Odniesienie Vcc/1.6
	ADCA.PRESCALER=ADC_PRESCALER_DIV64_gc;             //Probkowanie F_CPU/64

	ADCA.CALL=ReadCalibrationByte(offsetof(NVM_PROD_SIGNATURES_t, ADCACAL0));
	ADCA.CALH=ReadCalibrationByte(offsetof(NVM_PROD_SIGNATURES_t, ADCACAL1));
	
	ADCA.CTRLA=ADC_DMASEL_OFF_gc | ADC_ENABLE_bm;
	PORTA_PIN4CTRL=PORT_OPC_PULLUP_gc;   //Wymu� podci�ganie do Vcc na X+ dzi�ki czemu b�dziemy mogli wykry� naci�ni�cie panela
}

static uint16_t TP_ADCResult(TouchPanel_Cord ctrl_byte)
{
	int16_t result, tmpres=0xffff;
	
	PORTA_OUTCLR=Touch_IO_Cfg[ctrl_byte].PORTCLR;  //Zmie� stan wybranych pin�w na 0
	PORTA_OUTSET=Touch_IO_Cfg[ctrl_byte].PORTSET;  //Zmie� stan wybranych pin�w na 1
	PORTA_DIRCLR=0b11110000;    //Piny 4-7 zmieniamy na wej�cia
	PORTA_DIRSET=Touch_IO_Cfg[ctrl_byte].PORTSET | Touch_IO_Cfg[ctrl_byte].PORTCLR; //A teraz wybrane z nich robimy wyj�ciami
	ADCA_CH0_MUXCTRL=Touch_IO_Cfg[ctrl_byte].ADCMUX;   //Skonfiguruj piny IO i ADC do odczytu wybranej osi
	ADCA_CH0_INTFLAGS=ADC_CH_CHIF_bm;     //Kasujemy flag� na wszelki wypadek
	do{
		ADCA_CH0_CTRL=ADC_CH_START_bm | ADC_CH_INPUTMODE_DIFFWGAIN_gc | ADC_CH_GAIN_DIV2_gc;   //Rozpocznij pomiar ADC, tryb r�nicowy, wzmocnienie 1/2
		while(!(ADCA_CH0_INTFLAGS & ADC_CH_CHIF_bm));  //Zaczekaj na koniec konwersji
		result=tmpres;
		tmpres=ADCA_CH0_RES;               //Odczytaj wynik konwersji
		ADCA_CH0_INTFLAGS=ADC_CH_CHIF_bm;  //Kasujemy flag� informuj�c� o zako�czeniu konwersji
	}while((abs(tmpres-result) > ADC_Noise) || (result > 0x1000));	  //Czekaj na dwa identyczne wyniki
	
	PORTA_OUTCLR=Touch_IO_Cfg[2].PORTCLR;  //Wymu� GND na Y-, dzi�ki czemu b�dziemy mogli wykrywa� dotyk poprzez zwarcie X+ do masy
	PORTA_DIRCLR=0b11110000;
	PORTA_DIRSET=Touch_IO_Cfg[2].PORTCLR;
	return result;
}

uint16_t TouchPanel_GetPosition(TouchPanel_Cord cord)
{
	return TP_ADCResult(cord);  //Odczytaj ADC
}

void TouchPanel_GetPositionXY(TP_Position *pos)
{
	pos->X=TP_ADCResult(TouchPanel_CordX);  //Najpierw odczytujemy pozycj� X
	pos->Y=TP_ADCResult(TouchPanel_CordY);  //Odbierz pozycj� Y
}