/*
 * bufusart.h
 *
 * Created: 2013-01-24 18:12:26
 *  Author: tmf
 */ 


#ifndef BUFUSART_H_
#define BUFUSART_H_

#include <stdint.h>
#include <stdbool.h>

#define CB_MAXTRANS  100         //Maksymalna liczba elementów bufora 

typedef struct
{
	uint8_t Beg;                       //Pierwszy element bufora
	volatile uint8_t Count;            //Liczba elementów w buforze
	uint8_t *elements;              //Elementy bufora
} CircBuffer;

_Bool cb_Add(CircBuffer *cb, uint8_t elem);  //Dodaj element do bufora
uint8_t cb_Read(CircBuffer *cb);             //Pobie¿ element z bufora

static inline _Bool cb_IsFull(CircBuffer *cb)   //Czy bufor jest pe³ny?
{
	return cb->Count == CB_MAXTRANS;
}

static inline _Bool cb_IsEmpty(CircBuffer *cb)  //Czy bufor jest pusty?
{
	return cb->Count == 0;
}

static inline uint8_t cb_ElementsNo(CircBuffer *cb)  //Ile jest elementów w buforze?
{
	return cb->Count;
}

extern CircBuffer recBuf;        //Bufor odbiorczy
extern CircBuffer sendBuf;       //Bufor nadawczy

void USART_bufs_init();          //Zainicjuj bufory USART

_Bool USART_send_buf(const char __flash *txt);  //false jeœli nie ma miejsca w buforze


#endif /* BUFUSART_H_ */