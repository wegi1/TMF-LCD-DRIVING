/*
 * FT5x06.h
 *
 * Created: 2016-01-24 13:38:56
 *  Author: tmf
 */ 


#ifndef FT5X06_H_
#define FT5X06_H_

#include <stdint.h>

#define FT5206_Address		0x70		//Adres I2C uk³adu

//Rejestry uk³adu
#define FT_5x06_DEVICE_MODE		0x00
#define FT_5x06_GEST_ID			0x01
#define FT_5x06_TD_STATUS		0x02
#define FT_5x06_TOUCH_XH		0x03	//Kolejne 4 rejestry powtarzach¹ siê piêciokrotnie dla kolejnych punktów dotyku
#define FT_5x06_TOUCH_XL		0x04
#define FT_5x06_TOUCH_YH		0x05
#define FT_5x06_TOUCH_YL		0x06
#define FT_5x06_ID_G_THGROUP	0x80	//Próg detekcji dotyku
#define FT_5x06_ID_G_THPEAK		0x81	//Próg detekcji szczytu dotyku
#define FT_5x06_ID_G_THCAL		0x82	//Próg dla wyliczenia centralnego punktu dotyku
#define FT_5x06_ID_G_THWATER	0x83	//Próg jeœli powierzchni¹ jest woda
#define FT_5x06_ID_G_THTEMP		0x84	//Kompensacja temperatury
#define FT_5x06_ID_G_CTRL		0x86	//Kontrola zasilania
#define FT_5x06_ID_G_TIME_ENTER_MONITOR	0x87
#define FT_5x06_ID_G_PERIODACTIVE	0x88
#define FT_5x06_ID_G_PERIODMONITOR	0x89
#define FT_5x06_ID_G_LIB_VERSION_H	0xA1
#define FT_5x06_ID_G_LIB_VERSION_L	0xA2
#define FT_5x06_ID_G_CIPHER		0xA3
#define FT_5x06_ID_G_MODE		0xA4
#define FT_5x06_ID_G_PMODE		0xA5
#define FT_5x06_ID_G_FIRMID		0xA6
#define FT_5x06_ID_G_STATE		0xA7
#define FT_5x06_ID_G_TFID		0xA8
#define FT_5x06_ID_G_ERR		0xA9
#define FT_5x06_ID_G_CLB		0xAA
#define FT_5x06_ID_G_B_AREAT_TH	0xAE

#define FT_5x06_InterruptEnable		0x00		//Odblokuj liniê INT
#define FT_5x06_InterruptDisable	0x01		//Zablokuj liniê INT

//Typy rozpoznawanych gestów
typedef enum {FT_MoveUp=0x10, FT_MoveLeft=0x14, FT_MoveDown=0x18, FT_MoveRight=0x1C, FT_ZoomIn=0x48, FT_ZoomOut=0x49, FT_NoGesture=0x00} FT5x06_GEST;

typedef union
{
	struct
	{
		uint8_t TouchPointsNo     : 4;		//Liczba naciœniêtych punktów (1-5)
	};
	uint8_t byte;
} FT_Status_Reg;

typedef enum {FT_PutDown, FT_PutUp, FT_Contact, FT_Reserved} FT5x06_Touch;

typedef union
{
	struct
	{
		uint8_t XPOS_MSB     : 4;		//Bity 11:8 pozycji X
		uint8_t Reserved	 : 2;
		FT5x06_Touch Type	 : 2;		//Co siê sta³o?
		uint8_t XPOS_LSB	 : 8;
		uint8_t YPOS_MSB	 : 4;
		uint8_t TouchID		 : 4;
		uint8_t YPOS_LSB	 : 8;
		uint16_t dummy;					//Pomiêdzy kolejnymi punktami s¹ dwa niewykorzystane bajty
	};
	uint8_t byte;
} FT_TouchPoint_Reg;

void CTP_Init();			//Inicjalizacja interfejsu CTP

_Bool CTP_ReadRegister(uint8_t reg, uint8_t *value, uint8_t bytesno);  //Odczytaj bytesno bajtów pocz¹wszy od rejestru o podanym numerze. Zwraca false jeœli coœ jest nie tak
_Bool CTP_WriteRegister(uint8_t reg, uint8_t *value, uint8_t bytesno); //j.w. tylko zapisuje

inline uint16_t FT_5x06_GetX(FT_TouchPoint_Reg XY)
{
	return XY.XPOS_LSB + (XY.XPOS_MSB << 8);
}

inline uint16_t FT_5x06_GetY(FT_TouchPoint_Reg XY)
{
	return XY.YPOS_LSB + (XY.YPOS_MSB << 8);
}

#endif /* FT5X06_H_ */