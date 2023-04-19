/*
 * FT_SPI.h
 *
 * Created: 2015-01-02 10:29:25
 *  Author: tmf
 *Podstawowe funkcje specyficzne dla hardware do komunikacji z FT800/801
 */ 


#ifndef FT_SPI_H_
#define FT_SPI_H_

#include <avr/io.h>
#include <stdbool.h>

//Wszystkie sygna�y domy�lnie s� na PORTE
#define SPIPORT		USARTE0				//Port do transmisji SPI
#define FT800_CTRL_PORT	PORTE			//Dort do kt�rego pod��czone s� sygna�y steruj�ce
#define FT800_PDN	PIN5_bm				//Pin PDN
#define FT800_CSel	PIN0_bm             //Pin CS kontrolera
#define FT800_IRQ	PIN4_bm             //Pin IRQ

void FT800_CS(_Bool state)
{
	if(state) FT800_CTRL_PORT.OUTCLR=FT800_CSel;   //Aktywuj CS
	else FT800_CTRL_PORT.OUTSET=FT800_CSel;        //Deaktywuj CS
}

void FT800_PD(_Bool state)
{
	if(state) FT800_CTRL_PORT.OUTSET=FT800_PDN;   //Aktywuj PDN
	else FT800_CTRL_PORT.OUTCLR=FT800_PDN;        //Deaktywuj PDN
}

//Inicjalizacja interfejsu SPI
void USART_init()
{
	FT800_CTRL_PORT.DIRSET=PIN1_bm | PIN3_bm | FT800_CSel | FT800_PDN;      //CS, SCK, Txd. CSel i PDN jako wyj�cia

	SPIPORT.CTRLC=USART_CMODE_MSPI_gc;            //SPI Mode 0
	//Szybko�� transmisji 2 Mbps, jeden bit trwa ok. 500 ns - pami�taj, aby nie korzysta� z generatora frakcyjnego
	SPIPORT.BAUDCTRLA=0;         //Dla taktowania 2 MHz, SPI taktowane 1 MHz
	SPIPORT.BAUDCTRLB=0;
	SPIPORT.CTRLB=USART_TXEN_bm | USART_RXEN_bm;  //W��cz nadajnik i odbiornik USART
}

//Funkcja wysy�a/odbiera bajt z SPI, przy czym czeka na zako�czenie wysy�ki
uint8_t FT800_SPIRW(uint8_t ch)
{
	SPIPORT.DATA=ch;
	while(!(SPIPORT.STATUS & USART_TXCIF_bm));  //Zaczekaj na wys�anie danych
	SPIPORT.STATUS=USART_TXCIF_bm;              //Skasuj flag�
	return SPIPORT.DATA;
}

//Cz�stotliwo�� SPI nie wi�ksza ni� 10 MHz do inicjalizacji FT800/801
void FT800_SPICLK10M()
{
	SPIPORT.BAUDCTRLA=0;
	SPIPORT.BAUDCTRLB=0;	
}

//Maksymalna dost�pna w MCU szybko�� SPI lecz nie wi�cej ni� 30 MHz
void FT800_SPICLKMAX()
{
	SPIPORT.BAUDCTRLA=0;
	SPIPORT.BAUDCTRLB=0;
}

#endif /* FT_SPI_H_ */