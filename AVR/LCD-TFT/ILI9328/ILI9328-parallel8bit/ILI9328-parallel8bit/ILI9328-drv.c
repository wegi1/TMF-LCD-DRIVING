/*
 * SSD2119_drv.c
 *
 * Created: 2014-03-07 17:08:33
 *  Author: tmf
 */

#include <stdbool.h>
#include "ILI9328.h"
#include "i8080-xmega.h"
#include "GFXDrv.h"
#include <util/delay.h>
#include <stdlib.h>

static struct
{
	_Bool FullScreen       : 1;           //true jeœli zosta³ wybrany do zapisu ca³y ekran
	_Bool Mode565          : 1;           //true jeœli mamy tryb przesy³u 2 bajty na piksel, false jeœli 3 bajty na piksel (tryb 18-bitowy)

} LCD_Driver_Status;            //Stan sterownika LCD

void ILI_SendCmdWithData(uint8_t cmd, uint16_t data)
{
	LCD_SendCmd(cmd);
	i8080_Write_W(data);
	LCD_CS(1);     //Deaktywuj kontroler
}

uint16_t ILI_SendCmdReadData(uint8_t cmd)
{
	LCD_SendCmd(cmd);
	uint16_t data=i8080_Read_W();
	LCD_CS(1);     //Deaktywuj kontroler
	return data;
}

void LCD_Init262()
{
	LCD_RESET();   //Zresetuj kontroler

	ILI_SendCmdWithData(ILI9328_DRIVEROUTPUTCONTROL1, 0x0000);  // Driver Output Control Register (R01h)
	ILI_SendCmdWithData(ILI9328_LCDDRIVINGCONTROL, 0x0700);     // LCD Driving Waveform Control (R02h)
	ILI_SendCmdWithData(ILI9328_ENTRYMODE, 0xD030);             // Entry Mode (R03h) - RGB 666 3 transfery przez szynê 8-bitow¹
	ILI_SendCmdWithData(ILI9328_DISPLAYCONTROL2, 0x0302);
	ILI_SendCmdWithData(ILI9328_DISPLAYCONTROL3, 0x0000);
	ILI_SendCmdWithData(ILI9328_DISPLAYCONTROL4, 0x0000);       // Fmark On
	ILI_SendCmdWithData(ILI9328_POWERCONTROL1, 0x0000);         // Power Control 1 (R10h)
	ILI_SendCmdWithData(ILI9328_POWERCONTROL2, 0x0007);         // Power Control 2 (R11h)
	ILI_SendCmdWithData(ILI9328_POWERCONTROL3, 0x0000);         // Power Control 3 (R12h)
	ILI_SendCmdWithData(ILI9328_POWERCONTROL4, 0x0000);         // Power Control 4 (R13h)
	_delay_ms(1);
	ILI_SendCmdWithData(ILI9328_POWERCONTROL1, 0x14B0);         // Power Control 1 (R10h)
	_delay_us(500);
	ILI_SendCmdWithData(ILI9328_POWERCONTROL2, 0x0007);         // Power Control 2 (R11h)
	_delay_us(500);
	ILI_SendCmdWithData(ILI9328_POWERCONTROL3, 0x008E);         // Power Control 3 (R12h)
	ILI_SendCmdWithData(ILI9328_POWERCONTROL4, 0x0C00);         // Power Control 4 (R13h)
	ILI_SendCmdWithData(ILI9328_POWERCONTROL7, 0x0015);         // NVM read data 2 (R29h)
	_delay_us(500);
	ILI_SendCmdWithData(ILI9328_GAMMACONTROL1, 0x0000);         // Gamma Control 1
	ILI_SendCmdWithData(ILI9328_GAMMACONTROL2, 0x0107);         // Gamma Control 2
	ILI_SendCmdWithData(ILI9328_GAMMACONTROL3, 0x0000);         // Gamma Control 3
	ILI_SendCmdWithData(ILI9328_GAMMACONTROL4, 0x0203);         // Gamma Control 4
	ILI_SendCmdWithData(ILI9328_GAMMACONTROL5, 0x0402);         // Gamma Control 5
	ILI_SendCmdWithData(ILI9328_GAMMACONTROL6, 0x0000);         // Gamma Control 6
	ILI_SendCmdWithData(ILI9328_GAMMACONTROL7, 0x0207);         // Gamma Control 7
	ILI_SendCmdWithData(ILI9328_GAMMACONTROL8, 0x0000);         // Gamma Control 8
	ILI_SendCmdWithData(ILI9328_GAMMACONTROL9, 0x0203);         // Gamma Control 9
	ILI_SendCmdWithData(ILI9328_GAMMACONTROL10, 0x0403);        // Gamma Control 10
	ILI_SendCmdWithData(ILI9328_HORIZONTALADDRESSSTARTPOSITION, 0x0000);                      // Window Horizontal RAM Address Start (R50h)
	ILI_SendCmdWithData(ILI9328_HORIZONTALADDRESSENDPOSITION, LCD_GetMaxX() - 1);			  // Window Horizontal RAM Address End (R51h)
	ILI_SendCmdWithData(ILI9328_VERTICALADDRESSSTARTPOSITION, 0X0000);                        // Window Vertical RAM Address Start (R52h)
	ILI_SendCmdWithData(ILI9328_VERTICALADDRESSENDPOSITION, LCD_GetMaxY() - 1);				  // Window Vertical RAM Address End (R53h)
	ILI_SendCmdWithData(ILI9328_DRIVEROUTPUTCONTROL2, 0xa700);    // Driver Output Control (R60h)
	ILI_SendCmdWithData(ILI9328_BASEIMAGEDISPLAYCONTROL, 0x0003); // Driver Output Control (R61h) - enable VLE
	ILI_SendCmdWithData(ILI9328_PANELINTERFACECONTROL1, 0X0010);  // Panel Interface Control 1 (R90h)

// Display On
	ILI_SendCmdWithData(ILI9328_DISPLAYCONTROL1, 0x0133);     // Display Control (R07h)

	LCD_Driver_Status.Mode565=false;    //Tryb przesy³u 3 bajty/piksel
}

