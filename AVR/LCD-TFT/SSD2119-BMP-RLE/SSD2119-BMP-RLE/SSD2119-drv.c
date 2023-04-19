/*
 * SSD2119_drv.c
 *
 * Created: 2014-03-07 17:08:33
 *  Author: tmf
 */

#include <stdbool.h>
#include "ssd2119.h"
#include "xmega-spi.h"
#include "GFXDrv.h"
#include <util/delay.h>
#include <stdlib.h>

typedef struct
{
	uint8_t reg;        //Numer rejestru uk��du SSD
	uint16_t value;     //Warto�� rejestru
} SSD_Regs;

const SSD_Regs __flash SSD_Init262[] = {{0x28, 0x0006}, {0x00, 0x0001}, {0x10, 0x0000}, {0x01, 0x72EF}, {0x02, 0x0600}, {0x03, 0x6A38}, {0x11, 0x4870},
{0x0F, 0x0000}, {0x0B, 0x5308}, {0x0C, 0x0003}, {0x0D, 0x000A}, {0x0E, 0x2E00}, {0x1E, 0x00BE}, {0x25, 0x8000},
{0x26, 0x7800}, {0x4E, 0x0000}, {0x4F, 0x0000}, {0x12, 0x08D9}, {0x30, 0x0000}, {0x31, 0x0104}, {0x32, 0x0100},
{0x33, 0x0305}, {0x34, 0x0505}, {0x35, 0x0305}, {0x36, 0x0707}, {0x37, 0x0300}, {0x3A, 0x1200}, {0x3B, 0x0800},
{0x07, 0x0033}};

const SSD_Regs __flash SSD_Init65[] = {{0x28, 0x0006}, {0x00, 0x0001}, {0x10, 0x0000}, {0x01, 0x72EF}, {0x02, 0x0600}, {0x03, 0x6A38}, {0x11, 0x6870},
{0x0F, 0x0000}, {0x0B, 0x5308}, {0x0C, 0x0003}, {0x0D, 0x000A}, {0x0E, 0x2E00}, {0x1E, 0x00BE}, {0x25, 0x8000},
{0x26, 0x7800}, {0x4E, 0x0000}, {0x4F, 0x0000}, {0x12, 0x08D9}, {0x30, 0x0000}, {0x31, 0x0104}, {0x32, 0x0100},
{0x33, 0x0305}, {0x34, 0x0505}, {0x35, 0x0305}, {0x36, 0x0707}, {0x37, 0x0300}, {0x3A, 0x1200}, {0x3B, 0x0800},
{0x07, 0x0033}};

static struct
{
	_Bool FullScreen       : 1;           //true je�li zosta� wybrany do zapisu ca�y ekran
	_Bool Mode565          : 1;           //true je�li mamy tryb przesy�u 2 bajty na piksel, false je�li 3 bajty na piksel (tryb 18-bitowy)

} LCD_Driver_Status;            //Stan sterownika LCD

uint8_t DMABuffer[10][2];          //Bufor na transakcj� DMA

void ssd2119_SendCmdWithData(uint8_t cmd, uint16_t data)
{
	ssd2119_RS(0);
	ssd2119_CS(0);
	SPI_RW_Byte(cmd);
	ssd2119_RS(1);     //Deaktywuj tryb wysy�ania polece�
	ssd2119_SendDataByte(data >> 8);
	SPI_RW_Byte(data);
	ssd2119_CS(1);     //Deaktywuj kontroler
}

void LCD_Init262()
{
	ssd2119_RESET();   //Zresetuj kontroler

	for(uint8_t indeks=0; indeks < sizeof(SSD_Init262)/sizeof(SSD_Init262[0]); indeks++)
	{
		ssd2119_SendCmdWithData(SSD_Init262[indeks].reg, SSD_Init262[indeks].value);
	}

	LCD_Driver_Status.Mode565=false;    //Tryb przesy�u 3 bajty/piksel
}

