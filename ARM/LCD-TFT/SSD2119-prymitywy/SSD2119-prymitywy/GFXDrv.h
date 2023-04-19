/*
 * GFXDrv.h
 *
 * Created: 2014-03-07 17:09:55
 *  Author: tmf
 */


#ifndef GFXDRV_H_
#define GFXDRV_H_

static inline uint16_t LCD_GetMaxX() {return 320;};         //Zwróæ rozmiar ekranu w poziomie
static inline uint16_t LCD_GetMaxY() {return 240;};         //Zwróæ rozmiar ekranu w pionie

void LCD_Init262();           //Zainicjuj transfer LCD w trybie 262k kolorów
void LCD_Init65();            //Zainicjuj transfer LCD w trybie 65k kolorów
void LCD_Rect(uint16_t x1, uint8_t y1, uint16_t x2, uint8_t y2, uint32_t color);          //Narysuj prostok¹t
void LCD_SetPixel(uint16_t x1, uint8_t y1, uint32_t color);                               //Narysuj punkt
void LCD_LineTo(uint16_t x, uint8_t y, uint16_t x2, uint8_t y2, uint32_t color);          //Narysuj liniê
void LCD_Circle(uint16_t cx, uint8_t cy , uint8_t radius, uint8_t Fill, uint32_t color);  //Narysuj ko³o (fill==true) lub okr¹g (fill==false)
void LCD_SetText(uint16_t x, uint8_t y,
                 const char *tekst, const uint8_t * const font[],
				 uint32_t color, uint32_t bkgcolor, _Bool transparent);                   //Wyœwietl tekst, jeœli transparent==true, to nie jest wyœwietlane t³o

void LCD_DrawBitmap_565(uint16_t x, uint8_t y, const uint16_t *data);      //Wyœwietl bitmapê na pozycji (x,y) zapisan¹ w formacie RGB565
void LCD_DrawBitmap_Mono(uint16_t x, uint8_t y, uint16_t color, uint16_t bkgcolor,
                         const uint8_t *data);                             //Wyœwietl monochromatyczn¹bitmapê na pozycji (x,y) z podanymi kolorami

static inline void LCD_GetBitmapSize(uint16_t *x, uint16_t *y, const void *data)	   //Zwróæ wysokoœæ i szerokoœæ wskazanej mapy bitowej w pikselach
{
	*x=*(const uint16_t *)data++; data++;
	*y=*(const uint16_t *)data++; data++;
}
#endif /* GFXDRV_H_ */