void LCD_Init65()
{
	LCD_Init262();
	ILI_RGB565();		//Prze³¹cz w tryb 5-6-5

	LCD_Driver_Status.Mode565=true;    //Tryb przesy³u 3 bajty/piksel
}

//Ustaw pozycjê (x,y) w GRAM
void LCD_SetPosition(uint16_t x1, uint8_t y1)
{
	ILI_SendCmdWithData(ILI9328_HORIZONTALGRAMADDRESSSET, y1);
	ILI_SendCmdWithData(ILI9328_VERTICALGRAMADDRESSSET, x1);
}

//Ustaw okno w pamiêci GRAM
void LCD_SetWindow(uint16_t x1, uint8_t y1, uint16_t x2, uint8_t y2)
{
  ILI_SendCmdWithData(ILI9328_HORIZONTALADDRESSSTARTPOSITION, y1);
  ILI_SendCmdWithData(ILI9328_HORIZONTALADDRESSENDPOSITION, y2);
  ILI_SendCmdWithData(ILI9328_VERTICALADDRESSSTARTPOSITION, x1);
  ILI_SendCmdWithData(ILI9328_VERTICALADDRESSENDPOSITION, x2);
}

void LCD_Rect(uint16_t x1, uint8_t y1, uint16_t x2, uint8_t y2, __uint24 color)
{
	LCD_SetWindow(x1, y1, x2, y2);
	LCD_SetPosition(x1, y1);
	LCD_SendCmd(ILI_Write_to_GRAM);    // Zapis pod wskazan¹ pozycjê

	__uint24 val=(1+y2-y1)*(__uint24)(1+x2-x1) + 1;  //Policz z ilu pikseli sk³ada siê prostok¹t
	for(;val;val--)
	{
		i8080_Write_W(color >> 8);
		i8080_Write_B(color & 0xff);
	}
	LCD_CS(1);
}

void LCD_DrawBitmap_565_Alpha(uint16_t x, uint8_t y, uint8_t alfaneg, const uint16_t __flash *data)
{
	LCD_RGB565 bmppixel;		//Piksel bitmapy
	LCD_BGR565 origpixel;		//Piksel ekranu
	uint8_t red, green, blue;
	
	uint16_t width=*data++;		//Pobierz rozmiary bitmapy
	uint8_t height=*data++;
	uint8_t alfa=~alfaneg;
	
	LCD_SetWindow(x, y, x + width -1, y +  height - 1);
	LCD_SetPosition(x, y);
	
	LCD_SendCmd(ILI_Write_to_GRAM);    // Zapis pod wskazan¹ pozycjê
	i8080_Read_W();					   //Odczytaj dummy byte - dla magistrali 8-bitowej dwukrotnie
	
	uint16_t val=width*height;  //Policz z ilu pikseli sk³ada siê prostok¹t
	
	for(; val>0; val--)
	{
		bmppixel.word=*data++;		      //Pobierz dane do wyœwietlenia
		origpixel.word=i8080_Read_W();    //Odczytaj piksel - ILI odczytuje tylko w formacie 5-6-5
		red=(alfa*(bmppixel.red << 3) + alfaneg*(origpixel.red << 3)) >> 8;
		green=(alfa*(bmppixel.green << 2) + alfaneg*(origpixel.green <<2)) >> 8;
		blue=(alfa*(bmppixel.blue << 3) + alfaneg*(origpixel.blue << 3)) >> 8;
		i8080_Write_W((red << 8) | green);
		i8080_Write_B(blue);
	}
	LCD_CS(1);
}