void LCD_Init65()
{
	ssd2119_RESET();   //Zresetuj kontroler

	for(uint8_t indeks=0; indeks < sizeof(SSD_Init65)/sizeof(SSD_Init65[0]); indeks++)
	{
		ssd2119_SendCmdWithData(SSD_Init65[indeks].reg, SSD_Init65[indeks].value);
	}

	LCD_Driver_Status.Mode565=true;    //Tryb przesy�u 3 bajty/piksel
}

void SED2119_RGB565()   //Wybierz przesy� 16-bitowych danych (2 bajty/piksel)
{
	ssd2119_SendCmdWithData(0x11, (ssd2119_EntryMode_Reg){.DFM=0b11, .DenMode=1, .TY=0b01, .ID=0b11, .AM=0}.word);  //Wybierz rejestr Entry Mode
}

void SED2119_RGB666()  //Wybierz przesy� 18-bitowych danych (3 bajty/piksel)
{
	ssd2119_SendCmdWithData(0x11, (ssd2119_EntryMode_Reg){.DFM=0b10, .DenMode=1, .TY=0b01, .ID=0b11, .AM=0}.word);  //Wybierz rejestr Entry Mode
}

//Ustaw pozycj� (x,y) w GRAM
void LCD_SetPosition(uint16_t x1, uint8_t y1)
{
	ssd2119_SendCmdWithData(ssd2119_Set_GDDRAM_X_address_counter, x1);    // Pozycja X piksela
	ssd2119_SendCmdWithData(ssd2119_Set_GDDRAM_Y_address_counter, y1);    // Pozycja Y piksela
}

//Ustaw okno w pami�ci GRAM
void LCD_SetWindow(uint16_t x1, uint8_t y1, uint16_t x2, uint8_t y2)
{
	ssd2119_SendCmdWithData(ssd2119_Vertical_RAM_address_position, ((uint16_t)y1) | (((uint16_t)y2) << 8)); //G�rny i dolny brzeg obszaru
	ssd2119_SendCmdWithData(ssd2119_Horizontal_RAM_address_start_position, x1);    //Lewy brzeg obszaru
	ssd2119_SendCmdWithData(ssd2119_Horizontal_RAM_address_end_position, x2);    //Prawy brzeg obszaru
}

void LCD_Rect(uint16_t x1, uint8_t y1, uint16_t x2, uint8_t y2, __uint24 color)
{
	LCD_SetWindow(x1, y1, x2, y2);

	ssd2119_SendCmdWithData(ssd2119_Set_GDDRAM_X_address_counter, x1);    // Pozycja X piksela
	ssd2119_SendCmdWithData(ssd2119_Set_GDDRAM_Y_address_counter, y1);    // Pozycja Y piksela

	ssd2119_SendCmd(0x22);    // Zapis pod wskazan� pozycj�

	ssd2119_CS(0);            //Aktywuj kontroler - DMA prze�le dane do GRAM i automatycznie zdezaktywuje sygna� CS
	__uint24 val=(1+y2-y1)*(__uint24)(1+x2-x1);  //Policz z ilu pikseli sk�ada si� prostok�t
	for(;val;val--)
	{
		ssd2119_SendDataByte(color >> 16);
		ssd2119_SendDataByte(color >> 8);
		ssd2119_SendDataByte(color & 0xff);
	}
	_delay_loop_1(5);    //Zaczekaj na koniec wysy�ania danych
	ssd2119_CS(1);
}

void LCD_SetPixel(uint16_t x1, uint8_t y1, __uint24 color)
{
	ssd2119_SendCmdWithData(ssd2119_Set_GDDRAM_X_address_counter, x1);    // Pozycja X piksela
	ssd2119_SendCmdWithData(ssd2119_Set_GDDRAM_Y_address_counter, y1);    // Pozycja Y piksela

	ssd2119_SendCmd(ssd2119_Write_to_GRAM);    // Zapis pod wskazan� pozycj�
	ssd2119_SendDataByte(color >> 16);
	ssd2119_SendDataByte(color >> 8);
	SPI_RW_Byte(color);                        //Czekamy na wys�anie wszystkich danych
	ssd2119_CS(1);                             //i dezaktywujemy CS
}

