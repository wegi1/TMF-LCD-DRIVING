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

void LCD_Init262();           //Zainicjuj transfer LCD w trybie 262k kolor�w
void LCD_Init65();            //Zainicjuj transfer LCD w trybie 65k kolor�w
void LCD_Rect(uint16_t x1, uint8_t y1, uint16_t x2, uint8_t y2, uint32_t color);          //Narysuj prostok�t
void LCD_SetPixel(uint16_t x1, uint8_t y1, uint32_t color);                               //Narysuj punkt
void LCD_LineTo(uint16_t x, uint8_t y, uint16_t x2, uint8_t y2, uint32_t color);          //Narysuj lini�
void LCD_Circle(uint16_t cx, uint8_t cy , uint8_t radius, uint8_t Fill, uint32_t color);  //Narysuj ko�o (fill==true) lub okr�g (fill==false)
void LCD_SetText(uint16_t x, uint8_t y,
                 const char *tekst, const uint8_t * const font[],
				 uint32_t color, uint32_t bkgcolor, _Bool transparent);                   //Wy�wietl tekst, je�li transparent==true, to nie jest wy�wietlane t�o

void LCD_DrawBitmap_565(uint16_t x, uint8_t y, const uint16_t *data);      //Wy�wietl bitmap� na pozycji (x,y) zapisan� w formacie RGB565
void LCD_DrawBitmap_Mono(uint16_t x, uint8_t y, uint16_t color, uint16_t bkgcolor,
                         const uint8_t *data);                             //Wy�wietl monochromatyczn�bitmap� na pozycji (x,y) z podanymi kolorami

static inline void LCD_GetBitmapSize(uint16_t *x, uint16_t *y, const void *data)	   //Zwr�� wysoko�� i szeroko�� wskazanej mapy bitowej w pikselach
{
	*x=*(const uint16_t *)data++; data++;
	*y=*(const uint16_t *)data++; data++;
}
#endif /* GFXDRV_H_ */