void LCD_SetPixel(uint16_t x1, uint8_t y1, __uint24 color)
{
	//LCD_SetWindow(x1, y1, x1, y1);		//Dla ILI musimy okreœliæ okno w którym rysowany bêdzie piksel
	LCD_SetPosition(x1, y1);
	LCD_SendCmd(ILI_Write_to_GRAM);    // Zapis pod wskazan¹ pozycjê
	i8080_Write_W(color >> 8);
	i8080_Write_B(color & 0xff);             //Czekamy na wys³anie wszystkich danych
	LCD_CS(1);                               //i dezaktywujemy CS
}

void LCD_LineTo(uint16_t x, uint8_t y, uint16_t x2, uint8_t y2, __uint24 color)
{
	void LCD_SetPixel(uint16_t x1, uint8_t y1, __uint24 color)
	{
		LCD_SetPosition(x1, y1);
		LCD_SendCmd(ILI_Write_to_GRAM);    // Zapis pod wskazan¹ pozycjê
		i8080_Write_W(color >> 8);
		i8080_Write_B(color & 0xff);
	}

	int16_t dy;
	int16_t dx;
	int8_t stepx, stepy;

	LCD_SetWindow(0, 0, LCD_GetMaxX()-1, LCD_GetMaxY()-1);

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
	LCD_CS(1); //i dezaktywujemy CS
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
	LCD_SetPosition(x1, y1);
	LCD_SendCmd(ILI_Write_to_GRAM);    // Zapis pod wskazan¹ pozycjê

	uint8_t alfa=~alfaneg;
	i8080_Write_B((alfa*((LCD_RGB)color).red + alfaneg*(((LCD_RGB)bkgcolor).red)) >> 8);     //Blending kolorów
	i8080_Write_B((alfa*((LCD_RGB)color).green + alfaneg*(((LCD_RGB)bkgcolor).green)) >> 8);
	i8080_Write_B((alfa*((LCD_RGB)color).blue + alfaneg*(((LCD_RGB)bkgcolor).blue)) >> 8);
}

