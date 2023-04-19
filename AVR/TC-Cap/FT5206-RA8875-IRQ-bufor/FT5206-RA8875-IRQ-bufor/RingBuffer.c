/*
 * RingBuffer.c
 *
 * Created: 2013-01-22 23:09:49
 *  Author: tmf
 */ 

#include "RingBuffer.h"

#include <util/atomic.h>
#include <stdbool.h>
#include <string.h>


_Bool cb_Add(CircBuffer *cb, CB_Element elem)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		if(cb_IsFull(cb)) return false;         //Czy jest miejsce w kolejce?
		uint8_t end = (cb->Beg + cb->Count) % CB_MAXTRANS;
		memcpy(&cb->elements[end * sizeof(CB_Element)], &elem, sizeof(CB_Element));	//Dodaj dane do kolejki
		++cb->Count;
	}
	return true;      //Wszystko ok
}

_Bool cb_Read(CircBuffer *cb, CB_Element *elem)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		if(cb_IsEmpty(cb)) return false;       //Bufor pusty, nie mo¿na zwróciæ elementu
		memcpy(elem, &cb->elements[cb->Beg * sizeof(CB_Element)], sizeof(CB_Element));	//Skopiuj dane z bufora
		cb->Beg = (cb->Beg + 1) % CB_MAXTRANS;
		-- cb->Count;                        //Zmniejszamy liczbê elementów pozosta³ych
	}		                                 //w buforze
	return true;
}