void LCD_LineTo(uint16_t x, uint8_t y, uint16_t x2, uint8_t y2, __uint24 color)
{
	void LCD_SetPixel(uint16_t x1, uint8_t y1, __uint24 color)
	{
		ssd2119_SendCmdWithData(ssd2119_Set_GDDRAM_X_address_counter, x1);    // Pozycja X piksela
		ssd2119_SendCmdWithData(ssd2119_Set_GDDRAM_Y_address_counter, y1);    // Pozycja Y piksela

		ssd2119_SendCmd(ssd2119_Write_to_GRAM);    // Zapis pod wskazan� pozycj�
		ssd2119_SendDataByte(color >> 16);
		ssd2119_SendDataByte(color >> 8);
		ssd2119_SendDataByte(color);
	}
	
	int16_t dy;
	int16_t dx;
	int8_t stepx, stepy;

	if(y2>y) { dy = y2-y; stepy = 1;} else { dy = y-y2; stepy = -1;}
	if(x2>x) { dx = x2-x; stepx = 1;} else { dx = x-x2; stepx = -1;}
	dy <<= 1;
	dx <<= 1;
	LCD_SetPixel(x,y, color);
	if (dx > dy)
	{
		int fraction = dy - (dx >> 1);
		while (x != x2)
		{
			if (fraction >= 0)
			{
				y += stepy;
				fraction -= dx;
			}
			x += stepx;
			fraction += dy;
			LCD_SetPixel(x,y, color);
		}
	} else
	{
		int fraction = dx - (dy >> 1);
		while (y != y2)
		{
			if (fraction >= 0)
			{
				x += stepx;
				fraction -= dy;
			}
			y += stepy;
			fraction += dx;
			LCD_SetPixel(x,y, color);
		}
	}
	
	ssd2119_CS(1); //i dezaktywujemy CS - od ostatniego kre�lenia piksela up�yn�o wystarczaj�co du�o czasu, aby opr�ni� bufor
}

void LCD_Circle(uint16_t cx, uint8_t cy , uint8_t radius, uint8_t Fill, __uint24 color)
{
	int16_t x, y, xchange, ychange, radiusError;
	x=radius;
	y=0;
	xchange=1-2*radius;
	ychange=1;
	radiusError=0;
	while(x>=y)
	{
		if(Fill==0)
		{
			LCD_SetPixel(cx+x, cy+y, color);
			LCD_SetPixel(cx-x, cy+y, color);
			LCD_SetPixel(cx-x, cy-y, color);
			LCD_SetPixel(cx+x, cy-y, color);
			LCD_SetPixel(cx+y, cy+x, color);
			LCD_SetPixel(cx-y, cy+x, color);
			LCD_SetPixel(cx-y, cy-x, color);
			LCD_SetPixel(cx+y, cy-x, color);
		} else
		{
			LCD_Rect(cx-x, cy+y, cx+x, cy+y, color);
			LCD_Rect(cx-x, cy-y, cx+x, cy-y, color);
			LCD_Rect(cx-y, cy+x, cx+y, cy+x, color);
			LCD_Rect(cx-y, cy-x, cx+y, cy-x, color);
		}

		y++;
		radiusError+=ychange;
		ychange+=2;
		if(2*radiusError+xchange>0)
		{
			x--;
			radiusError+=xchange;
			xchange+=2;
		}
	}
}

static void setPixelAA(uint16_t x1, uint8_t y1, uint8_t alfaneg, __uint24 color, __uint24 bkgcolor)
{
	ssd2119_SendCmdWithData(ssd2119_Set_GDDRAM_X_address_counter, x1);    // Pozycja X piksela
	ssd2119_SendCmdWithData(ssd2119_Set_GDDRAM_Y_address_counter, y1);    // Pozycja Y piksela

	ssd2119_SendCmd(ssd2119_Write_to_GRAM);    // Zapis pod wskazan� pozycj�

	uint8_t alfa=~alfaneg;
	ssd2119_SendDataByte((alfa*((LCD_RGB)color).red + alfaneg*(((LCD_RGB)bkgcolor).red)) >> 8);     //Blending kolor�w
	ssd2119_SendDataByte((alfa*((LCD_RGB)color).green + alfaneg*(((LCD_RGB)bkgcolor).green)) >> 8);
	ssd2119_SendDataByte((alfa*((LCD_RGB)color).blue + alfaneg*(((LCD_RGB)bkgcolor).blue)) >> 8);
}

