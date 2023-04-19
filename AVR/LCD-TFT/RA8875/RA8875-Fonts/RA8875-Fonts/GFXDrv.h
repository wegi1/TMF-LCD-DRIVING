/*
 * GFXDrv.h
 *
 * Created: 2014-03-07 17:09:55
 *  Author: tmf
 */


#ifndef GFXDRV_H_
#define GFXDRV_H_

#ifdef LCD_P800x480

static inline uint16_t LCD_GetMaxX() {return 800;};         //Zwr�� rozmiar ekranu w poziomie
static inline uint16_t LCD_GetMaxY() {return 480;};         //Zwr�� rozmiar ekranu w pionie

#endif

#ifdef LCD_P640x480

static inline uint16_t LCD_GetMaxX() {return 640;};         //Zwr�� rozmiar ekranu w poziomie
static inline uint16_t LCD_GetMaxY() {return 480;};         //Zwr�� rozmiar ekranu w pionie

#endif

#ifdef LCD_P480x272

static inline uint16_t LCD_GetMaxX() {return 480;};         //Zwr�� rozmiar ekranu w poziomie
static inline uint16_t LCD_GetMaxY() {return 272;};         //Zwr�� rozmiar ekranu w pionie

#endif

#ifdef LCD_P320x240

static inline uint16_t LCD_GetMaxX() {return 320;};         //Zwr�� rozmiar ekranu w poziomie
static inline uint16_t LCD_GetMaxY() {return 240;};         //Zwr�� rozmiar ekranu w pionie

#endif
	
#include "RA8875.h"

typedef union
{
	struct
	{
		uint8_t blue   : 5;
		uint8_t green  : 6;
		uint8_t red    : 5;
	};
	uint16_t word;
} LCD_RGB565;

typedef union
{
	struct
	{
		uint8_t red    : 5;
		uint8_t green  : 6;
		uint8_t blue   : 5;
	};
	uint16_t word;
} LCD_BGR565;

typedef union
{
	struct
	{
		uint8_t blue;
		uint8_t green;
		uint8_t red;
	};
	__uint24 color;
} LCD_RGB;

typedef union
{
	struct
	{
		uint8_t red;
		uint8_t green;
		uint8_t blue;
	};
	__uint24 color;
} LCD_BGR;

typedef union
{
	struct 
	{
		uint8_t bpp       : 5;   //Liczba bit�w na piksel
		_Bool   Color     : 1;   //Antyaliasing subpikselowy
		_Bool   RLE       : 1;   //true je�li w��czona kompresjaRLE
		_Bool   AntiAlias : 1;   //true je�li font z antyaliasingiem
	};
	uint8_t Header;
} LCD_FontDesc;

typedef struct  
{
	LCD_FontDesc  FontDesc;     //Deskryptor typu fontu
	uint8_t       Height;       //Wysoko�� fontu w pikselach
	uint8_t       Chars;        //Liczba znak�w w pliku
} LCD_FontHdr;

void LCD_Init65k();           //Zainicjuj transfer LCD w trybie 65k kolor�w
void LCD_Init256();           //Zainicjuj LCD w trybie 256 kolor�w
void LCD_Rect(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);        //Narysuj prostok�t
void LCD_DrawRect(uint16_t fromx, uint16_t fromy, uint16_t tox, uint16_t toy, _Bool filled);	//Narysuj prostok�t z wyk. uk�. wspomagania kre�lenia figur
void LCD_DrawTriangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, _Bool filled);
void LCD_DrawEllipse(uint16_t x, uint16_t y, uint16_t longaxis, uint16_t shortaxis, _Bool filled);
void LCD_DrawRoundedRect(uint16_t fromx, uint16_t fromy, uint16_t tox, uint16_t toy, uint16_t longaxis, uint16_t shortaxis, _Bool filled);
void LCD_DrawCurve(uint16_t x, uint16_t y, uint16_t longaxis, uint16_t shortaxis, RA_DCRCDraw quadrant, _Bool filled);
void LCD_SetPixel(uint16_t x, uint8_t y, uint16_t color);                                 //Narysuj punkt - pami�taj, aby wcze�niej okre�li� okno, najlepiej obejmuj�ce ca�y ekran
void LCD_LineTo(uint16_t fromx, uint16_t fromy, uint16_t tox, uint16_t toy);	          //Narysuj lini�
void LCD_LineToAA(uint16_t x0, uint8_t y0, uint16_t x1, uint8_t y1, uint16_t color, uint16_t bkgcolor);  //Narysuj lini� wykorzystuj�c antyaliasing
void LCD_Circle(uint16_t x, uint16_t y, uint8_t radius, _Bool filled);					  //Narysuj ko�o (fill==true) lub okr�g (fill==false)
void LCD_CircleAA(uint16_t x0, uint8_t y0, int r, uint16_t color, uint16_t bkgcolor);     //Narysuj ko�o wykorzystuj�c antyaliasing
void LCD_SetTextTransparent(uint16_t x, uint8_t y, const __memx char *tekst, 
                            const uint8_t __flash * const __flash font[], __uint24 color); //Wy�wietl tekst bez t�a
void LCD_SetTextOpaque(uint16_t x, uint8_t y, const __memx char *tekst, const uint8_t __flash * const __flash font[],
__uint24 color, __uint24 bkgcolor);                                //Wy�wietl tekst z t�em
void LCD_SetTextAA(uint16_t x, uint8_t y, const __memx char *tekst, const uint8_t __flash * const __flash font[], uint16_t color, uint16_t bkgcolor);

void LCD_DrawBitmap_565(uint16_t x, uint16_t y, const uint16_t __flash *data);      //Wy�wietl bitmap� na pozycji (x,y) zapisan� w formacie RGB565
void LCD_DrawBitmap_Mono(uint16_t x, uint16_t y, uint16_t color, uint16_t bkgcolor,
                         const uint8_t __flash *data, _Bool transparency);         //Wy�wietl monochromatyczn�bitmap� na pozycji (x,y) z podanymi kolorami i opcjonaln� przezroczysto�ci�, tylko g��bia 16-bitowa

void LCD_DrawBitmap_565_Alpha(uint16_t x, uint16_t y, uint8_t alfaneg, const uint16_t __flash *data);   //Wy�wietl bitmap� na pozycji (x,y) z uwzgl�dnieniem kana�u alfa

static inline void LCD_GetBitmapSize(uint16_t *x, uint16_t *y, const void __flash *data)	   //Zwr�� wysoko�� i szeroko�� wskazanej mapy bitowej w pikselach
{
	*x=*(const uint16_t __flash *)data++; data++;
	*y=*(const uint16_t __flash *)data++; data++;
}

void LCD_DrawBitmap_Scale(uint16_t x, uint8_t y, uint8_t scale, const uint16_t __flash *data);  //Wy�wietl grafik�w formacie 5-6-5 z uwzgl�dnieniem skali zapisanej w rejestrze ILI

#endif /* GFXDRV_H_ */