/*
 * SetClk.h
 *
 * Created: 2017-01-03 10:48:49
 *  Author: tmf
 */ 


#ifndef SETCLK_H_
#define SETCLK_H_

#include <sam.h>

void Set48MHzClk();		//Ustaw zegar na 48MHz

#  define cpu_irq_enable()                     \
do {                                       \
	__DMB();                               \
	__enable_irq();                        \
} while (0)
#  define cpu_irq_disable()                    \
do {                                       \
	__disable_irq();                       \
	__DMB();                               \
} while (0)

#endif /* SETCLK_H_ */