void LCD_LineToAA(uint16_t x0, uint8_t y0, uint16_t x1, uint8_t y1, __uint24 color, __uint24 bkgcolor)
{
   int sx=x0 < x1 ? 1 : -1, sy = y0 < y1 ? 1 : -1, x2;
   long dx=abs(x1-x0), dy = abs(y1-y0);
   long err = dx*dx+dy*dy;
   long e2=err == 0 ? 1 : 0xffff7fl/sqrt(err);
	
   dx*=e2; dy*=e2; err=dx-dy;
   for(;;)                                         //Rysujemy piksele
   {
      setPixelAA(x0,y0,labs(err-dx+dy)>>16, color, bkgcolor);
      e2 = err; x2 = x0;
      if(2*e2 >= -dx)
	  {                                            // Krok na osi x
         if (x0 == x1) break;
         if (e2+dy < 0xff0000l) setPixelAA(x0,y0+sy,(e2+dy)>>16, color, bkgcolor);
         err -= dy; x0 += sx; 
      } 
      if(2*e2 <= dy)
	  {                                             // Krok na osi y
         if(y0 == y1) break;
         if(dx-e2 < 0xff0000l) setPixelAA(x2+sx,y0,(dx-e2)>>16, color, bkgcolor);
         err += dx; y0 += sy; 
	  }
    }
}

void LCD_CircleAA(uint16_t x0, uint8_t y0, int r, __uint24 color, __uint24 bkgcolor)
{
   int x=-r, y=0;
   int x2; 
   long e2, intensity, err=2-2*r;
   r = 1-err;
   do {
      intensity=255*abs(err-2*(x+y)-2)/r;                   //Oblicz b��d - intensywno�� piksela
      setPixelAA(x0-x, y0+y, intensity, color, bkgcolor);   //Rysujemy odbicie symetryczne w czterech kwadrantach
      setPixelAA(x0-y, y0-x, intensity, color, bkgcolor); 
      setPixelAA(x0+x, y0-y, intensity, color, bkgcolor); 
      setPixelAA(x0+y, y0+x, intensity, color, bkgcolor); 
      e2=err; x2=x;
      if(err+y > 0)
	  {                                              // Krok na osi x
         intensity=255*(err-2*x-1)/r;                //Piksel zewn�trzny
         if(intensity < 256)
		 {
            setPixelAA(x0-x, y0+y+1, intensity, color, bkgcolor);
            setPixelAA(x0-y-1, y0-x, intensity, color, bkgcolor);
            setPixelAA(x0+x, y0-y-1, intensity, color, bkgcolor);
            setPixelAA(x0+y+1, y0+x, intensity, color, bkgcolor);
         }
         err+=++x*2+1;
      }
      if(e2+x2 <= 0)
	  {                                             // Krok na osi y
         intensity=255*(2*y+3-e2)/r;                // Piksel wewn�trzny
         if(intensity < 256)
		 {
            setPixelAA(x0-x2-1, y0+y, intensity, color, bkgcolor);
            setPixelAA(x0-y, y0-x2-1, intensity, color, bkgcolor);
            setPixelAA(x0+x2+1, y0-y, intensity, color, bkgcolor);
            setPixelAA(x0+y, y0+x2+1, intensity, color, bkgcolor);
         }
         err+=++y*2+1;
      }
   } while(x < 0);
}

