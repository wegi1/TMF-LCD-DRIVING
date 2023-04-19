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

static __attribute__((always_inline)) inline void i8080_Write_B(uint8_t byte)   //Zapisz bajt, zak�ada, �e CS jest aktywny
{
	LCD_WR(0);
	i8080_D0D7.OUT.reg=byte;
	LCD_WR(1);
}

static __attribute__((always_inline)) inline void i8080_Write_W8(uint16_t w16) //Zapisz s�owo, zak�ada, �e CS jest aktywny
{
	LCD_WR(0);
	i8080_D0D7.OUT.reg=w16 >> 8;
	LCD_WR(1);
	LCD_WR(0);
	i8080_D0D7.OUT.reg=w16 & 0xff;
	LCD_WR(1);
}

static __attribute__((always_inline)) inline void i8080_Write_W(uint16_t w16) //Zapisz s�owo, zak�ada, �e CS jest aktywny
{
	LCD_WR(0);
	i8080_D0D7.OUT.reg=w16;
	LCD_WR(1);
}

static __attribute__((always_inline)) inline uint8_t i8080_Read_B()   //Odczytaj bajt, zak�ada, �e CS jest aktywny
{
	i8080_D0D7.DIRCLR.reg=0xff;   //Prze��czamy port na wej�cie
	LCD_RD(0);
	//delay_us(i8080_RDL);    //Odczekaj na dane
	uint8_t data=i8080_D0D7.IN.reg & 0xff;
	LCD_RD(1);
	//delay_us(i8080_RDH);
	i8080_D0D7.DIRSET.reg=0xff;  //Z powrotem jest wyj�ciem
	return data;
}

static __attribute__((always_inline)) inline uint16_t i8080_Read_W8() //Odczytaj s�owo, zak�ada, �e CS jest aktywny
{
	i8080_D0D7.DIRCLR.reg=0xff;   //Prze��czamy port na wej�cie
	LCD_RD(0);
	//delay_us(i8080_RDL);    //Odczekaj na dane
	uint16_t data=(i8080_D0D7.IN.reg & 0xff) << 8;
	LCD_RD(1);
	//delay_us(i8080_RDH);
	LCD_RD(0);
	//delay_us(i8080_RDL);    //DOdczekaj na dane
	data|=(i8080_D0D7.IN.reg & 0xff);
	//delay_us(i8080_RDH);
	LCD_RD(1);
	i8080_D0D7.DIRSET.reg=0xff;  //Z powrotem jest wyj�ciem
	return data;
}

static __attribute__((always_inline)) inline uint16_t i8080_Read_W() //Odczytaj s�owo, zak�ada, �e CS jest aktywny
{
	i8080_D0D7.DIRCLR.reg=0xffff;   //Prze��czamy port na wej�cie
	LCD_RD(0);
	//delay_us(i8080_RDL);    //Odczekaj na dane
	uint16_t data=i8080_D0D7.IN.reg & 0xffff;
	//delay_us(i8080_RDH);
	LCD_RD(1);
	i8080_D0D7.DIRSET.reg=0xffff;  //Z powrotem jest wyj�ciem
	return data;
}

static inline void RA_RESET()
{
	i8080_SIGNALS.OUTCLR.reg=i8080_RESET;
	delay_ms(1);               //Sygna� reset musi trwa� >=1024 takty zegara RA8875
	i8080_SIGNALS.OUTSET.reg=i8080_RESET;
	delay_ms(1);				//A przed wys�aniem pierwszego polecenia musimy odczeka� co najmniej 1 ms
}

//Wysy�amy poprzez 8-bitowy interfejs i8080
static __attribute__((always_inline))  inline void LCD_SendDataWord(uint16_t data)
{
	LCD_CS(0);
	i8080_Write_W(data); 
	LCD_CS(1);
}

static inline void LCD_SendCmd(uint8_t cmd)
{
	LCD_CS(0);
	LCD_RS(0);
	i8080_Write_W(cmd);		//Wystaw polecenie na szyn�
	LCD_RS(1);     //Deaktywuj tryb wysy�ania polece�, CS zostawiamy aktywny, bo trzeba wys�a� dane dla polecenia
}

static inline void LCD_Interface_Init()
{
	i8080_D0D7.DIRSET.reg=0xffff;   //Piny D0-D15 s� wyj�ciem 
	
	i8080_SIGNALS.OUTSET.reg=i8080_RD | i8080_WR | i8080_CS | i8080_RESET | i8080_RS;
	i8080_SIGNALS.DIRSET.reg=i8080_RD | i8080_WR | i8080_CS | i8080_RESET | i8080_RS;  //Ustaw sygna�y steruj�ce jako wyj�cia
}

#endif /* I8080_ARM_H_ */