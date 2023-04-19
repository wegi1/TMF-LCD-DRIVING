/*
 * i8080_defs.h
 *
 * Created: 2014-02-24 21:18:34
 *  Author: tmf
 */


#ifndef I8080_DEFS_H_
#define I8080_DEFS_H_

//Pod³¹czenie LCD:
// PD5 - RESET
// PD0 - RS
// PD1 - CS
// PD2 - RD
// PD3 - WR

#define i8080_RD		PIN2_bm
#define i8080_WR		PIN3_bm
#define i8080_RS		PIN0_bm
#define i8080_CS		PIN1_bm
#define i8080_RESET		PIN5_bm
#define i8080_D0D7		PORTCFG_VP02MAP_PORTB_gc	//Port z sygna³ami D0-D7
#define i8080_D8D15		PORTCFG_VP13MAP_PORTC_gc	//Port z sygna³ami D8-D15
#define i8080_SIGNALS	PORTCFG_VP02MAP_PORTD_gc	//Port do którego pod³¹czone s¹ sygna³y steruj¹ce magistrali

#define i8080_RDL		0.5							//Czas trwania stanu niskiego sygna³u RD
#define i8080_RDH		0.5							//Czas trwania stanu wysokiego sygna³u RD

#endif /* I8080_DEFS_H_ */