void LCD_SetTextTransparent(uint16_t x, uint8_t y, const __memx char *tekst, const uint8_t __flash * const __flash font[], __uint24 color)
{
	uint8_t rows=(uint8_t)(uint16_t)font[0]; //Pobierz wysoko�� fontu
	char ch;
	y+=rows-1;

	while((ch=*tekst++))  //Wy�wietl kolejne znaki a� do ko�ca tekstu (znaku NUL)
	{
		const uint8_t __flash *znak=font[ch-30]; //Adres pocz�tku opisu znaku
		uint8_t col=*znak++;                     //Szeroko�� znaku w pikselach
		uint8_t coldesc=coldesc;                 //Bajt opisu znaku
		uint8_t pixelcnt=0;                      //Licznik pikseli w znaku

		for(uint8_t ox=0; ox < col; ox++)        //Wy�wietlamy kolejne kolumny tworz�ce znak
		{
			for(uint8_t oy=0; oy < rows; oy++)   //Narysuj jedn� kolumn� znaku
			{
				if((pixelcnt & 0b111) == 0) coldesc=*znak++;     //Pobierz bajt opisu znaku
				pixelcnt++;
				if(coldesc & 0x80) LCD_SetPixel(x, y-oy, color);  //Wy�wietlamy tylko piksele znaku, pomijamy t�o - ma by� przezroczyste
				coldesc<<=1;
			}
			x++;  //Rysujemy kolejn� kolumn�
			if(x >= LCD_GetMaxX()) return;   //Jeste�my poza obszarem kre�lenia
		}
	}
}

typedef union
{
	struct
	{
		uint8_t blue   : 2;
		uint8_t green  : 3;
		uint8_t red    : 3; 
	};
	uint8_t color;
} LCD_RGB332;