void LCD_LineToAA(uint16_t x0, uint8_t y0, uint16_t x1, uint8_t y1, __uint24 color, __uint24 bkgcolor)
{
   LCD_SetWindow(0, 0, LCD_GetMaxX()-1, LCD_GetMaxY()-1);
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
	LCD_SetWindow(0, 0, LCD_GetMaxX()-1, LCD_GetMaxY()-1);
   int x=-r, y=0;
   int x2; 
   long e2, intensity, err=2-2*r;
   r = 1-err;
   do {
      intensity=255*abs(err-2*(x+y)-2)/r;                   //Oblicz b³¹d - intensywnoœæ piksela
      setPixelAA(x0-x, y0+y, intensity, color, bkgcolor);   //Rysujemy odbicie symetryczne w czterech kwadrantach
      setPixelAA(x0-y, y0-x, intensity, color, bkgcolor); 
      setPixelAA(x0+x, y0-y, intensity, color, bkgcolor); 
      setPixelAA(x0+y, y0+x, intensity, color, bkgcolor); 
      e2=err; x2=x;
      if(err+y > 0)
	  {                                              // Krok na osi x
         intensity=255*(err-2*x-1)/r;                //Piksel zewnêtrzny
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
         intensity=255*(2*y+3-e2)/r;                // Piksel wewnêtrzny
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
	uint8_t rows=(uint8_t)(uint16_t)font[1]; //Pobierz wysokoœæ fontu
	char ch;
	y+=rows-1;

	while((ch=*tekst++))  //Wyœwietl kolejne znaki a¿ do koñca tekstu (znaku NUL)
	{
		const uint8_t __flash *znak=font[ch-29]; //Adres pocz¹tku opisu znaku
		uint8_t col=*znak++;                     //Szerokoœæ znaku w pikselach
		uint8_t coldesc=coldesc;                 //Bajt opisu znaku
		uint8_t pixelcnt=0;                      //Licznik pikseli w znaku

		for(uint8_t ox=0; ox < col; ox++)        //Wyœwietlamy kolejne kolumny tworz¹ce znak
		{
			for(uint8_t oy=0; oy < rows; oy++)   //Narysuj jedn¹ kolumnê znaku
			{
				if((pixelcnt & 0b111) == 0) coldesc=*znak++;     //Pobierz bajt opisu znaku
				pixelcnt++;
				if(coldesc & 0x80) LCD_SetPixel(x, y-oy, color);  //Wyœwietlamy tylko piksele znaku, pomijamy t³o - ma byæ przezroczyste
				coldesc<<=1;
			}
			x++;  //Rysujemy kolejn¹ kolumnê
			if(x >= LCD_GetMaxX()) return;   //Jesteœmy poza obszarem kreœlenia
		}
	}
}

void LCD_SetTextOpaque(uint16_t x, uint8_t y, const __memx char *tekst, const uint8_t __flash * const __flash font[], __uint24 color, __uint24 bkgcolor)
{
	uint8_t rows=(uint8_t)(uint16_t)font[1]; //Pobierz wysokoœæ fontu
	char ch;
	__uint24 pixcolor;    //Kolor rysowanego piksela


	ILI_SendCmdWithData(ILI9328_ENTRYMODE, (ILI9328_EntryMode_Reg){.TRI=1, .DFM=1, .BGR=1, .ID=0b10, .AM=0}.word);

	while((ch=*tekst++))  //Wyœwietl kolejne znaki a¿ do koñca tekstu (znaku NUL)
	{
		const uint8_t __flash *znak=font[ch-29]; //Adres pocz¹tku opisu znaku
		uint8_t col=*znak++;                     //Szerokoœæ znaku w pikselach
		uint8_t coldesc=coldesc;                 //Bajt opisu znaku
		uint8_t pixelcnt=0;                      //Licznik pikseli w znaku

		LCD_SetWindow(x, y, x + col -1, y +  rows - 1); //Okno w którym zapiszemy znak
		LCD_SetPosition(x, y + rows - 1);        //Pocz¹tek okna

		LCD_SendCmd(ILI_Write_to_GRAM);      // Zapis pod wskazan¹ pozycjê
		for(uint8_t ox=0; ox < col; ox++)        //Wyœwietlamy kolejne kolumny tworz¹ce znak
		{
			for(uint8_t oy=0; oy < rows; oy++)   //Narysuj jedn¹ kolumnê znaku
			{
				if((pixelcnt & 0b111) == 0) coldesc=*znak++;     //Pobierz bajt opisu znaku
				pixelcnt++;
				if(coldesc & 0x80) pixcolor=color;
				else pixcolor=bkgcolor;         //Narysuj t³o tylko jeœli nie wyœwietlamy przeŸroczystego tekstu
				i8080_Write_W(pixcolor >> 8);  //Zapisz do GRAM kolor piksela
				i8080_Write_B(pixcolor & 0xff);
				coldesc<<=1;
			}
			x++;  //Rysujemy kolejn¹ kolumnê
		}
		LCD_CS(1);
	}
	ILI_SendCmdWithData(ILI9328_ENTRYMODE, (ILI9328_EntryMode_Reg){.TRI=1, .DFM=1, .BGR=1, .ID=0b11, .AM=1}.word);
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
		
		for(uint16_t ox=0; ox < pixelsno; ox++)        //Wyœwietlamy kolejne kolumny tworz¹ce znak
		{
			if((pixelcnt & 0b111) == 0) coldesc=*znak++;     //Pobierz bajt opisu znaku
			pixelcnt++;
			if(coldesc & 0x80) pixcolor=color;
			else pixcolor=bkgcolor;                //Narysuj t³o tylko jeœli nie wyœwietlamy przeŸroczystego tekstu
			i8080_Write_W(pixcolor >> 8);  //Zapisz do GRAM kolor piksela
			i8080_Write_B(pixcolor & 0xff);
			coldesc<<=1;
		}	
	}

//
// Funkcja renderuj¹ca znak z wykorzystaniem 2 bitowego antyaliasingu
//	
	void CharRenderFunc2bitMono(uint16_t pixelsno, uint8_t rows, const uint8_t __flash *znak, __uint24 color, __uint24 bkgcolor)
	{
		uint8_t alfa, alfaneg;
		uint8_t pixcnt=0;
		uint8_t alfavalue=alfavalue;

		for(uint16_t ox=0; ox < pixelsno; ox++)        //Wyœwietlamy kolejne piksele tworz¹ce znak
		{
			if((pixcnt & 0b11) == 0) alfavalue=*znak++;
			alfa=alfavalue & 0b11000000;
			if(alfa) alfa|=0b00111111;
			alfaneg=~alfa;
			i8080_Write_B((alfa*((LCD_RGB)color).red + alfaneg*(((LCD_RGB)bkgcolor).red)) >> 8);     //Blending kolorów
			pixcnt++;
			alfavalue<<=2;
			i8080_Write_B((alfa*((LCD_RGB)color).green + alfaneg*(((LCD_RGB)bkgcolor).green)) >> 8);
			if(pixcnt >= rows) pixcnt=0;
			i8080_Write_B((alfa*((LCD_RGB)color).blue + alfaneg*(((LCD_RGB)bkgcolor).blue)) >> 8);
		}		
	}

//
// Funkcja renderuj¹ca znak z wykorzystaniem 8 bitowego antyaliasingu mono
//
	void CharRenderFunc8bitMono(uint16_t pixelsno, uint8_t rows, const uint8_t __flash *znak, __uint24 color, __uint24 bkgcolor)
	{
		for(uint16_t ox=0; ox < pixelsno; ox++)        //Wyœwietlamy kolejne piksele tworz¹ce znak
		{
			uint8_t alfa=*znak++;
			uint8_t alfaneg=~alfa;
			i8080_Write_B((alfa*((LCD_RGB)color).red + alfaneg*(((LCD_RGB)bkgcolor).red)) >> 8);     //Blending kolorów
			i8080_Write_B((alfa*((LCD_RGB)color).green + alfaneg*(((LCD_RGB)bkgcolor).green)) >> 8);
			i8080_Write_B((alfa*((LCD_RGB)color).blue + alfaneg*(((LCD_RGB)bkgcolor).blue)) >> 8);
		}
	}

//
// Funkcja renderuj¹ca znak z wykorzystaniem 8 bitowego antyaliasingu subpikselowego (3-3-2)
//
	void CharRenderFunc322Subpixel(uint16_t pixelsno, uint8_t rows, const uint8_t __flash *znak, __uint24 color, __uint24 bkgcolor)
	{
		for(uint16_t ox=0; ox < pixelsno; ox++)   //Wyœwietlamy kolejne piksele tworz¹ce znak
		{
			LCD_RGB332 tmpalfa={.color=*znak++};
			uint8_t alfa, alfaneg;
			alfa=tmpalfa.red << 5; if(alfa) alfa|=0b00011111; alfaneg=~alfa; 
			i8080_Write_B((alfa*((LCD_RGB)color).red + alfaneg*(((LCD_RGB)bkgcolor).red)) >> 8);     //Blending kolorów
			alfa=tmpalfa.green << 5; if(alfa) alfa|=0b00011111; alfaneg=~alfa; 
			i8080_Write_B((alfa*((LCD_RGB)color).green + alfaneg*(((LCD_RGB)bkgcolor).green)) >> 8);
			alfa=tmpalfa.blue << 6; if(alfa) alfa|=0b00111111; alfaneg=~alfa; 
			i8080_Write_B((alfa*((LCD_RGB)color).blue + alfaneg*(((LCD_RGB)bkgcolor).blue)) >> 8);
		}
	}
	
	uint8_t rows=(uint8_t)(uint16_t)font[1]; //Pobierz wysokoœæ fontu, dziwna konwersja wynika z zapisu w tablicy sta³ej jako wskaŸnika

	void (*RenderFuncPtr)(uint16_t pikselsno, uint8_t rows, const uint8_t __flash *znak, __uint24 color, __uint24 bkgcolor); //WskaŸnik na funkcjê renderuj¹c¹

	switch((uint8_t)(uint16_t)font[0])
	{
		case 0x01:       RenderFuncPtr=&CharRenderFunc1bitNoAA;  //Renderujemy czcionkê bez antyaliasingiem 1bpp
						 ILI_SendCmdWithData(ILI9328_ENTRYMODE, (ILI9328_EntryMode_Reg){.TRI=1, .DFM=1, .BGR=1, .ID=0b10, .AM=0}.word);
						 break;
		case 0x82:       RenderFuncPtr=&CharRenderFunc2bitMono;  //Renderujemy czcionkê z antyaliasingiem 2bpp mono
						 ILI_SendCmdWithData(ILI9328_ENTRYMODE, (ILI9328_EntryMode_Reg){.TRI=1, .DFM=1, .BGR=1, .ID=0b10, .AM=0}.word);
						 break;
		case 0x88:       RenderFuncPtr=&CharRenderFunc8bitMono;
						 ILI_SendCmdWithData(ILI9328_ENTRYMODE, (ILI9328_EntryMode_Reg){.TRI=1, .DFM=1, .BGR=1, .ID=0b11, .AM=1}.word);
						 break;      //Renderujemy czcionkê z antyaliasingiem 8bpp mono
		case 0xA8:       RenderFuncPtr=&CharRenderFunc322Subpixel;
						 ILI_SendCmdWithData(ILI9328_ENTRYMODE, (ILI9328_EntryMode_Reg){.TRI=1, .DFM=1, .BGR=1, .ID=0b11, .AM=1}.word);
						 break;   //Renderujemy czcionkê z antyaliasingiem 8bpp kolor 3-3-2
		default:         RenderFuncPtr=CharRenderFuncNone;
	};

	char ch;
	while((ch=*tekst++))  //Wyœwietl kolejne znaki a¿ do koñca tekstu (znaku NUL)
	{
		const uint8_t __flash *znak=font[ch-29]; //Adres pocz¹tku opisu znaku
		uint8_t col=*znak++;                     //Szerokoœæ znaku w pikselach

		LCD_SetWindow(x, y, x + col -1 , y + rows -1); //Okno w którym zapiszemy znak
		LCD_SetPosition(x, y + rows - 1);              //Pocz¹tek okna
		LCD_SendCmd(ILI_Write_to_GRAM);        // Zapis pod wskazan¹ pozycjê
		
		RenderFuncPtr(col*rows, rows, znak, color, bkgcolor);  //Wyœwietl znak stosuj¹c wybran¹ technikê renderowania
		
		x+=col;
		if(x >= LCD_GetMaxX()) break;   //Jesteœmy poza obszarem kreœlenia
	}

	LCD_CS(1);
	ILI_SendCmdWithData(ILI9328_ENTRYMODE, (ILI9328_EntryMode_Reg){.TRI=1, .DFM=1, .BGR=1, .ID=0b11, .AM=0}.word);
}

void LCD_DrawBitmap_Mono(uint16_t x, uint8_t y, uint16_t color, uint16_t bkgcolor, const uint8_t __flash *data)
{
	if(LCD_Driver_Status.Mode565 == false) ILI_RGB565();     //Prze³¹cz siê na tryb przesy³ania 2 bajty na piksel

	uint16_t width=*(const uint16_t __flash *)data++; data++;
	uint16_t height=*(const uint16_t __flash *)data++; data++;

	LCD_SetWindow(x, y, x + width -1, y +  height - 1);
	LCD_SetPosition(x, y);
	LCD_SendCmd(ILI_Write_to_GRAM);    // Zapis pod wskazan¹ pozycjê
	height/=8;

	uint16_t tmpcolor;

	for(uint16_t ox=0; ox < width*height; ox++)
	{
		uint8_t bajt=*data++;
		for(uint8_t b=0; b<8; b++)
		{
			if(bajt & 0x80) tmpcolor=color; else tmpcolor=bkgcolor;
			i8080_Write_W(tmpcolor);
			bajt<<=1;
		}
	}
	LCD_CS(1);
	if(LCD_Driver_Status.Mode565 == false) ILI_RGB666();  //Wróæ do trybu 3 bajty/piksel
}

void LCD_DrawBitmap_565(uint16_t x, uint8_t y, const uint16_t __flash *data)
{
	if(LCD_Driver_Status.Mode565 == false) ILI_RGB565();     //Prze³¹cz siê na tryb przesy³ania 2 bajty na piksel

	uint16_t width=*data++;
	uint8_t height=*data++;

	LCD_SetWindow(x, y, x + width -1, y +  height - 1);
	LCD_SetPosition(x, y);
	LCD_SendCmd(ILI_Write_to_GRAM);    // Zapis pod wskazan¹ pozycjê

	for(uint16_t ox=0; ox < width*height - 1; ox++)  //Przeœli wszystkie piksele bitmapy z wyj¹tkiem ostatniego
	{
			i8080_Write_W(*data++);
	}
	i8080_Write_W(*data++);

	LCD_CS(1);
	if(LCD_Driver_Status.Mode565 == false) ILI_RGB666();  //Wróæ do trybu 3 bajty/piksel
}
