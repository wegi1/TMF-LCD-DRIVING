/*
 * ssd2119.h
 *
 * Created: 2014-01-26 18:37:55
 *  Author: tmf
 */


#ifndef SSD2119_H_
#define SSD2119_H_

#include <stdint.h>

typedef union
{
	struct
	{
		uint8_t IB00       : 8;    //Bity o warto�ci 0
		uint8_t SS         : 1;		//Kolejno�� wyj�� 0-720 lub 720-0
		uint8_t IB01       : 1;
		uint8_t SM	       : 1;		//Spos�b sterowania driverami
		uint8_t IB02       : 5;		//5 bit�w o warto�ci 0
	};
	uint16_t word;
} ILI9328_DriverOutputControl_Reg;

//Definicja p�l rejestru Entry Mode

typedef union
{
	struct
	{
		uint8_t IB02       : 3;    //Bity o warto�ci 0
		uint8_t AM         : 1;
		uint8_t ID         : 2;
		uint8_t IB06       : 1;		//Bit 0
		uint8_t ORG        : 1;
		uint8_t IB08       : 4;		//4 bity o warto�ci 0
		uint8_t BGR        : 1;
		uint8_t IB13       : 1;		//Bit 0
		uint8_t DFM        : 1;
		uint8_t TRI        : 1;
	};
	uint16_t word;
} ILI9328_EntryMode_Reg;


typedef enum {ILI_NoResizing=0b00, ILI_Resize1to2=0b01, ILI_Resize1to4=0b11} ILI_RSZ;
typedef enum {ILI_0Pixels=0b00, ILI_1Pixels=0b01, ILI_2Pixels=0b10, ILI_3Pixels=0b11} ILI_RCH;
	
typedef union
{
	struct
	{
		ILI_RSZ RSZ        : 2;     // Wsp. skalowania
		uint8_t IB00       : 2;		//
		ILI_RCH RCH        : 2;		//Ile pikseli pozostaje w ka�dej linii 
		uint8_t IB01       : 2;		//
		ILI_RCH RCV	       : 2;		//Ile pikseli pozostaje w ka�dej kolumnie 
		uint8_t IB02       : 6;		//6 bit�w o warto�ci 0
	};
	uint16_t word;
} ILI9328_ResizingControlRegister_Reg;

typedef enum {ILI_DCHalt=0b00, ILI_VCOMGND=0b01, ILI_DisplayOn=0b10, ILI_DisplayOn1=0b1} ILI_DisplayControl;

typedef union
{
	struct
	{
		ILI_DisplayControl	D   : 2;     // Wsp. skalowania
		uint8_t IB00       : 1;		//
		_Bool CL	       : 1;		// 1 - tryb 8 kolorowy, 0 - normalny
		_Bool DTE	       : 1;		// 
		_Bool GON	       : 1;		//DTE=GON=1 - normalne wy�wietlanie
		uint8_t IB01       : 2;		//
		_Bool BASEE		   : 1;		// 1 - wy�wietlaj obraz podstawowy
		uint8_t IB02       : 3;		//
		_Bool PTDE0		   : 1;		// 1 - w��cz wy�wietlanie cz�ciowego obrazu
		_Bool PTDE1		   : 1;		// 1 - w��cz wy�wietlanie cz�ciowego obrazu
		uint8_t IB03       : 2;		//
	};
	uint16_t word;
} ILI9328_DisplayControl1_Reg;

typedef union
{
	struct
	{
		_Bool REV	       : 1;		// Odwr�cenie skali szaro�ci (inwersja obrazu)
		_Bool VLE	       : 1;		// Rozpocz�cie wy�wietlania obrazu pocz�wszy od linii wskazywanej przez rejestr ILI9328_BASEIMAGESCROLL
		_Bool NDL	       : 1;		//Stan drivera w niewy�wietlanych fragmentach obrazu
		uint16_t IB01      : 13;	//
	};
	uint16_t word;
} ILI9328_BaseImageDisplayControl_Reg;