void LCD_SetTextAA(uint16_t x, uint8_t y, const __memx char *tekst, const uint8_t __flash * const __flash font[], __uint24 color, __uint24 bkgcolor)
{
//
// Pusta funkcja na wypadek napotkania nieznanego deskryptora
//
	void CharRenderFuncNone(uint16_t pixelsno, uint8_t rows, const uint8_t __flash *znak, __uint24 color, __uint24 bkgcolor)
	{		
	}
	
//
// Pusta funkcja na wypadek napotkania nieznanego deskryptora
//
	void CharRenderFunc1bitNoAA(uint16_t pixelsno, uint8_t rows, const uint8_t __flash *znak, __uint24 color, __uint24 bkgcolor)
	{
		uint8_t coldesc=coldesc; //Bajt opisu znaku
		uint8_t pixelcnt=0;      //Licznik pikseli w znaku
		__uint24 pixcolor;       //Kolor rysowanego piksela
		
		for(uint16_t ox=0; ox < pixelsno; ox++)        //Wy�wietlamy kolejne kolumny tworz�ce znak
		{
			if((pixelcnt & 0b111) == 0) coldesc=*znak++;     //Pobierz bajt opisu znaku
			pixelcnt++;
			if(coldesc & 0x80) pixcolor=color;
			else pixcolor=bkgcolor;                //Narysuj t�o tylko je�li nie wy�wietlamy prze�roczystego tekstu
			ssd2119_SendDataByte(pixcolor >> 16);  //Zapisz do GRAM kolor piksela
			ssd2119_SendDataByte(pixcolor >> 8);
			ssd2119_SendDataByte(pixcolor);
			coldesc<<=1;
		}	
	}

//
// Funkcja renderuj�ca znak z wykorzystaniem 2 bitowego antyaliasingu
//	
	void CharRenderFunc2bitMono(uint16_t pixelsno, uint8_t rows, const uint8_t __flash *znak, __uint24 color, __uint24 bkgcolor)
	{
		uint8_t alfa, alfaneg;
		uint8_t pixcnt=0;
		uint8_t alfavalue=alfavalue;

		for(uint16_t ox=0; ox < pixelsno; ox++)        //Wy�wietlamy kolejne piksele tworz�ce znak
		{
			if((pixcnt & 0b11) == 0) alfavalue=*znak++;
			alfa=alfavalue & 0b11000000;
			if(alfa) alfa|=0b00111111;
			alfaneg=~alfa;
			ssd2119_SendDataByte((alfa*((LCD_RGB)color).red + alfaneg*(((LCD_RGB)bkgcolor).red)) >> 8);     //Blending kolor�w
			pixcnt++;
			alfavalue<<=2;
			ssd2119_SendDataByte((alfa*((LCD_RGB)color).green + alfaneg*(((LCD_RGB)bkgcolor).green)) >> 8);
			if(pixcnt >= rows) pixcnt=0;
			ssd2119_SendDataByte((alfa*((LCD_RGB)color).blue + alfaneg*(((LCD_RGB)bkgcolor).blue)) >> 8);
		}		
	}

//
// Funkcja renderuj�ca znak z wykorzystaniem 8 bitowego antyaliasingu mono
//
	void CharRenderFunc8bitMono(uint16_t pixelsno, uint8_t rows, const uint8_t __flash *znak, __uint24 color, __uint24 bkgcolor)
	{
		for(uint16_t ox=0; ox < pixelsno; ox++)        //Wy�wietlamy kolejne piksele tworz�ce znak
		{
			uint8_t alfa=*znak++;
			uint8_t alfaneg=~alfa;
			ssd2119_SendDataByte((alfa*((LCD_RGB)color).red + alfaneg*(((LCD_RGB)bkgcolor).red)) >> 8);     //Blending kolor�w
			ssd2119_SendDataByte((alfa*((LCD_RGB)color).green + alfaneg*(((LCD_RGB)bkgcolor).green)) >> 8);
			ssd2119_SendDataByte((alfa*((LCD_RGB)color).blue + alfaneg*(((LCD_RGB)bkgcolor).blue)) >> 8);
		}
	}

//
// Funkcja renderuj�ca znak z wykorzystaniem 8 bitowego antyaliasingu subpikselowego (3-3-2)
//
	void CharRenderFunc322Subpixel(uint16_t pixelsno, uint8_t rows, const uint8_t __flash *znak, __uint24 color, __uint24 bkgcolor)
	{
		for(uint16_t ox=0; ox < pixelsno; ox++)   //Wy�wietlamy kolejne piksele tworz�ce znak
		{
			LCD_RGB332 tmpalfa={.color=*znak++};
			uint8_t alfa, alfaneg;
			alfa=tmpalfa.red << 5; if(alfa) alfa|=0b00011111; alfaneg=~alfa; 
			ssd2119_SendDataByte((alfa*((LCD_RGB)color).red + alfaneg*(((LCD_RGB)bkgcolor).red)) >> 8);     //Blending kolor�w
			alfa=tmpalfa.green << 5; if(alfa) alfa|=0b00011111; alfaneg=~alfa; 
			ssd2119_SendDataByte((alfa*((LCD_RGB)color).green + alfaneg*(((LCD_RGB)bkgcolor).green)) >> 8);
			alfa=tmpalfa.blue << 6; if(alfa) alfa|=0b00111111; alfaneg=~alfa; 
			ssd2119_SendDataByte((alfa*((LCD_RGB)color).blue + alfaneg*(((LCD_RGB)bkgcolor).blue)) >> 8);
		}
	}
	
	uint8_t rows=(uint8_t)(uint16_t)font[1]; //Pobierz wysoko�� fontu, dziwna konwersja wynika z zapisu w tablicy sta�ej jako wska�nika

	void (*RenderFuncPtr)(uint16_t pikselsno, uint8_t rows, const uint8_t __flash *znak, __uint24 color, __uint24 bkgcolor); //Wska�nik na funkcj� renderuj�c�

	switch((uint8_t)(uint16_t)font[0])
	{
		case 0x01:       RenderFuncPtr=&CharRenderFunc1bitNoAA;  //Renderujemy czcionk� bez antyaliasingiem 1bpp
						 ssd2119_SendCmdWithData(ssd2119_Entry_Mode, (ssd2119_EntryMode_Reg){.DFM=0b10, .DenMode=1, .TY=0b01, .ID=0b01, .AM=1}.word);
						 break;
		case 0x82:       RenderFuncPtr=&CharRenderFunc2bitMono;  //Renderujemy czcionk� z antyaliasingiem 2bpp mono
						 ssd2119_SendCmdWithData(ssd2119_Entry_Mode, (ssd2119_EntryMode_Reg){.DFM=0b10, .DenMode=1, .TY=0b01, .ID=0b01, .AM=1}.word);
		                 break;      
		case 0x88:       RenderFuncPtr=&CharRenderFunc8bitMono; break;      //Renderujemy czcionk� z antyaliasingiem 8bpp mono
		case 0xA8:       RenderFuncPtr=&CharRenderFunc322Subpixel; break;   //Renderujemy czcionk� z antyaliasingiem 8bpp kolor 3-3-2
		default:         RenderFuncPtr=CharRenderFuncNone;
	};

	char ch;
	while((ch=*tekst++))  //Wy�wietl kolejne znaki a� do ko�ca tekstu (znaku NUL)
	{
		const uint8_t __flash *znak=font[ch-29]; //Adres pocz�tku opisu znaku
		uint8_t col=*znak++;                     //Szeroko�� znaku w pikselach

		LCD_SetWindow(x, y, x + col -1 , y + rows -1); //Okno w kt�rym zapiszemy znak
		LCD_SetPosition(x, y + rows - 1);              //Pocz�tek okna
		ssd2119_SendCmd(ssd2119_Write_to_GRAM);        // Zapis pod wskazan� pozycj�
		
		RenderFuncPtr(col*rows, rows, znak, color, bkgcolor);  //Wy�wietl znak stosuj�c wybran� technik� renderowania
		
		x+=col;
		if(x >= LCD_GetMaxX()) break;   //Jeste�my poza obszarem kre�lenia
	}

	_delay_loop_1(2);   //Zaczekaj na opr�nienie bufora nadajnika USART (2*3 takty + co�tam z poprzedzaj�cego kodu)
	ssd2119_CS(1);
	ssd2119_SendCmdWithData(ssd2119_Entry_Mode, (ssd2119_EntryMode_Reg){.DFM=0b10, .DenMode=1, .TY=0b01, .ID=0b11, .AM=0}.word);
}

