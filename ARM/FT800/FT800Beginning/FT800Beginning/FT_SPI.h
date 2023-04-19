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

//Pod³¹czenie LCD:
// PA10 - INT
// PA17 - CS
// PA20 - PDN
//SERCOM1-SPI:
// PA19 - SCK
// PA16 - MISO
// PA18 - MOSI

//Wszystkie sygna³y domyœlnie s¹ na GROUP[0]
#define SPIPORT		    SERCOM1->SPI				//Port do transmisji SPI
#define FT800_CTRL_PORT	PORT->Group[0]			//Dort do którego pod³¹czone s¹ sygna³y steruj¹ce
#define FT800_PDN	PORT_PA20				//Pin PDN
#define FT800_CSel	PORT_PA17             //Pin CS kontrolera
#define FT800_IRQ	PORT_PA10             //Pin IRQ

typedef union
{
	uint32_t UUint32;
	uint8_t  A[4];
} UINT32_B;

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

//Czêstotliwoœæ SPI nie wiêksza ni¿ 10 MHz do inicjalizacji FT800/801
static __attribute__((always_inline)) inline void FT800_SPICLK10M()
{
	_Bool en=SPIPORT.CTRLA.bit.ENABLE;
	if(en) SPIPORT.CTRLA.bit.ENABLE=0;      //Wy³¹cz SERCOM1
	SPIPORT.BAUD.reg=2;  //48MHz/6 = 8 MHz
	SPIPORT.CTRLA.bit.ENABLE=en;      //W³¹cz SERCOM1
}

//Maksymalna dostêpna w MCU szybkoœæ SPI
static __attribute__((always_inline)) inline void FT800_SPICLKMAX()
{
	_Bool en=SPIPORT.CTRLA.bit.ENABLE;
	if(en) SPIPORT.CTRLA.bit.ENABLE=0;      //Wy³¹cz SERCOM1
	SPIPORT.BAUD.reg=0;  //MAX SPI SCK dla D21 to 12 MHz
	SPIPORT.CTRLA.bit.ENABLE=en;      //W³¹cz SERCOM1
}

//Inicjalizacja interfejsu SPI
void USART_init()
{
	FT800_CTRL_PORT.DIRSET.reg=FT800_CSel | FT800_PDN;      //CS, SCK, Txd. CSel i PDN jako wyjœcia
	
	//Konfiguracja SPI
	REG_PM_APBCMASK|=PM_APBCMASK_SERCOM1;  //W³¹cz zegar dla SERCOM1
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(GCLK_CLKCTRL_ID_SERCOM1_CORE_Val) | //Generic Clock 0
	GCLK_CLKCTRL_GEN_GCLK0 |                            // jest Ÿród³em zegara
	GCLK_CLKCTRL_CLKEN;
	
	while(GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY); //Zaczekaj na zynchronizacjê
	
	SPIPORT.CTRLA.bit.ENABLE=0;      //Wy³¹cz SERCOM1
	while(SPIPORT.SYNCBUSY.bit.ENABLE);
	SPIPORT.CTRLA.bit.SWRST=1;      //Zresetuj SERCOM1
	while(SPIPORT.CTRLA.bit.SWRST || SPIPORT.SYNCBUSY.bit.SWRST);
	
	FT800_SPICLKMAX();  //Zegar SPI max - 12 MHz
	SPIPORT.CTRLB.reg=SERCOM_SPI_CTRLB_RXEN; //Odblokuj odbiornik SPI, ramka 8 bitowa, programowa kontrola SS
	while(SPIPORT.SYNCBUSY.reg & SERCOM_SPI_SYNCBUSY_MASK);  //Zaczekaj na synchreonizacjê zapisu rejestrów
	
	FT800_CTRL_PORT.WRCONFIG.reg=PORT_WRCONFIG_HWSEL | PORT_WRCONFIG_WRPINCFG | PORT_WRCONFIG_WRPMUX | PORT_WRCONFIG_PMUX(2) | PORT_WRCONFIG_PMUXEN | 0b1101; //Wybierz funkcjê SERCOM1 dla PA16-19
	
	SPIPORT.CTRLA.reg=SERCOM_SPI_CTRLA_ENABLE | SERCOM_SPI_CTRLA_MODE_SPI_MASTER | SERCOM_SPI_CTRLA_DOPO(1) | SERCOM_SPI_CTRLA_RUNSTDBY; //Tryb master SPI, Mode 0, MSB, PAD0 - MISO
	while(SPIPORT.SYNCBUSY.reg & SERCOM_SPI_SYNCBUSY_MASK);  //Zaczekaj na synchreonizacjê zapisu rejestrów
}

//Funkcja wysy³a/odbiera bajt z SPI, przy czym czeka na zakoñczenie wysy³ki
uint8_t FT800_SPIRW(uint8_t ch)
{
	SPIPORT.DATA.reg=ch;
	while(!(SPIPORT.INTFLAG.reg & SERCOM_SPI_INTFLAG_TXC));  //Zaczekaj na wys³anie danych
	return SPIPORT.DATA.reg;   //Odczytaj bajt i skasuj flagê
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
void FT800_WritefromFLASH(uint32_t Addr, const uint8_t *Src, uint32_t NBytes)
{
	uint32_t i;
	FT800_StartWrite(Addr);
	for(i=0; i<NBytes; i++) FT800_SPIRW(*Src++);
	FT800_CS(false);
}

#endif /* FT_SPI_H_ */