#define ILI_Write_to_GRAM									0x22        //Zapis do GRAM
#define LCDCTRL_IDREGISTER									0x00		//Rejestr ID kontrolera LCD - dla wszystkich kontroler�w ten sam
#define ILI9328_DRIVEROUTPUTCONTROL1						0x01		  // Driver Output Control Register (R01h)
#define ILI9328_LCDDRIVINGCONTROL							0x02	     // LCD Driving Waveform Control (R02h)
#define ILI9328_ENTRYMODE									0x03          // Entry Mode (R03h)
#define ILI9328_ResizingControlRegister						0x04          // Resizing Control Register
#define ILI9328_DISPLAYCONTROL1								0x07
#define ILI9328_DISPLAYCONTROL2								0x08
#define ILI9328_DISPLAYCONTROL3								0x09
#define ILI9328_DISPLAYCONTROL4								0x0a
#define ILI9328_POWERCONTROL1								0x10         // Power Control 1 (R10h)
#define ILI9328_POWERCONTROL2								0x11         // Power Control 2 (R11h)
#define ILI9328_POWERCONTROL3								0x12         // Power Control 3 (R12h)
#define ILI9328_POWERCONTROL4								0x13         // Power Control 4 (R13h)
#define ILI9328_POWERCONTROL7								0x29         // NVM read data 2 (R29h)
#define ILI9328_GAMMACONTROL1								0x30         // Gamma Control 1
#define ILI9328_GAMMACONTROL2								0x31         // Gamma Control 2
#define ILI9328_GAMMACONTROL3								0x32         // Gamma Control 3
#define ILI9328_GAMMACONTROL4								0x35         // Gamma Control 4
#define ILI9328_GAMMACONTROL5								0x36         // Gamma Control 5
#define ILI9328_GAMMACONTROL6								0x37         // Gamma Control 6
#define ILI9328_GAMMACONTROL7								0x38         // Gamma Control 7
#define ILI9328_GAMMACONTROL8								0x39         // Gamma Control 8
#define ILI9328_GAMMACONTROL9								0x3c         // Gamma Control 9
#define ILI9328_GAMMACONTROL10								0x3d         // Gamma Control 10
#define ILI9328_HORIZONTALADDRESSSTARTPOSITION				0x50         // Window Horizontal RAM Address Start (R50h)
#define ILI9328_HORIZONTALADDRESSENDPOSITION				0x51         // Window Horizontal RAM Address End (R51h)
#define ILI9328_VERTICALADDRESSSTARTPOSITION				0x52         // Window Vertical RAM Address Start (R52h)
#define ILI9328_VERTICALADDRESSENDPOSITION					0x53         // Window Vertical RAM Address End (R53h)
#define ILI9328_DRIVEROUTPUTCONTROL2						0x60         // Driver Output Control (R60h)
#define ILI9328_BASEIMAGEDISPLAYCONTROL						0x61         // Driver Output Control (R61h) - enable VLE
#define ILI9328_BASEIMAGESCROLL								0x6A         // Driver Output Control (R6Ah) - scrolling of base image
#define ILI9328_PANELINTERFACECONTROL1						0x90         // Panel Interface Control 1 (R90h)
#define ILI9328_VERTICALGRAMADDRESSSET						0x21		 // GRAM Horizontal Address Set
#define ILI9328_HORIZONTALGRAMADDRESSSET					0x20		 // GRAM Vertical Address Set
#define ILI9328_PartialImage1DisplayPosition				0x80		 // Sets the display start position of partial image 1. The display areas of the partial images 1 and 2 must not overlap each another.
#define ILI9328_PartialImage1RAMStartAddress				0x81
#define ILI9328_PartialImage1RAMEndAddress					0x82
#define ILI9328_PartialImage2DisplayPosition				0x83		 // Sets the display start position of partial image 2. The display areas of the partial images 1 and 2 must not overlap each another.
#define ILI9328_PartialImage2RAMStartAddress				0x84
#define ILI9328_PartialImage2RAMEndAddress					0x85


void ILI_SendCmdWithData(uint8_t cmd, uint16_t data)  __attribute__ ((flatten)); //Wy�lij poecenie i jego parametr do kontrolera
uint16_t ILI_SendCmdReadData(uint8_t cmd)  __attribute__ ((flatten)); //Wy�lij poecenie i odczytaj parametr z rejestru kontrolera

static inline void ILI_RGB565()   //Wybierz przesy� 16-bitowych danych (2 bajty/piksel)
{
	ILI_SendCmdWithData(ILI9328_ENTRYMODE, (ILI9328_EntryMode_Reg){.TRI=0, .DFM=0, .BGR=1, .ID=0b11, .AM=1}.word);  //Wybierz rejestr Entry Mode
}

static inline void ILI_RGB666()  //Wybierz przesy� 18-bitowych danych (3 bajty/piksel)
{
	ILI_SendCmdWithData(ILI9328_ENTRYMODE, (ILI9328_EntryMode_Reg){.TRI=1, .DFM=1, .BGR=1, .ID=0b11, .AM=1}.word);  //Wybierz rejestr Entry Mode
}
#endif /* ILI9328_H_ */