void LCD_DrawBitmap_Mono(uint16_t x, uint8_t y, uint16_t color, uint16_t bkgcolor,const uint8_t __flash *data)
{
	if(LCD_Driver_Status.Mode565 == false) SED2119_RGB565();     //Prze��cz si� na tryb przesy�ania 2 bajty na piksel

	uint16_t width=*(const uint16_t __flash *)data++; data++;
	uint16_t height=*(const uint16_t __flash *)data++; data++;

	LCD_SetWindow(x, y, x + width -1, y +  height - 1);
	LCD_SetPosition(x, y);
	ssd2119_SendCmd(ssd2119_Write_to_GRAM);    // Zapis pod wskazan� pozycj�
	height/=8;

	uint16_t tmpcolor;

	for(uint16_t ox=0; ox < width*height; ox++)
	{
		uint8_t bajt=*data++;
		for(uint8_t b=0; b<8; b++)
		{
			if(bajt & 0x80) tmpcolor=color; else tmpcolor=bkgcolor;
			ssd2119_SendDataByte(tmpcolor >> 8);
			ssd2119_SendDataByte(tmpcolor);
			bajt<<=1;
		}
	}
	_delay_loop_1(4);   //Zaczekaj na opr�nienie bufora nadajnika USART (4*3 takty + co�tam)
	ssd2119_CS(1);
	if(LCD_Driver_Status.Mode565 == false) SED2119_RGB666();  //Wr�� do trybu 3 bajty/piksel
}

void LCD_DrawBitmap_565(uint16_t x, uint8_t y, const uint16_t __flash *data)
{
	if(LCD_Driver_Status.Mode565 == false) SED2119_RGB565();     //Prze��cz si� na tryb przesy�ania 2 bajty na piksel

	uint16_t width=*data++;
	uint8_t height=*data++;

	LCD_SetWindow(x, y, x + width -1, y +  height - 1);
	LCD_SetPosition(x, y);
	ssd2119_SendCmd(ssd2119_Write_to_GRAM);    // Zapis pod wskazan� pozycj�

	for(uint16_t ox=0; ox < width*height - 1; ox++)  //Prze�li wszystkie piksele bitmapy z wyj�tkiem ostatniego
	{
			ssd2119_SendDataByte(*data >> 8);
			ssd2119_SendDataByte(*data++);
	}
	ssd2119_SendDataByte(*data >> 8);   //Prze�lij ostatni piksel
	SPI_RW_Byte(*data++);

	ssd2119_CS(1);
	if(LCD_Driver_Status.Mode565 == false) SED2119_RGB666();  //Wr�� do trybu 3 bajty/piksel
}

