/*
 * BMPLib.h
 *
 * Created: 2014-03-28 21:03:54
 *  Author: tmf
 */ 


#ifndef BMPLIB_H_
#define BMPLIB_H_

//Wyœwietla bitmapê na pozycji Xpos, Ypos. Bitmapa musi byæ w formacie 24-bitowego koloru, 5-6-5, lub skompresowana RLE4 lub RLE8
void LCD_BMP(const uint8_t *Image, uint16_t Xpos, uint8_t Ypos);


#endif /* BMPLIB_H_ */