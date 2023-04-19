/*
 * i8080_defs.h
 *
 * Created: 2014-02-24 21:18:34
 *  Author: tmf
 */


#ifndef I8080_DEFS_H_
#define I8080_DEFS_H_

//Pod��czenie LCD:
// PD5 - RESET
// PD0 - RS
// PD1 - CS
// PD2 - RD
// PD3 - WR
// PD4 - WAIT (RA8875)

#define i8080_PORT		PORTD
#define i8080_RD		PIN2_bm
#define i8080_WR		PIN3_bm
#define i8080_RS		PIN0_bm
#define i8080_CS		PIN1_bm
#define i8080_RESET		PIN5_bm
#define RA8875_WAIT		PIN4_bm						//Sygna� WAIT z kontrolera - najlepiej po��czy� go z VPORT2 ze wzgl�du na szybki dost�p

//Sygna�y ��czone z innym portem IO
#define RA8875_PORT		PORTE						//Port do komunikacji z INT z RA8875 i kontrolerem CTP
#define RA8875_INT		PIN4_bm						//Sygna� INT z kontrolera
#define i8080_D0D7		PORTCFG_VP02MAP_PORTB_gc	//Port z sygna�ami D0-D7
#define i8080_D8D15		PORTCFG_VP13MAP_PORTC_gc	//Port z sygna�ami D8-D15
#define i8080_SIGNALS	PORTCFG_VP02MAP_PORTD_gc	//Port do kt�rego pod��czone s� sygna�y steruj�ce magistrali



#define i8080_RDL		0.05						//Czas trwania stanu niskiego sygna�u RD
#define i8080_RDH		0.05						//Czas trwania stanu wysokiego sygna�u RD
#endif /* I8080_DEFS_H_ */