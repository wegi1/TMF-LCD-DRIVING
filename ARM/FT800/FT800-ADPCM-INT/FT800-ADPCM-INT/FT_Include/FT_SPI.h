/*
 * FT_SPI.h
 *
 * Created: 2015-01-02 10:29:25
 *  Author: tmf
 *Podstawowe funkcje specyficzne dla hardware do komunikacji z FT800/801
 */ 


#ifndef FT_SPI_H_
#define FT_SPI_H_

#include "sam.h"
#include <stdbool.h>

//Pod��czenie LCD:
// PA10 - INT
// PA17 - CS
// PA20 - PDN
//SERCOM1-SPI:
// PA19 - SCK
// PA16 - MISO
// PA18 - MOSI

//Wszystkie sygna�y domy�lnie s� na GROUP[0]
#define SPIPORT		    SERCOM1->SPI				//Port do transmisji SPI
#define FT800_CTRL_PORT	PORT->Group[0]			//Dort do kt�rego pod��czone s� sygna�y steruj�ce
#define FT800_PDN	PORT_PA20				//Pin PDN
#define FT800_CSel	PORT_PA17             //Pin CS kontrolera
#define FT800_IRQ	PORT_PA10             //Pin IRQ


void FT800_CS(_Bool state)
{
	if(state) FT800_CTRL_PORT.OUTCLR.reg=FT800_CSel;   //Aktywuj CS
	else FT800_CTRL_PORT.OUTSET.reg=FT800_CSel;        //Deaktywuj CS
}

void FT800_PD(_Bool state)
{
	if(state) FT800_CTRL_PORT.OUTSET.reg=FT800_PDN;   //Aktywuj PDN
	else FT800_CTRL_PORT.OUTCLR.reg=FT800_PDN;        //Deaktywuj PDN
}

//Cz�stotliwo�� SPI nie wi�ksza ni� 10 MHz do inicjalizacji FT800/801
static __attribute__((always_inline)) inline void FT800_SPICLK10M()
{
	_Bool en=SPIPORT.CTRLA.bit.ENABLE;
	if(en) SPIPORT.CTRLA.bit.ENABLE=0;      //Wy��cz SERCOM1
	SPIPORT.BAUD.reg=2;  //48MHz/6 = 8 MHz
	SPIPORT.CTRLA.bit.ENABLE=en;      //W��cz SERCOM1
}

//Maksymalna dost�pna w MCU szybko�� SPI
static __attribute__((always_inline)) inline void FT800_SPICLKMAX()
{
	_Bool en=SPIPORT.CTRLA.bit.ENABLE;
	if(en) SPIPORT.CTRLA.bit.ENABLE=0;      //Wy��cz SERCOM1
	SPIPORT.BAUD.reg=0;  //MAX SPI SCK dla D21 to 12 MHz
	SPIPORT.CTRLA.bit.ENABLE=en;      //W��cz SERCOM1
}

//Inicjalizacja interfejsu SPI
void USART_init()
{
	FT800_CTRL_PORT.DIRSET.reg=FT800_CSel | FT800_PDN;      //CS, SCK, Txd. CSel i PDN jako wyj�cia
	
	//Konfiguracja SPI
	REG_PM_APBCMASK|=PM_APBCMASK_SERCOM1;  //W��cz zegar dla SERCOM1
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(GCLK_CLKCTRL_ID_SERCOM1_CORE_Val) | //Generic Clock 0
	GCLK_CLKCTRL_GEN_GCLK0 |                            // jest �r�d�em zegara
	GCLK_CLKCTRL_CLKEN;
	
	while(GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY); //Zaczekaj na synchronizacj�
	
	SPIPORT.CTRLA.bit.ENABLE=0;      //Wy��cz SERCOM1
	while(SPIPORT.SYNCBUSY.bit.ENABLE);
	SPIPORT.CTRLA.bit.SWRST=1;      //Zresetuj SERCOM1
	while(SPIPORT.CTRLA.bit.SWRST || SPIPORT.SYNCBUSY.bit.SWRST);
	
	FT800_SPICLKMAX();  //Zegar SPI max - 12 MHz
	SPIPORT.CTRLB.reg=SERCOM_SPI_CTRLB_RXEN; //Odblokuj odbiornik SPI, ramka 8 bitowa, programowa kontrola SS
	while(SPIPORT.SYNCBUSY.reg & SERCOM_SPI_SYNCBUSY_MASK);  //Zaczekaj na synchreonizacj� zapisu rejestr�w
	
	FT800_CTRL_PORT.WRCONFIG.reg=PORT_WRCONFIG_HWSEL | PORT_WRCONFIG_WRPINCFG | PORT_WRCONFIG_WRPMUX | PORT_WRCONFIG_PMUX(2) | PORT_WRCONFIG_PMUXEN | 0b1101; //Wybierz funkcj� SERCOM1 dla PA16-19
	
	SPIPORT.CTRLA.reg=SERCOM_SPI_CTRLA_ENABLE | SERCOM_SPI_CTRLA_MODE_SPI_MASTER | SERCOM_SPI_CTRLA_DOPO(1) | SERCOM_SPI_CTRLA_RUNSTDBY; //Tryb master SPI, Mode 0, MSB, PAD0 - MISO
	while(SPIPORT.SYNCBUSY.reg & SERCOM_SPI_SYNCBUSY_MASK);  //Zaczekaj na synchreonizacj� zapisu rejestr�w
	
#ifdef FT_INT
	FT800_CTRL_PORT.DIRCLR.reg=FT800_IRQ; //Wej�cie INT TP
	FT800_CTRL_PORT.WRCONFIG.reg=FT800_IRQ | PORT_WRCONFIG_WRPINCFG | PORT_WRCONFIG_WRPMUX | PORT_WRCONFIG_PMUX(0) | PORT_WRCONFIG_PMUXEN |PORT_WRCONFIG_INEN | PORT_WRCONFIG_PULLEN; //Odblokowujemy pin i w��czamy pullup oraz EXTINT10
	
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(GCLK_CLKCTRL_ID_EIC_Val) | //Generic Clock 0
	GCLK_CLKCTRL_GEN_GCLK0 |                            // jest �r�d�em zegara dla EIC
	GCLK_CLKCTRL_CLKEN;
	
	while(GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY); //Zaczekaj na synchronizacj�
	
	EIC->CONFIG[1].bit.SENSE2=EIC_CONFIG_SENSE0_FALL_Val;  //Zbocze opadaj�ce wywo�a przerwanie EXTINT10
	EIC->INTENSET.bit.EXTINT10=1;  //W��cz przerwanie EXTINT10
	EIC->CTRL.bit.ENABLE=1;
	NVIC_EnableIRQ(EIC_IRQn);   //Zezw�l na przerwania EIC w NVIC
#endif
}

//Funkcja wysy�a/odbiera bajt z SPI, przy czym czeka na zako�czenie wysy�ki
uint8_t FT800_SPIRW(uint8_t ch)
{
	SPIPORT.DATA.reg=ch;
	while(!(SPIPORT.INTFLAG.reg & SERCOM_SPI_INTFLAG_TXC));  //Zaczekaj na wys�anie danych
	return SPIPORT.DATA.reg;   //Odczytaj bajt i skasuj flag�
}

#endif /* FT_SPI_H_ */