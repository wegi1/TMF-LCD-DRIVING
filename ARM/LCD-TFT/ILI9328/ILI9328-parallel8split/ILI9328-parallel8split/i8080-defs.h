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

#define i8080_RD		PORT_PA14
#define i8080_WR		PORT_PA13
#define i8080_RS		PORT_PA15
#define i8080_CS		PORT_PA16
#define i8080_RESET		PORT_PA12
#define i8080_D0D7		PORT->Group[1]	//Port z sygna³ami D0-D7
#define i8080_D8D15		PORT->Group[1]	//Port z sygna³ami D8-D15
#define i8080_SIGNALS	PORT->Group[0]	//Port do którego pod³¹czone s¹ sygna³y steruj¹ce magistrali

#define i8080_RDL		1							//Czas trwania stanu niskiego sygna³u RD
#define i8080_RDH		1							//Czas trwania stanu wysokiego sygna³u RD

#endif /* I8080_DEFS_H_ */