/*
 * Fonts.h
 *
 * Created: 2014-03-01 09:58:58
 *  Author: tmf
 */ 


#ifndef FONTS_H_
#define FONTS_H_

#include <stdint.h>

//Fonty bez antyaliasingu
extern const __flash uint8_t* const __flash system16_array[];
extern const __flash uint8_t* const __flash system12_array[];
extern const __flash uint8_t* const __flash system8_array[];
extern const __flash uint8_t* const __flash Times16WA_array[];

//Fonty z antyaliasingiem
extern const __flash uint8_t* const __flash Times16AA332_array[];  //Times New Roman z antyaliasingiem 8 bitowym subpikselowym 3-3-2
extern const __flash uint8_t* const __flash Times16AA8bitmono[];   //Times New Roman z antyaliasingiem 8 bitowym monochromatycznym
extern const __flash uint8_t* const __flash Times16AA2bit[]; //Times New Roman z AA monochromatycznym 2 bitowym

#endif /* FONTS_H_ */