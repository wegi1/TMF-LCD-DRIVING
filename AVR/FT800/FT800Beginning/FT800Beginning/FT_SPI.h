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

//Wszystkie sygna³y domyœlnie s¹ na PORTE
#define SPIPORT		USARTE0				//Port do transmisji SPI
#define FT800_CTRL_PORT	PORTE			//Dort do którego pod³¹czone s¹ sygna³y steruj¹ce
#define FT800_PDN	PIN5_bm				//Pin PDN
#define FT800_CSel	PIN0_bm             //Pin CS kontrolera
#define FT800_IRQ	PIN4_bm             //Pin IRQ

typedef union
{
	uint32_t UUint32;
	uint8_t  A[4];
} UINT32_B;

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
	FT800_CTRL_PORT.DIRSET=PIN1_bm | PIN3_bm | FT800_CSel | FT800_PDN;      //CS, SCK, Txd. CSel i PDN jako wyjœcia

	SPIPORT.CTRLC=USART_CMODE_MSPI_gc;            //SPI Mode 0
	//Szybkoœæ transmisji 2 Mbps, jeden bit trwa ok. 500 ns - pamiêtaj, aby nie korzystaæ z generatora frakcyjnego
	SPIPORT.BAUDCTRLA=0;         //Dla taktowania 2 MHz, SPI taktowane 1 MHz
	SPIPORT.BAUDCTRLB=0;
	SPIPORT.CTRLB=USART_TXEN_bm | USART_RXEN_bm;  //W³¹cz nadajnik i odbiornik USART
}

//Funkcja wysy³a/odbiera bajt z SPI, przy czym czeka na zakoñczenie wysy³ki
uint8_t FT800_SPIRW(uint8_t ch)
{
	SPIPORT.DATA=ch;
	while(!(SPIPORT.STATUS & USART_TXCIF_bm));  //Zaczekaj na wys³anie danych
	SPIPORT.STATUS=USART_TXCIF_bm;              //Skasuj flagê
	return SPIPORT.DATA;
}

//Czêstotliwoœæ SPI nie wiêksza ni¿ 10 MHz do inicjalizacji FT800/801
void FT800_SPICLK10M()
{
	SPIPORT.BAUDCTRLA=0;
	SPIPORT.BAUDCTRLB=0;	
}

//Maksymalna dostêpna w MCU szybkoœæ SPI lecz nie wiêcej ni¿ 30 MHz
void FT800_SPICLKMAX()
{
	SPIPORT.BAUDCTRLA=0;
	SPIPORT.BAUDCTRLB=0;
}

//Rozpocznij odczyt spod wskazanej lokacji
void FT800_StartRead(uint32_t Addr)
{
	UINT32_B UUint32;
	UUint32.UUint32 = Addr;
	FT800_CS(true);
	FT800_SPIRW(UUint32.A[2]);
	FT800_SPIRW(UUint32.A[1]);
	FT800_SPIRW(UUint32.A[0]);
	FT800_SPIRW(0x00);            //Aby coœ odczytaæ musimy wys³aæ dodatkowy bajt, bez znaczenia jaki
}

//Rozpocznij zapis pod wskazan¹ lokacjê
void FT800_StartWrite(uint32_t Addr)
{
	UINT32_B UUint32;
	UUint32.UUint32 = Addr;
	FT800_CS(true);
	FT800_SPIRW(UUint32.A[2] | 0x80);
	FT800_SPIRW(UUint32.A[1]);
	FT800_SPIRW(UUint32.A[0]);
}
	
//Odczytaj bajt spod wskazanej lokacji
uint8_t FT800_Read(uint32_t Addr)
{
	uint8_t ReadByte;
	FT800_StartRead(Addr);
	ReadByte = FT800_SPIRW(0x00);
	FT800_CS(false);
	return ReadByte;
}
	
//Odczytaj s³owo spod wskazanej lokacji
uint16_t FT800_Read16(uint32_t Addr)
{
	FT800_StartRead(Addr);
	uint16_t retval;
	
	retval = FT800_SPIRW(0x00);
	retval |= FT800_SPIRW(0x00) <<8;
	FT800_CS(false);
	return retval;
}

//Odczytaj podwójne s³owo spod wskazanej lokacji
uint32_t FT800_Read32(uint32_t Addr)
{
	UINT32_B retval;
	FT800_StartRead(Addr);
	retval.A[0] = FT800_SPIRW(0x00);
	retval.A[1] = FT800_SPIRW(0x00);
	retval.A[2] = FT800_SPIRW(0x00);
	retval.A[3] = FT800_SPIRW(0x00);
	FT800_CS(false);
	return retval.UUint32;
}

//Odczytaj n-bajtów spod wskazanej lokacji i umieœæ jee w tablicy Src
void FT800_ReadN(uint32_t Addr, uint8_t *Src, uint32_t NBytes)
{
	uint32_t i;
	FT800_StartRead(Addr);
	for(i=0; i<NBytes; i++)
	*Src++ = FT800_SPIRW(0x00);
	FT800_CS(false);
}
	
//Zapisz bajt pod wskazan¹ lokacjê
void FT800_Write(uint32_t Addr, uint8_t Value8)
{
	FT800_StartWrite(Addr);
	FT800_SPIRW(Value8);
	FT800_CS(false);
}

//Zapisz s³owo pod wskazan¹ lokacjê
void FT800_Write16(uint32_t Addr, uint16_t Value16)
{
	FT800_StartWrite(Addr);
	FT800_SPIRW(Value16 & 0xFF);
	FT800_SPIRW((Value16 >> 8) & 0xff);
	FT800_CS(false);
}

//Zapisz podwójne s³owo pod wskazan¹ lokacjê
void FT800_Write32(uint32_t Addr, uint32_t Value32)
{
	UINT32_B UUint32;
	UUint32.UUint32 = Value32;
	FT800_StartWrite(Addr);
	FT800_SPIRW(UUint32.A[0]);
	FT800_SPIRW(UUint32.A[1]);
	FT800_SPIRW(UUint32.A[2]);
	FT800_SPIRW(UUint32.A[3]);
	FT800_CS(false);
}

void FT800_Write32b(uint32_t Value32)
{
	UINT32_B UUint32;
	UUint32.UUint32 = Value32;
	FT800_SPIRW(UUint32.A[0]);
	FT800_SPIRW(UUint32.A[1]);
	FT800_SPIRW(UUint32.A[2]);
	FT800_SPIRW(UUint32.A[3]);
}

//Zapisz n-bajtów z Src pod wskazan¹ lokacjê
void FT800_WriteN(uint32_t Addr, uint8_t *Src, uint32_t NBytes)
{
	uint32_t i;
	FT800_StartWrite(Addr);
	for(i=0; i<NBytes; i++)  FT800_SPIRW(*Src++);
	FT800_CS(false);
}
	
//Zapisz n-bajtów znajduj¹cych siê w FLASH pod wskazan¹ lokacjê	
void FT800_WritefromFLASH(uint32_t Addr, const uint8_t __flash *Src, uint32_t NBytes)
{
	uint32_t i;
	FT800_StartWrite(Addr);
	for(i=0; i<NBytes; i++) FT800_SPIRW(*Src++);
	FT800_CS(false);
}

#endif /* FT_SPI_H_ */