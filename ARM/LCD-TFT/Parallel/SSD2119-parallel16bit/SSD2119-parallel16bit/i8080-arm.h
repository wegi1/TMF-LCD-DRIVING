/*
 * i8080_arm.h
 *
 * Created: 2014-02-24 21:17:04
 *  Author: tmf
 */


#ifndef I8080_ARM_H_
#define I8080_ARM_H_

#include "sam.h"
#include "Delay/delay.h"
#include "i8080-defs.h"

static __attribute__((always_inline)) inline void LCD_RS(_Bool state)
{
	if(state) i8080_SIGNALS.OUTSET.reg=i8080_RS;
	else i8080_SIGNALS.OUTCLR.reg=i8080_RS;
}

static __attribute__((always_inline)) inline void LCD_CS(_Bool state)
{
	if(state) i8080_SIGNALS.OUTSET.reg=i8080_CS;
	else i8080_SIGNALS.OUTCLR.reg=i8080_CS;
}

static __attribute__((always_inline)) inline void LCD_WR(_Bool state)
{
	if(state) i8080_SIGNALS.OUTSET.reg=i8080_WR;
	else i8080_SIGNALS.OUTCLR.reg=i8080_WR;
}

static __attribute__((always_inline)) inline void LCD_RD(_Bool state)
{
	if(state) i8080_SIGNALS.OUTSET.reg=i8080_RD;
	else i8080_SIGNALS.OUTCLR.reg=i8080_RD;
}

static __attribute__((always_inline)) inline void i8080_Write_W(uint16_t w16) //Zapisz s³owo, zak³ada, ¿e CS jest aktywny
{
	LCD_WR(0);
	i8080_D0D7.OUT.reg=w16;
	LCD_WR(1);
}

static __attribute__((always_inline)) inline void i8080_Write_B(uint8_t b8)   //Zapisz bajt, zak³ada, ¿e CS jest aktywny
{
	LCD_WR(0);
	i8080_D0D7.OUT.reg=b8;  //W trybie 8-bitowym wykorzystywane s¹ linie D8-D15
	LCD_WR(1);
}

static __attribute__((always_inline)) inline uint16_t i8080_Read_W() //Odczytaj s³owo, zak³ada, ¿e CS jest aktywny
{
	i8080_D0D7.DIRCLR.reg=0xffff;   //Prze³¹czamy port na wejœcie
	LCD_RD(0);
	delay_us(i8080_RDL);    //Dane pojawiaj¹ siê najwczeœniej po 250 ns, a niski stan CS musi trwaæ co najmniej 500 ns
	uint16_t data=i8080_D0D7.IN.reg & 0xffff;
	LCD_RD(1);
	i8080_D0D7.DIRSET.reg=0xffff;  //Z powrotem jest wyjœciem
	return data;
}

static inline void ssd2119_RESET()
{
	i8080_SIGNALS.OUTCLR.reg=i8080_RESET;
	delay_us(40);               //Sygna³ reset musi trwaæ >20 us
	i8080_SIGNALS.OUTSET.reg=i8080_RESET;
}

//Wysy³amy poprzez 8-bitowy interfejs i8080
static __attribute__((always_inline))  inline void ssd2119_SendDataWord(uint16_t data)
{
	LCD_CS(0);
	i8080_Write_W(data);		 //Wyœlij bardziej znacz¹cy bajt danych
	LCD_CS(1);
}

static inline void LCD_SendCmd(uint8_t cmd)
{
	LCD_CS(0);
	LCD_RS(0);
	i8080_Write_W(cmd);
	LCD_RS(1);     //Deaktywuj tryb wysy³ania poleceñ, CS zostawiamy aktywny, bo trzeba wys³aæ dane dla polecenia
}

static inline void LCD_Interface_Init()
{
	i8080_D0D7.DIRSET.reg=0xffff;   //Piny D0-D15 s¹ wyjœciem 
	
	i8080_SIGNALS.OUTSET.reg=i8080_RD | i8080_WR | i8080_CS | i8080_RESET | i8080_RS;
	i8080_SIGNALS.DIRSET.reg=i8080_RD | i8080_WR | i8080_CS | i8080_RESET | i8080_RS;  //Ustaw sygna³y steruj¹ce jako wyjœcia
}

#endif /* I8080_ARM_H_ */