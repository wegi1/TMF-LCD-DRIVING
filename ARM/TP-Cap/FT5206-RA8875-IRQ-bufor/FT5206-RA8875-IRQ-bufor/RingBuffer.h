/*
 * RingBuffer.h
 *
 * Created: 2013-01-22 23:10:31
 *  Author: tmf
 */ 


#ifndef RINGBUFFER_H_
#define RINGBUFFER_H_

#include <stdint.h>
#include <stdbool.h>
#include "FT5x06.h"

#define CB_MAXTRANS  40         //Maksymalna liczba elementów bufora

typedef FT_TouchPoint_Reg CB_Element;     //Typ elementów w buforze

typedef struct
{
	uint8_t Beg;                       //Pierwszy element bufora
	volatile uint8_t Count;            //Liczba elementów w buforze
	CB_Element elements[CB_MAXTRANS];  //Elementy bufora
} CircBuffer;

_Bool cb_Add(CircBuffer *cb, CB_Element elem);		//Dodaj element do kolejki, jeœli true to ok, jeœli false to nie uda³o siê dodaæ elementu (kolejka pe³na)
_Bool cb_Read(CircBuffer *cb, CB_Element *elem);	//Pobierz element z kolejki, jeœli true to ok, jeœli false to nie ma elementu

static inline _Bool cb_IsFull(CircBuffer *cb)
{
	return cb->Count == CB_MAXTRANS;
}

static inline _Bool cb_IsEmpty(CircBuffer *cb)
{
	return cb->Count == 0;
}

#endif /* RINGBUFFER_H_ */