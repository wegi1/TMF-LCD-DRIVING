/*
 * RA8875_Kbd.h
 *
 * Created: 2016-01-23 13:19:15
 *  Author: tmf
 */ 


#ifndef RA8875_KBD_H_
#define RA8875_KBD_H_

#include <stdint.h>

typedef enum {RA_KBDDebounce4=0, RA_KBDDebounce8=1, RA_KBDDebounce16=2, RA_KBDDebounce32=3} RA_KBDDebounce;

typedef union
{
	struct
	{
		uint8_t ScanFreq					: 3;
		uint8_t IB01						: 1;
		RA_KBDDebounce DebounceSampleTime   : 2;
		_Bool LongKeyEn				        : 1;	
		_Bool KeyScanEn						: 1;
	};
	uint8_t byte;
} RS8875_KSCR1_Reg;

typedef enum {RA_KBDNoHit=0, RA_KBDHitOnce=1, RA_KBDHitTwice=2, RA_KBDThreeTimes=3} RA_KBDHitNo;

typedef union
{
	struct
	{
		RA_KBDHitNo	KBDHitNo				: 2;
		uint8_t LongKeyTimeAdj				: 2;
		uint8_t IB01						: 3;
		_Bool KeyWakeUpEn					: 1;
	};
	uint8_t byte;
} RS8875_KSCR2_Reg;


//Rejestry kontroli klawiatury
#define RA_Key_Scan_Control_Register1									0xC0
#define RA_Key_Scan_Control_Register2									0xC1
#define RA_Key_Scan_Data_Register0										0xC2
#define RA_Key_Scan_Data_Register1										0xC3
#define RA_Key_Scan_Data_Register2										0xC4
#define RA_Extra_General_Purpose_IO_Register							0xC7

void RA_KBDInit();												//W³¹cz obs³ugê klawiatury przez RA8875
uint8_t RA_GetKeys(uint8_t *k1, uint8_t *k2, uint8_t *k3);		//Odczytaj klawisz (wynik ró¿ny od zera) i zapisz do wskazanych zmiennych. k2 lub k3==NULL jeœli s¹ nieu¿ywane

#endif /* RA8875-KBD_H_ */