void LCD_DrawBitmapRLE_565(uint16_t x, uint8_t y, const uint16_t __flash *data)
{
	if(LCD_Driver_Status.Mode565 == false) SED2119_RGB565();     //Prze��cz si� na tryb przesy�ania 2 bajty na piksel

	uint16_t width=*data++;  //Rozmiary mapy bitowej
	uint8_t height=*data++;
	
	LCD_SetWindow(x, y, x + width -1, y +  height - 1);
	LCD_SetPosition(x, y);
	ssd2119_SendCmd(ssd2119_Write_to_GRAM);    // Zapis pod wskazan� pozycj�
	
	__uint24 counter=width*height;
	
	while(counter)
	{
		int16_t seq=*data++;
		if(seq < 0)
		{
			for(;seq<0;seq++)     //Nie ma sekwencji lecz niepowtarzaj�cy si� ci�g s��w
			{
				ssd2119_SendDataByte(*data >> 8);
				ssd2119_SendDataByte(*data++);
				counter--;
			}
		} else
		{
			uint16_t d0=*data++;
			for(;seq>0;seq--)    //Powt�rz dan� sekwencj�
			{
				ssd2119_SendDataByte(d0 >> 8);
				ssd2119_SendDataByte(d0);
				counter--;
			}
		}
	}

	_delay_loop_1(8);
	ssd2119_CS(1);
	if(LCD_Driver_Status.Mode565 == false) SED2119_RGB666();  //Wr�� do trybu 3 bajty/piksel
	
}

void LCD_DrawBitmapRLE_Mono(uint16_t x, uint8_t y, uint16_t color, uint16_t bkgcolor, const uint8_t __flash *data)
{
	inline void Draw8bits(uint8_t byte)  //Narysuj osiem pikseli kodowanych przez bajt danych
	{
		uint16_t tmpcolor;
		
		for(uint8_t b=0; b<8; b++)
		{
			if(byte & 0x80) tmpcolor=color; else tmpcolor=bkgcolor;
			ssd2119_SendDataByte(tmpcolor >> 8);
			ssd2119_SendDataByte(tmpcolor);
			byte<<=1;
		}
	}
	
	if(LCD_Driver_Status.Mode565 == false) SED2119_RGB565();     //Prze��cz si� na tryb przesy�ania 2 bajty na piksel

	uint16_t width=*(const uint16_t __flash *)data++; data++;
	uint16_t height=*(const uint16_t __flash *)data++; data++;

	LCD_SetWindow(x, y, x + width -1, y +  height - 1);
	LCD_SetPosition(x, y);
	ssd2119_SendCmd(ssd2119_Write_to_GRAM);    // Zapis pod wskazan� pozycj�

	__uint24 counter=width*height;
	
	while(counter)
	{
		int8_t seq=*data++;
		if(seq < 0)
		{
			for(;seq<0;seq++)     //Nie ma sekwencji lecz niepowtarzaj�cy si� ci�g s��w
			{
				Draw8bits(*data++);  //Wy�wietl 8 pikseli kodowanych przez bajt danych
				counter-=8;
			}
		} else
		{
			uint8_t d0=*data++;
			for(;seq>0;seq--)    //Powt�rz dan� sekwencj�
			{
				Draw8bits(d0);  //Wy�wietl 8 pikseli kodowanych przez bajt danych
				counter-=8;
			}
		}
	}

	_delay_loop_1(4);   //Zaczekaj na opr�nienie bufora nadajnika USART (4*3 takty + co�tam)
	ssd2119_CS(1);
	if(LCD_Driver_Status.Mode565 == false) SED2119_RGB666();  //Wr�� do trybu 3 bajty/piksel
}
