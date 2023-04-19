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

static __attribute__((always_inline)) inline void i8080_Write_B(uint8_t byte)   //Zapisz bajt, zak³ada, ¿e CS jest aktywny
{
	LCD_WR(0);
	VPORT1_OUT=byte;
	LCD_WR(1);
}

static __attribute__((always_inline)) inline uint8_t i8080_Read_B()   //Odczytaj bajt, zak³ada, ¿e CS jest aktywny
{
	VPORT1_DIR=0x00;   //Prze³¹czamy port na wejœcie
	LCD_RD(0);
	_delay_us(i8080_RDL);    //Dane pojawiaj¹ siê najwczeœniej po 250 ns, a niski stan CS musi trwaæ co najmniej 500 ns
	uint8_t data=VPORT1_IN;
	LCD_RD(1);
	_delay_us(i8080_RDH);
	VPORT1_DIR=0xff;  //Z powrotem jest wyjœciem
	return data;
}

static inline void ssd2119_RESET()
{
	VPORT2_OUT&=~i8080_RESET;
	_delay_us(40);               //Sygna³ reset musi trwaæ >20 us
	VPORT2_OUT|=i8080_RESET;
}

//Wysy³amy poprzez 8-bitowy interfejs i8080
static __attribute__((always_inline))  inline void ssd2119_SendDataWord(uint16_t data)
{
	LCD_CS(0);
	i8080_Write_B(data >> 8);  //Wyœlij bardziej znacz¹cy bajt danych
	i8080_Write_B(data & 0xff);  //Wyœlij mniej znacz¹cy bajt danych
	LCD_CS(1);
}

static inline void LCD_SendCmd(uint8_t cmd)
{
	LCD_CS(0);
	LCD_RS(0);
	i8080_Write_B(0);
	i8080_Write_B(cmd); //Wystaw polecenie na szynê
	LCD_RS(1);     //Deaktywuj tryb wysy³ania poleceñ, CS zostawiamy aktywny, bo trzeba wys³aæ dane dla polecenia
}

static inline void LCD_Interface_Init()
{
	PORTCFG_VPCTRLA=i8080_D8D15;				    //Zmapuj szynê danych na VPORT1
	PORTCFG_VPCTRLB=i8080_SIGNALS;                 //Zmapuj sygna³y steruj¹ce magistral¹
	
	VPORT1_DIR=0xff; 
	
	VPORT2_OUT=i8080_RD | i8080_WR | i8080_CS | i8080_RESET | i8080_RS;
	VPORT2_DIR=i8080_RD | i8080_WR | i8080_CS | i8080_RESET | i8080_RS;  //Ustaw sygna³y steruj¹ce jako wyjœcia
}

#endif /* I8080_XMEGA_H_ */