/*
 * i8080_xmega.h
 *
 * Created: 2014-02-24 21:17:04
 *  Author: tmf
 */


#ifndef I8080_XMEGA_H_
#define I8080_XMEGA_H_

#include <avr/io.h>
#include <util/delay.h>
#include "i8080-defs.h"

static __attribute__((always_inline)) inline void LCD_RS(_Bool state)
{
	if(state) VPORT2_OUT|=i8080_RS;
	else VPORT2_OUT&=~i8080_RS;
}

static __attribute__((always_inline)) inline void LCD_CS(_Bool state)
{
	if(state) VPORT2_OUT|=i8080_CS;
	else VPORT2_OUT&=~i8080_CS;
}

static __attribute__((always_inline)) inline void LCD_WR(_Bool state)
{
	if(state) VPORT2_OUT|=i8080_WR;
	else VPORT2_OUT&=~i8080_WR;
}

static __attribute__((always_inline)) inline void LCD_RD(_Bool state)
{
	if(state) VPORT2_OUT|=i8080_RD;
	else VPORT2_OUT&=~i8080_RD;
}

static __attribute__((always_inline)) inline void i8080_Write_B(uint8_t byte)   //Zapisz bajt, zak�ada, �e CS jest aktywny
{
	LCD_WR(0);
	VPORT0_OUT=byte;
	LCD_WR(1);
}

static __attribute__((always_inline)) inline void i8080_Write_W8(uint16_t w16) //Zapisz s�owo, zak�ada, �e CS jest aktywny
{
	LCD_WR(0);
	VPORT0_OUT=w16 >> 8;
	LCD_WR(1);
	LCD_WR(0);
	VPORT0_OUT=w16 & 0xff;
	LCD_WR(1);
}

static __attribute__((always_inline)) inline void i8080_Write_W(uint16_t w16) //Zapisz s�owo, zak�ada, �e CS jest aktywny
{
	LCD_WR(0);
	VPORT1_OUT=w16 >> 8;
	VPORT0_OUT=w16 & 0xff;
	LCD_WR(1);
}


static __attribute__((always_inline)) inline uint8_t i8080_Read_B()   //Odczytaj bajt, zak�ada, �e CS jest aktywny
{
	VPORT0_DIR=0x00;   //Prze��czamy port na wej�cie
	LCD_RD(0);
	_delay_us(i8080_RDL);    //Odczekaj na dane
	uint8_t data=VPORT0_IN;
	LCD_RD(1);
	_delay_us(i8080_RDH);
	VPORT0_DIR=0xff;  //Z powrotem jest wyj�ciem
	return data;
}

static __attribute__((always_inline)) inline uint16_t i8080_Read_W8() //Odczytaj s�owo, zak�ada, �e CS jest aktywny
{
	VPORT0_DIR=0x00;   //Prze��czamy port na wej�cie
	LCD_RD(0);
	_delay_us(i8080_RDL);    //Odczekaj na dane
	uint16_t data=VPORT0_IN << 8;
	LCD_RD(1);
	_delay_us(i8080_RDH);
	LCD_RD(0);
	_delay_us(i8080_RDL);    //DOdczekaj na dane
	data|=VPORT0_IN;
	_delay_us(i8080_RDH);
	LCD_RD(1);
	VPORT0_DIR=0xff;  //Z powrotem jest wyj�ciem
	return data;
}

static __attribute__((always_inline)) inline uint16_t i8080_Read_W() //Odczytaj s�owo, zak�ada, �e CS jest aktywny
{
	VPORT0_DIR=0x00;   //Prze��czamy port na wej�cie
	VPORT1_DIR=0x00;   //Prze��czamy port na wej�cie
	LCD_RD(0);
	_delay_us(i8080_RDL);    //Odczekaj na dane
	uint16_t data=VPORT1_IN << 8;
	data|=VPORT0_IN;
	_delay_us(i8080_RDH);
	LCD_RD(1);
	VPORT0_DIR=0xff;  //Z powrotem jest wyj�ciem
	VPORT1_DIR=0xff;  //Z powrotem jest wyj�ciem
	return data;
}

static inline void RA_RESET()
{
	VPORT2_OUT&=~i8080_RESET;
	_delay_ms(1);               //Sygna� reset musi trwa� >=1024 takty zegara RA8875
	VPORT2_OUT|=i8080_RESET;
	_delay_ms(1);				//A przed wys�aniem pierwszego polecenia musimy odczeka� co najmniej 1 ms
}

//Wysy�amy poprzez 8-bitowy interfejs i8080
static __attribute__((always_inline))  inline void ssd2119_SendDataWord(uint16_t data)
{
	LCD_CS(0);
	i8080_Write_W(data);  
	LCD_CS(1);
}

static inline void LCD_SendCmd(uint8_t cmd)
{
	LCD_CS(0);
	LCD_RS(1);
	i8080_Write_W(cmd);
	LCD_RS(0);		//Deaktywuj tryb wysy�ania polece�, CS zostawiamy aktywny, bo trzeba wys�a� dane dla polecenia
}

static inline void LCD_Interface_Init()
{
	PORTCFG_VPCTRLA=i8080_D0D7 | i8080_D8D15;      //Zmapuj szyn� danych na VPORT0 i VPORT1
	//PORTCFG_VPCTRLA=i8080_D8D15;				   //Zmapuj szyn� danych na VPORT1
	PORTCFG_VPCTRLB=i8080_SIGNALS;                 //Zmapuj sygna�y steruj�ce magistral�
	
	VPORT0_OUT=0x00;		//Te dwie instrukcje ustawiaj� bity D0-D7 kontrolera w stan niski co jest wymagane przez not�
	VPORT0_DIR=0xff;		//Je�li ich nie ��czymy z MCU mo�na te instrukcje pomin��
	VPORT1_DIR=0xff;		//Port steruj�cy liniami D8-D15
	
	VPORT2_OUT=i8080_RD | i8080_WR | i8080_CS | i8080_RESET;
	VPORT2_DIR=i8080_RD | i8080_WR | i8080_CS | i8080_RESET | i8080_RS;  //Ustaw sygna�y steruj�ce jako wyj�cia
}

#endif /* I8080_XMEGA_H_ */