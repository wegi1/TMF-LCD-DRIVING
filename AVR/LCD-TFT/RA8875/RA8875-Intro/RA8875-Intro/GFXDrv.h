/*
 * GFXDrv.h
 *
 * Created: 2014-03-07 17:09:55
 *  Author: tmf
 */


#ifndef GFXDRV_H_
#define GFXDRV_H_

static inline uint16_t LCD_GetMaxX() {return 800;};         //Zwróæ rozmiar ekranu w poziomie
static inline uint16_t LCD_GetMaxY() {return 480;};         //Zwróæ rozmiar ekranu w pionie

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
		uint8_t bpp       : 5;   //Liczba bitów na piksel
		_Bool   Color     : 1;   //Antyaliasing subpikselowy
		_Bool   RLE       : 1;   //true jeœli w³¹czona kompresjaRLE
		_Bool   AntiAlias : 1;   //true jeœli font z antyaliasingiem
	};
	uint8_t Header;
} LCD_FontDesc;

typedef struct  
{
	LCD_FontDesc  FontDesc;     //Deskryptor typu fontu
	uint8_t       Height;       //Wysokoœæ fontu w pikselach
	uint8_t       Chars;        //Liczba znaków w pliku
} LCD_FontHdr;

void LCD_Init65k();           //Zainicjuj transfer LCD w trybie 65k kolorów
void LCD_Rect(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);        //Narysuj prostok¹t
void LCD_SetPixel(uint16_t x1, uint8_t y1, __uint24 color);                               //Narysuj punkt - pamiêtaj, aby wczeœniej okreœliæ okno, najlepiej obejmuj¹ce ca³y ekran
void LCD_LineTo(uint16_t x, uint8_t y, uint16_t x2, uint8_t y2, __uint24 color);          //Narysuj liniê
void LCD_LineToAA(uint16_t x0, uint8_t y0, uint16_t x1, uint8_t y1, __uint24 color, __uint24 bkgcolor);  //Narysuj liniê wykorzystuj¹c antyaliasing
void LCD_Circle(uint16_t cx, uint8_t cy , uint8_t radius, uint8_t Fill, __uint24 color);  //Narysuj ko³o (fill==true) lub okr¹g (fill==false)
void LCD_CircleAA(uint16_t x0, uint8_t y0, int r, __uint24 color, __uint24 bkgcolor);     //Narysuj ko³o wykorzystuj¹c antyaliasing
void LCD_SetTextTransparent(uint16_t x, uint8_t y, const __memx char *tekst, 
                            const uint8_t __flash * const __flash font[], __uint24 color); //Wyœwietl tekst bez t³a
void LCD_SetTextOpaque(uint16_t x, uint8_t y, const __memx char *tekst, const uint8_t __flash * const __flash font[],
__uint24 color, __uint24 bkgcolor);                                //Wyœwietl tekst z t³em
void LCD_SetTextAA(uint16_t x, uint8_t y, const __memx char *tekst, const uint8_t __flash * const __flash font[], __uint24 color, __uint24 bkgcolor);

void LCD_DrawBitmap_565(uint16_t x, uint8_t y, const uint16_t __flash *data);      //Wyœwietl bitmapê na pozycji (x,y) zapisan¹ w formacie RGB565
void LCD_DrawBitmap_Mono(uint16_t x, uint8_t y, uint16_t color, uint16_t bkgcolor,
                         const uint8_t __flash *data);                             //Wyœwietl monochromatyczn¹bitmapê na pozycji (x,y) z podanymi kolorami

void LCD_DrawBitmap_565_Alpha(uint16_t x, uint8_t y, uint8_t alfaneg, const uint16_t __flash *data);   //Wyœwietl bitmapê na pozycji (x,y) z uwzglêdnieniem kana³u alfa

static inline void LCD_GetBitmapSize(uint16_t *x, uint16_t *y, const void __flash *data)	   //Zwróæ wysokoœæ i szerokoœæ wskazanej mapy bitowej w pikselach
{
	*x=*(const uint16_t __flash *)data++; data++;
	*y=*(const uint16_t __flash *)data++; data++;
}

void LCD_DrawBitmap_Scale(uint16_t x, uint8_t y, uint8_t scale, const uint16_t __flash *data);  //Wyœwietl grafikêw formacie 5-6-5 z uwzglêdnieniem skali zapisanej w rejestrze ILI

#endif /* GFXDRV_H_ */