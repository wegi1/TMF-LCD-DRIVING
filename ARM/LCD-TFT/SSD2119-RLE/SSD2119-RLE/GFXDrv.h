/*
 * GFXDrv.h
 *
 * Created: 2014-03-07 17:09:55
 *  Author: tmf
 */


#ifndef GFXDRV_H_
#define GFXDRV_H_

static inline uint16_t LCD_GetMaxX() {return 320;};         //Zwr�� rozmiar ekranu w poziomie
static inline uint16_t LCD_GetMaxY() {return 240;};         //Zwr�� rozmiar ekranu w pionie

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
	uint32_t color;
} LCD_RGB;

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

void LCD_Init262();           //Zainicjuj transfer LCD w trybie 262k kolor�w
void LCD_Init65();            //Zainicjuj transfer LCD w trybie 65k kolor�w
void LCD_Rect(uint16_t x1, uint8_t y1, uint16_t x2, uint8_t y2, uint32_t color);          //Narysuj prostok�t
void LCD_SetPixel(uint16_t x1, uint8_t y1, uint32_t color);                               //Narysuj punkt
void LCD_LineTo(uint16_t x, uint8_t y, uint16_t x2, uint8_t y2, uint32_t color);          //Narysuj lini�
void LCD_LineToAA(uint16_t x0, uint8_t y0, uint16_t x1, uint8_t y1, uint32_t color, uint32_t bkgcolor);  //Narysuj lini� wykorzystuj�c antyaliasing
void LCD_Circle(uint16_t cx, uint8_t cy , uint8_t radius, uint8_t Fill, uint32_t color);  //Narysuj ko�o (fill==true) lub okr�g (fill==false)
void LCD_CircleAA(uint16_t x0, uint8_t y0, int r, uint32_t color, uint32_t bkgcolor);     //Narysuj ko�o wykorzystuj�c antyaliasing
void LCD_SetTextTransparent(uint16_t x, uint8_t y, const char *tekst, const uint8_t *const font[], uint32_t color); //Wy�wietl tekst bez t�a
void LCD_SetTextOpaque(uint16_t x, uint8_t y, const char *tekst, const uint8_t *const font[], uint32_t color, uint32_t bkgcolor);  //Wy�wietl tekst z t�em
void LCD_SetTextAA(uint16_t x, uint8_t y, const char *tekst, const uint8_t * const font[], uint32_t color, uint32_t bkgcolor);

void LCD_DrawBitmap_565(uint16_t x, uint8_t y, const uint16_t *data);      //Wy�wietl bitmap� na pozycji (x,y) zapisan� w formacie RGB565
void LCD_DrawBitmap_Mono(uint16_t x, uint8_t y, uint16_t color, uint16_t bkgcolor,
                         const uint8_t *data);                             //Wy�wietl monochromatyczn�bitmap� na pozycji (x,y) z podanymi kolorami
//Wy�wietla bitmapy skopmpresowane algorytmem RLE
void LCD_DrawBitmapRLE_565(uint16_t x, uint8_t y, const uint16_t *data);     //Wy�wietl bitmap� na pozycji (x,y) zapisan� w formacie RGB565
void LCD_DrawBitmapRLE_Mono(uint16_t x, uint8_t y, uint16_t color, uint16_t bkgcolor,
const uint8_t *data);                             //Wy�wietl monochromatyczn�bitmap� na pozycji (x,y) z podanymi kolorami

static inline void LCD_GetBitmapSize(uint16_t *x, uint16_t *y, const void *data)	   //Zwr�� wysoko�� i szeroko�� wskazanej mapy bitowej w pikselach
{
	*x=*(const uint16_t *)data++; data++;
	*y=*(const uint16_t *)data++; data++;
}
#endif /* GFXDRV_H_ */