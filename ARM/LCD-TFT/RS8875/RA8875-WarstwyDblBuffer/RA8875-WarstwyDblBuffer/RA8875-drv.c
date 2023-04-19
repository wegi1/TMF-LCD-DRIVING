/*
 * SSD2119_drv.c
 *
 * Created: 2014-03-07 17:08:33
 *  Author: tmf
 */

#include <stdbool.h>
#include "RA8875.h"
#include "i8080-arm.h"
#include "GFXDrv.h"
#include "Delay/delay.h"
#include <stdlib.h>
#include <math.h>

void RA_SendCmdWithData(uint8_t cmd, uint8_t data)
{
	LCD_SendCmd(cmd);
	i8080_Write_W(data);			//W RA8875 wszelkie rejestry s¹ tylko 8 bitowe niezale¿nie od wybanej szerokoœci magistrali
	LCD_CS(1);     //Deaktywuj kontroler
}

void RA_SendCmdWithDataW(uint8_t cmd, uint16_t data)
{
	LCD_SendCmd(cmd);
	i8080_Write_W(data & 0xFF);		//Wyœlij mniej znacz¹c¹ po³owê argumentu
	LCD_SendCmd(cmd+1);				//Wyœlij bardziej znacz¹c¹ po³owê argumentu
	i8080_Write_W(data >> 8);
}

uint16_t RA_SendCmdReadData(uint8_t cmd)
{
	LCD_SendCmd(cmd);
	uint16_t data=i8080_Read_W();	//W RA8875 wszelkie rejestry s¹ tylko 8 bitowe niezale¿nie od wybanej szerokoœci magistrali
	LCD_CS(1);     //Deaktywuj kontroler
	return data;
}

uint8_t RA_ReadStatus()
{
	uint8_t status;
	LCD_RS(1);
	LCD_CS(0);
	status=i8080_Read_W();
	LCD_CS(1);
	LCD_RS(0);
	return status;
}

void RA_BTE_SetBkgColor(uint16_t color)
{
	LCD_RGB565 col;
	col.word=color;
	
	RA_SendCmdWithData(RA_Background_Color_Register0, col.red);
	RA_SendCmdWithData(RA_Background_Color_Register1, col.green);
	RA_SendCmdWithData(RA_Background_Color_Register2, col.blue);
}

void RA_BTE_SetColor(uint16_t color)
{
	LCD_RGB565 col;
	col.word=color;
	
	RA_SendCmdWithData(RA_Foreground_Color_Register0, col.red);
	RA_SendCmdWithData(RA_Foreground_Color_Register1, col.green);
	RA_SendCmdWithData(RA_Foreground_Color_Register2, col.blue);
}

#ifdef LCD_P320x240

const uint8_t RA_Init[]={0x88, 0x0a, 0xFF, 0x89, 0x02, 0xFF, 0x10, 0x0f, 0xFF, 0x04, 0x03, 0x14, 0x27, 0x15, 0x00, 0x16, 0x05, 0x17, 0x04, 0x18, 0x03, 0x19, 0xef, 0x1a, 0x00, 0x1b, 0x05, 0x1c, 0x00,
0x1d, 0x0e, 0x1e, 0x00, 0x1f, 0x02};

#endif

#ifdef LCD_P480x272

const uint8_t RA_Init[]={0x88, 0x0a, 0xFF, 0x89, 0x02, 0xFF, 0x10, 0x0f, 0xFF, 0x04, 0x82, 0x14, 0x3b, 0x15, 0x00, 0x16, 0x01, 0x17, 0x00, 0x18, 0x05, 0x19, 0x0f, 0x1a, 0x01, 0x1b, 0x02, 0x1c, 0x00,
0x1d, 0x07, 0x1e, 0x00, 0x1f, 0x09};

#endif

#ifdef LCD_P640x480

const uint8_t RA_Init[]={0x88, 0x0b, 0xFF, 0x89, 0x02, 0xFF, 0x10, 0x0f, 0xFF, 0x04, 0x01, 0x14, 0x4f, 0x15, 0x05, 0x16, 0x0f, 0x17, 0x01, 0x18, 0x00, 0x19, 0xdf, 0x1a, 0x01, 0x1b, 0x0a, 0x1c, 0x00,
0x1d, 0x0e, 0x1e, 0x00, 0x1f, 0x01};

#endif

#ifdef LCD_P800x480

const uint8_t RA_Init[]={0x88, 0x0b, 0xFF, 0x89, 0x02, 0xFF, 0x10, 0x0f, 0xFF, 0x04, 0x81, 0x14, 0x63, 0x15, 0x03, 0x16, 0x03, 0x17, 0x02, 0x18, 0x00, 0x19, 0xdf, 0x1a, 0x01, 0x1b, 0x14, 0x1c, 0x00,
0x1d, 0x06, 0x1e, 0x00, 0x1f, 0x01};

#endif

void LCD_Init65k()
{
	RA_RESET();   //Zresetuj kontroler

	for(uint8_t i=0; i < (sizeof(RA_Init) / sizeof(RA_Init[0])); i++)
	{
		if(RA_Init[i]==0xff) delay_ms(1);
		else
		{
			RA_SendCmdWithData(RA_Init[i], RA_Init[i+1]);
			i++;
		}
	}
	
	RA_SendCmdWithData(RA_PWM1_Duty_Cycle_Register, 0x80); //Jasnoœæ podœwietlenia
	RA_SendCmdWithData(RA_PWM1_Control_Register, (RS8875_PWMCR_Reg){.PWMEn=true, .PWMDiv=RA_PWMDiv2}.byte); //Konfiguracja PWM1
	
	#if defined LCD_P480x272 || defined LCD_P320x240
	//Potrzebujemy poni¿sze polecenie jeœli zamierzamy wykorzystaæ resztê GRAM nale¿¹cego do drugiej warstwy - dzia³a tylko w trybach, w których mamy wystarczaj¹c¹ iloœæ pamiêci
	RA_SendCmdWithData(RA_Display_Configuration_Register, (RS8875_DPCR_Reg){.VDIR=0, .HDIR=0, .Layers=1}.byte);
	#endif
	
	LCD_SetWindow(0, 0, LCD_GetMaxX(), LCD_GetMaxY());	//Ustaw okno dostêpu na ca³y ekran

	RA_SendCmdWithData(RA_Power_and_Display_Control_Register, (RS8875_PWRR_Reg) {.LCDOn=true}.byte);
}

void LCD_Init256()
{
	RA_RESET();   //Zresetuj kontroler
	LCD_Init65k();
	RA_SendCmdWithData(RA_System_Configuration_Register, (RS8875_SYSR_Reg){.MCUIF=RA_MCUIF_16b, .ColorDepth=RA_ColorDepth_8bpp}.byte);	 //256 kolorów, 16b interfejs
	//Potrzebujemy poni¿sze polecenie jeœli zamierzamy wykorzystaæ resztê GRAM nale¿¹cego do drugiej warstwy - dzia³a tylko w trybach, w których mamy wystarczaj¹c¹ iloœæ pamiêci
	RA_SendCmdWithData(RA_Display_Configuration_Register, (RS8875_DPCR_Reg){.VDIR=0, .HDIR=0, .Layers=1}.byte);
}

//Ustaw pozycjê zapisu (x,y) w GRAM
void LCD_SetPosition(uint16_t x, uint16_t y)
{
	RA_SendCmdWithDataW(RA_Memory_Write_Cursor_Vertical_Position_Register0, y);
	RA_SendCmdWithDataW(RA_Memory_Write_Cursor_Horizontal_Position_Register0, x);
}

//Ustaw pozycjê odczytu (x,y) w GRAM
void LCD_SetReadPosition(uint16_t x, uint16_t y)
{
	RA_SendCmdWithDataW(RA_Memory_Read_Cursor_Vertical_Position_Register0, y);
	RA_SendCmdWithDataW(RA_Memory_Read_Cursor_Horizontal_Position_Register0, x);
}

//Ustaw okno w pamiêci GRAM
void LCD_SetWindow(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	RA_SendCmdWithDataW(RA_Horizontal_Start_Point_0_of_Active_Window, x1);
	RA_SendCmdWithDataW(RA_Vertical_Start_Point_0_of_Active_Window, y1);
	RA_SendCmdWithDataW(RA_Horizontal_End_Point_0_of_Active_Window, x2);
	RA_SendCmdWithDataW(RA_Vertical_End_Point_of_Active_Window_0, y2);
}

void RA_BTE_SetSrc(uint16_t x, uint16_t y, uint8_t Layer)
{
	RA_SendCmdWithDataW(RA_Horizontal_Source_Point_0_of_BTE, x);
	if(Layer) y|=0x8000;   //Wybierz warstwê nr 1 - warstwa jest w tym samym rejeestrze co starsza czêœæ wsp. y
	RA_SendCmdWithDataW(RA_Vertical_Source_Point_0_of_BTE, y);
}

void RA_BTE_SetDst(uint16_t x, uint16_t y, uint8_t Layer)
{
	RA_SendCmdWithDataW(RA_Horizontal_Destination_Point_0_of_BTE, x);
	if(Layer) y|=0x8000;   //Wybierz warstwê nr 1 - warstwa jest w tym samym rejeestrze co starsza czêœæ wsp. y
	RA_SendCmdWithDataW(RA_Vertical_Destination_Point_0_of_BTE, y);
}

void RA_BTE_SetWidthHeight(uint16_t width, uint16_t height)
{
	RA_SendCmdWithDataW(RA_BTE_Width_Register0, width);
	RA_SendCmdWithDataW(RA_BTE_Height_Register0, height);
}

void RA_BTE_Move(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t width, uint16_t height, uint8_t srcLayer, uint8_t dstLayer, uint8_t direction)
{
	RA_BTE_SetSrc(x1, y1, srcLayer);
	RA_BTE_SetDst(x2, y2, dstLayer);
	RA_BTE_SetWidthHeight(width, height);
	
	if(direction) RA_SendCmdWithData(RA_BTE_Function_Control_Register1, (RS8875_BECR1_Reg){.ROPOperation=RA_BTEOp_MovePositive, .OpType=12}.byte);
	else RA_SendCmdWithData(RA_BTE_Function_Control_Register1, (RS8875_BECR1_Reg){.ROPOperation=RA_BTEOp_MoveNegative, .OpType=12}.byte);
	
	RA_SendCmdWithData(RA_BTE_Function_Control_Register0, (RS8875_BECR0_Reg){.DstDataType=RA_BTERectangular, .SrcDataType=RA_BTERectangular, .Enable=true}.byte); //Rozpocznij operacjê
}

void LCD_Rect(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
	LCD_SetWindow(x1, y1, x2, y2);
	RA_SendCmdWithData(RA_Background_Color_Register0, (LCD_RGB565){.word=color}.red);
	RA_SendCmdWithData(RA_Background_Color_Register1, (LCD_RGB565){.word=color}.green);
	RA_SendCmdWithData(RA_Background_Color_Register2, (LCD_RGB565){.word=color}.blue);
	RA_SendCmdWithData(RA_Memory_Clear_Control_Register, (RS8875_MCLR_Reg){.CLRArea=true, .MCLR=true}.byte); //Wyczyœæ okienko
	while(RA_SendCmdReadData(RA_Memory_Clear_Control_Register) & (RS8875_MCLR_Reg) {.MCLR=true}.byte);		//Zaczekaj na koniec operacji
}

void LCD_DrawBitmap_Mono(uint16_t x, uint16_t y, uint16_t color, uint16_t bkgcolor, const uint8_t *data, _Bool transparency)
{
	uint16_t width=*(const uint16_t *)data++; data++;
	uint16_t height=*(const uint16_t *)data++; data++;
	
	RA_WaitForBTEReady();			//Jeœli aktualnie toczy siê jakaœ operacja to zaczekaj na jej zakoñczenie
	RA_BTE_SetBkgColor(bkgcolor);	//Ustaw kolor t³a bitmapy - niepotrzebne, jeœli korzystamy z przezroczystoœci
	RA_BTE_SetColor(color);			//Ustaw kolor pikseli
	
	RA_BTE_SetDst(x, y, 0);
	RA_BTE_SetWidthHeight(width, height);
	
	RA_BTEROPOperationCode func=RA_BTEOp_ColorExpansion;
	if(transparency) func=RA_BTEOp_ColorExpansionWithTransparency; //Wyœwietlamy z przezroczystoœci¹

	RA_SendCmdWithData(RA_BTE_Function_Control_Register1, (RS8875_BECR1_Reg){.ROPOperation=func, .OpType=15}.byte);
	RA_SendCmdWithData(RA_BTE_Function_Control_Register0, (RS8875_BECR0_Reg){.DstDataType=RA_BTERectangular, .Enable=true}.byte); //Rozpocznij operacjê

	LCD_SendCmd(RA_Memory_Read_Write_Command);    // Zapis pod wskazan¹ pozycjê
	
	uint8_t owidth=(width+7)/8;  //Oblicz liczbe bajtów w linii z zaokr¹gleniem w górê
	uint8_t app=0;
	if(owidth & 1) {app=1; owidth&=0xfe;}
	
	for(uint16_t oy=0; oy<height; oy++)
	{
		for(uint8_t ox=0; ox<owidth; ox+=2)
		{
			uint16_t wrd=((uint16_t)(*data++)) << 8;
			wrd|=((uint16_t)*data++);
			i8080_Write_W(wrd);
			RA_WaitForWAIT();			//Zaczekaj na wykonanie polecenia
		}
		if(app)
		{
			uint16_t wrd=((uint16_t)(*data++)) << 8;
			i8080_Write_W(wrd);
			RA_WaitForWAIT();			//Zaczekaj na wykonanie polecenia
		}
	}
	LCD_CS(1);
}

void LCD_DrawBitmap_565(uint16_t x, uint16_t y, const uint16_t *data)
{
	uint16_t width=*data++;
	uint8_t height=*data++;

	LCD_SetWindow(x, y, x + width -1, y +  height - 1);
	LCD_SetPosition(x, y);
	LCD_SendCmd(RA_Memory_Read_Write_Command);    // Zapis pod wskazan¹ pozycjê

	for(uint16_t ox=0; ox < width*height - 1; ox++)  //Przeœli wszystkie piksele bitmapy z wyj¹tkiem ostatniego
	{
		i8080_Write_W(*data++);
	}
	i8080_Write_W(*data++);

	LCD_CS(1);
}

void LCD_SetPixel(uint16_t x, uint8_t y, uint16_t color)
{
	LCD_SetPosition(x, y);
	LCD_SendCmd(RA_Memory_Read_Write_Command);    // Zapis pod wskazan¹ pozycjê
	i8080_Write_W(color);
	LCD_CS(1);				                      //i dezaktywujemy CS
}

static void setPixelAA(uint16_t x, uint8_t y, uint8_t alfaneg, uint16_t color, uint16_t bkgcolor)
{
	LCD_SetPosition(x, y);
	LCD_SendCmd(RA_Memory_Read_Write_Command);    // Zapis pod wskazan¹ pozycjê

	uint8_t alfa=~alfaneg;
	uint8_t red=(alfa*((LCD_RGB565)color).red + alfaneg*(((LCD_RGB565)bkgcolor).red)) >> 8;     //Blending kolorów
	uint8_t green=(alfa*((LCD_RGB565)color).green + alfaneg*(((LCD_RGB565)bkgcolor).green)) >> 8;
	uint8_t blue=(alfa*((LCD_RGB565)color).blue + alfaneg*(((LCD_RGB565)bkgcolor).blue)) >> 8;
	LCD_RGB565 col=(LCD_RGB565){.red=red, .green=green, .blue=blue};
	i8080_Write_W(col.word);
}

void LCD_LineToAA(uint16_t x0, uint8_t y0, uint16_t x1, uint8_t y1, uint16_t color, uint16_t bkgcolor)
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

void LCD_CircleAA(uint16_t x0, uint8_t y0, int r, uint16_t color, uint16_t bkgcolor)
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

void LCD_SetTextAA(uint16_t x, uint8_t y, const char *tekst, const uint8_t * const font[], uint16_t color, uint16_t bkgcolor)
{
	//
	// Pusta funkcja na wypadek napotkania nieznanego deskryptora
	//
	void CharRenderFuncNone(uint16_t pixelsno, uint8_t rows, const uint8_t *znak, uint16_t color, uint16_t bkgcolor)
	{
	}
	
	//
	// Funkcja wyœwietlaj¹ca znak bez antyaliasingu
	//
	void CharRenderFunc1bitNoAA(uint16_t pixelsno, uint8_t rows, const uint8_t *znak, uint16_t color, uint16_t bkgcolor)
	{
		uint8_t coldesc=coldesc; //Bajt opisu znaku
		uint8_t pixelcnt=0;      //Licznik pikseli w znaku
		uint16_t pixcolor;       //Kolor rysowanego piksela
		
		for(uint16_t ox=0; ox < pixelsno; ox++)        //Wyœwietlamy kolejne kolumny tworz¹ce znak
		{
			if((pixelcnt & 0b111) == 0) coldesc=*znak++;     //Pobierz bajt opisu znaku
			pixelcnt++;
			if(coldesc & 0x80) pixcolor=color;
			else pixcolor=bkgcolor;                //Narysuj t³o tylko jeœli nie wyœwietlamy przeŸroczystego tekstu
			i8080_Write_W(pixcolor);
			coldesc<<=1;
		}
	}

	//
	// Funkcja renderuj¹ca znak z wykorzystaniem 2 bitowego antyaliasingu
	//
	void CharRenderFunc2bitMono(uint16_t pixelsno, uint8_t rows, const uint8_t *znak, uint16_t color, uint16_t bkgcolor)
	{
		uint8_t alfa, alfaneg;
		uint8_t pixcnt=0;
		uint8_t alfavalue=alfavalue;
		
		LCD_RGB rgbcolor=(LCD_RGB){.red=((LCD_RGB565)color).red<<3, .green=((LCD_RGB565)color).green<<2, .blue=((LCD_RGB565)color).blue<<3};
		LCD_RGB rgbbkg=(LCD_RGB){.red=((LCD_RGB565)bkgcolor).red<<3, .green=((LCD_RGB565)bkgcolor).green<<2, .blue=((LCD_RGB565)bkgcolor).blue<<3};

		for(uint16_t ox=0; ox < pixelsno; ox++)        //Wyœwietlamy kolejne piksele tworz¹ce znak
		{
			if((pixcnt & 0b11) == 0) alfavalue=*znak++;
			alfa=alfavalue & 0b11000000;
			if(alfa) alfa|=0b00111111;
			alfaneg=~alfa;
			uint8_t red=(alfa*rgbcolor.red + alfaneg*rgbbkg.red) >> 8;     //Blending kolorów
			pixcnt++;
			alfavalue<<=2;
			uint8_t green=(alfa*rgbcolor.green + alfaneg*rgbbkg.green) >> 8;
			if(pixcnt >= rows) pixcnt=0;
			uint8_t blue=(alfa*rgbcolor.blue + alfaneg*rgbbkg.blue) >> 8;
			
			LCD_RGB565 col=(LCD_RGB565){.red=red >> 3, .green=green >> 2, .blue=blue >> 3};
			i8080_Write_W(col.word);
		}
	}

	//
	// Funkcja renderuj¹ca znak z wykorzystaniem 8 bitowego antyaliasingu mono
	//
	void CharRenderFunc8bitMono(uint16_t pixelsno, uint8_t rows, const uint8_t *znak, uint16_t color, uint16_t bkgcolor)
	{
		LCD_RGB rgbcolor=(LCD_RGB){.red=((LCD_RGB565)color).red<<3, .green=((LCD_RGB565)color).green<<2, .blue=((LCD_RGB565)color).blue<<3};
		LCD_RGB rgbbkg=(LCD_RGB){.red=((LCD_RGB565)bkgcolor).red<<3, .green=((LCD_RGB565)bkgcolor).green<<2, .blue=((LCD_RGB565)bkgcolor).blue<<3};
		
		for(uint16_t ox=0; ox < pixelsno; ox++)        //Wyœwietlamy kolejne piksele tworz¹ce znak
		{
			uint8_t alfa=*znak++;
			uint8_t alfaneg=~alfa;
			uint8_t red=(alfa*rgbcolor.red + alfaneg*rgbbkg.red) >> 8;     //Blending kolorów
			uint8_t green=(alfa*rgbcolor.green + alfaneg*rgbbkg.green) >> 8;
			uint8_t blue=(alfa*rgbcolor.blue + alfaneg*rgbbkg.blue) >> 8;
			
			LCD_RGB565 col=(LCD_RGB565){.red=red >> 3, .green=green >> 2, .blue=blue >> 3};
			i8080_Write_W(col.word);
		}
	}

	//
	// Funkcja renderuj¹ca znak z wykorzystaniem 8 bitowego antyaliasingu subpikselowego (3-3-2)
	//
	void CharRenderFunc322Subpixel(uint16_t pixelsno, uint8_t rows, const uint8_t *znak, uint16_t color, uint16_t bkgcolor)
	{
		LCD_RGB rgbcolor=(LCD_RGB){.red=((LCD_RGB565)color).red<<3, .green=((LCD_RGB565)color).green<<2, .blue=((LCD_RGB565)color).blue<<3};
		LCD_RGB rgbbkg=(LCD_RGB){.red=((LCD_RGB565)bkgcolor).red<<3, .green=((LCD_RGB565)bkgcolor).green<<2, .blue=((LCD_RGB565)bkgcolor).blue<<3};

		for(uint16_t ox=0; ox < pixelsno; ox++)   //Wyœwietlamy kolejne piksele tworz¹ce znak
		{
			LCD_RGB332 tmpalfa={.color=*znak++};
			uint8_t alfa, alfaneg;
			alfa=tmpalfa.red << 5; if(alfa) alfa|=0b00011111; alfaneg=~alfa;
			uint8_t red=(alfa*rgbcolor.red + alfaneg*rgbbkg.red) >> 8;     //Blending kolorów
			alfa=tmpalfa.green << 5; if(alfa) alfa|=0b00011111; alfaneg=~alfa;
			uint8_t green=(alfa*rgbcolor.green + alfaneg*rgbbkg.green) >> 8;
			alfa=tmpalfa.blue << 6; if(alfa) alfa|=0b00111111; alfaneg=~alfa;
			uint8_t blue=(alfa*rgbcolor.blue + alfaneg*rgbbkg.blue) >> 8;
			
			LCD_RGB565 col=(LCD_RGB565){.red=red >> 3, .green=green >> 2, .blue=blue >> 3};
			i8080_Write_W(col.word);
		}
	}
	
	uint8_t rows=(uint8_t)(uint16_t)font[1]; //Pobierz wysokoœæ fontu, dziwna konwersja wynika z zapisu w tablicy sta³ej jako wskaŸnika

	void (*RenderFuncPtr)(uint16_t pikselsno, uint8_t rows, const uint8_t *znak, uint16_t color, uint16_t bkgcolor); //WskaŸnik na funkcjê renderuj¹c¹

	switch((uint8_t)(uint16_t)font[0])
	{
		case 0x01:       RenderFuncPtr=&CharRenderFunc1bitNoAA;  //Renderujemy czcionkê bez antyaliasingiem 1bpp
		RA_SendCmdWithData(RA_Memory_Write_Control_Register0, (RS8875_MWCR0_Reg){.NoWriteAutoIncr=false, .Direction=RA_MWDownTopLeftRight}.byte);
		break;
		case 0x82:       RenderFuncPtr=&CharRenderFunc2bitMono;  //Renderujemy czcionkê z antyaliasingiem 2bpp mono
		RA_SendCmdWithData(RA_Memory_Write_Control_Register0, (RS8875_MWCR0_Reg){.NoWriteAutoIncr=false, .Direction=RA_MWDownTopLeftRight}.byte);
		break;
		case 0x88:       RenderFuncPtr=&CharRenderFunc8bitMono;
		RA_SendCmdWithData(RA_Memory_Write_Control_Register0, (RS8875_MWCR0_Reg){.NoWriteAutoIncr=false, .Direction=RA_MWLeftRightTopDown}.byte);
		break;      //Renderujemy czcionkê z antyaliasingiem 8bpp mono
		case 0xA8:       RenderFuncPtr=&CharRenderFunc322Subpixel;
		RA_SendCmdWithData(RA_Memory_Write_Control_Register0, (RS8875_MWCR0_Reg){.NoWriteAutoIncr=false, .Direction=RA_MWLeftRightTopDown}.byte);
		break;   //Renderujemy czcionkê z antyaliasingiem 8bpp kolor 3-3-2
		default:         RenderFuncPtr=CharRenderFuncNone;
	};

	char ch;
	while((ch=*tekst++))  //Wyœwietl kolejne znaki a¿ do koñca tekstu (znaku NUL)
	{
		const uint8_t *znak=font[ch-29];         //Adres pocz¹tku opisu znaku
		uint8_t col=*znak++;                     //Szerokoœæ znaku w pikselach

		LCD_SetWindow(x, y, x + col -1 , y + rows -1); //Okno w którym zapiszemy znak
		LCD_SetPosition(x, y + rows - 1);              //Pocz¹tek okna
		LCD_SendCmd(RA_Memory_Read_Write_Command);     // Zapis pod wskazan¹ pozycjê
		
		RenderFuncPtr(col*rows, rows, znak, color, bkgcolor);  //Wyœwietl znak stosuj¹c wybran¹ technikê renderowania
		
		x+=col;
		if(x >= LCD_GetMaxX()) break;   //Jesteœmy poza obszarem kreœlenia
	}

	LCD_CS(1);
	RA_SendCmdWithData(RA_Memory_Write_Control_Register0, (RS8875_MWCR0_Reg){.NoWriteAutoIncr=false, .Direction=RA_MWLeftRightTopDown}.byte);
}

void LCD_LineTo(uint16_t fromx, uint16_t fromy, uint16_t tox, uint16_t toy)
{
	RA_WaitForWAIT();	//Zaczekaj a¿ kontroler bêdzie wolny
	RA_SendCmdWithDataW(RA_Draw_Line_Square_Horizontal_Start_Address_Register0, fromx); //Pozycja startowa
	RA_SendCmdWithDataW(RA_Draw_Line_Square_Vertical_Start_Address_Register0, fromy);
	RA_SendCmdWithDataW(RA_Draw_Line_Square_Horizontal_End_Address_Register0, tox);	//Pozycja koñca linii
	RA_SendCmdWithDataW(RA_Draw_Line_Square_Vertical_End_Address_Register0, toy);
	RA_SendCmdWithData(RA_Draw_Line_Circle_Square_Control_Register, (RS8875_DCR_Reg){.Draw=DCR_DrawLine, .StartDrawing=DCR_StartLineSquareTriangle}.byte);
}

void LCD_DrawRect(uint16_t fromx, uint16_t fromy, uint16_t tox, uint16_t toy, _Bool filled)
{
	RA_WaitForWAIT();	//Zaczekaj a¿ kontroler bêdzie wolny
	RA_SendCmdWithDataW(RA_Draw_Line_Square_Horizontal_Start_Address_Register0, fromx); //Pozycja startowa
	RA_SendCmdWithDataW(RA_Draw_Line_Square_Vertical_Start_Address_Register0, fromy);
	RA_SendCmdWithDataW(RA_Draw_Line_Square_Horizontal_End_Address_Register0, tox);
	RA_SendCmdWithDataW(RA_Draw_Line_Square_Vertical_End_Address_Register0, toy);
	RA_SendCmdWithData(RA_Draw_Line_Circle_Square_Control_Register, (RS8875_DCR_Reg){.Draw=DCR_DrawSquare, .Fill=filled, .StartDrawing=DCR_StartLineSquareTriangle}.byte);
}

void LCD_Circle(uint16_t x, uint16_t y, uint8_t radius, _Bool filled)
{
	RA_WaitForWAIT();	//Zaczekaj a¿ kontroler bêdzie wolny
	RA_SendCmdWithDataW(RA_Draw_Circle_Center_Horizontal_Address_Register0, x); //Pozycja startowa
	RA_SendCmdWithDataW(RA_Draw_Circle_Center_Vertical_Address_Register0, y);
	RA_SendCmdWithData(RA_Draw_Circle_Radius_Register, radius);
	RA_SendCmdWithData(RA_Draw_Line_Circle_Square_Control_Register, (RS8875_DCR_Reg){.Draw=DCR_DrawSquare, .Fill=filled, .StartDrawing=DCR_StartCircle}.byte);
}

void LCD_DrawTriangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, _Bool filled)
{
	RA_WaitForWAIT();	//Zaczekaj a¿ kontroler bêdzie wolny
	RA_SendCmdWithDataW(RA_Draw_Line_Square_Horizontal_Start_Address_Register0, x1); //Pozycja startowa
	RA_SendCmdWithDataW(RA_Draw_Line_Square_Vertical_Start_Address_Register0, y1);
	RA_SendCmdWithDataW(RA_Draw_Line_Square_Horizontal_End_Address_Register0, x2);
	RA_SendCmdWithDataW(RA_Draw_Line_Square_Vertical_End_Address_Register0, y2);
	RA_SendCmdWithDataW(RA_Draw_Triangle_Point_2_Horizontal_Address_Register0, x3);
	RA_SendCmdWithDataW(RA_Draw_Triangle_Point_2_Vertical_Address_Register0, y3);
	RA_SendCmdWithData(RA_Draw_Line_Circle_Square_Control_Register, (RS8875_DCR_Reg){.Draw=DCR_DrawTriangle, .Fill=filled, .StartDrawing=DCR_StartLineSquareTriangle}.byte);
}

void LCD_DrawEllipse(uint16_t x, uint16_t y, uint16_t longaxis, uint16_t shortaxis, _Bool filled)
{
	RA_WaitForWAIT();	//Zaczekaj a¿ kontroler bêdzie wolny
	RA_SendCmdWithDataW(RA_Draw_Ellipse_Circle_Square_Center_Horizontal_Address_Register0, x);
	RA_SendCmdWithDataW(RA_Draw_Ellipse_Circle_Square_Center_Vertical_Address_Register0, y);
	RA_SendCmdWithDataW(RA_Draw_Ellipse_Circle_Square_Long_axis_Setting_Register_A0, longaxis);
	RA_SendCmdWithDataW(RA_Draw_Ellipse_Circle_Square_Short_axis_Setting_Register_B0, shortaxis);
	RA_SendCmdWithData(RA_Draw_Ellipse_Ellipse_Curve_Circle_Square_Control_Register, (RS8875_DCRC_Reg){.Draw=DCRC_DrawEllipse, .Fill=filled, .StartDrawing=1}.byte);
}

void LCD_DrawRoundedRect(uint16_t fromx, uint16_t fromy, uint16_t tox, uint16_t toy, uint16_t longaxis, uint16_t shortaxis, _Bool filled)
{
	RA_WaitForWAIT();	//Zaczekaj a¿ kontroler bêdzie wolny
	RA_SendCmdWithDataW(RA_Draw_Line_Square_Horizontal_Start_Address_Register0, fromx); //Pozycja startowa
	RA_SendCmdWithDataW(RA_Draw_Line_Square_Vertical_Start_Address_Register0, fromy);
	RA_SendCmdWithDataW(RA_Draw_Line_Square_Horizontal_End_Address_Register0, tox);
	RA_SendCmdWithDataW(RA_Draw_Line_Square_Vertical_End_Address_Register0, toy);
	RA_SendCmdWithDataW(RA_Draw_Ellipse_Circle_Square_Long_axis_Setting_Register_A0, longaxis);
	RA_SendCmdWithDataW(RA_Draw_Ellipse_Circle_Square_Short_axis_Setting_Register_B0, shortaxis);
	RA_SendCmdWithData(RA_Draw_Ellipse_Ellipse_Curve_Circle_Square_Control_Register, (RS8875_DCRC_Reg){.Draw=DCRC_DrawRoundedSquare, .Fill=filled, .StartDrawing=1}.byte);
}

void LCD_DrawCurve(uint16_t x, uint16_t y, uint16_t longaxis, uint16_t shortaxis, RA_DCRCDraw quadrant, _Bool filled)
{
	RA_WaitForWAIT();	//Zaczekaj a¿ kontroler bêdzie wolny
	RA_SendCmdWithDataW(RA_Draw_Ellipse_Circle_Square_Center_Horizontal_Address_Register0, x);
	RA_SendCmdWithDataW(RA_Draw_Ellipse_Circle_Square_Center_Vertical_Address_Register0, y);
	RA_SendCmdWithDataW(RA_Draw_Ellipse_Circle_Square_Long_axis_Setting_Register_A0, longaxis);
	RA_SendCmdWithDataW(RA_Draw_Ellipse_Circle_Square_Short_axis_Setting_Register_B0, shortaxis);
	RA_SendCmdWithData(RA_Draw_Ellipse_Ellipse_Curve_Circle_Square_Control_Register, (RS8875_DCRC_Reg){.Draw=quadrant, .Fill=filled, .StartDrawing=1}.byte);
}

void LCD_SetText(uint16_t x, uint16_t y, const char *tekst, RA_FNType FontType, _Bool CGRAM)
{
	RA_SendCmdWithData(RA_Memory_Write_Control_Register0, (RS8875_MWCR0_Reg){.TextModeEn=true, .MemCursorEn=true, .BlinkEn=true}.byte); //W³¹cz tryb tekstowy
	RA_SendCmdWithDataW(RA_Font_Write_Cursor_Horizontal_Position_Register0, x);	//Pozycja tekstu
	RA_SendCmdWithDataW(RA_Font_Write_Cursor_Vertical_Position_Register0, y);
	if(CGRAM) RA_SendCmdWithData(RA_Font_Control_Register0, (RS8875_FNCR0_Reg){.FNTSource=FNTCGRAM, .ExternalFont=true, .FontType=FontType}.byte);
	else RA_SendCmdWithData(RA_Font_Control_Register0, (RS8875_FNCR0_Reg){.FNTSource=FNTCGROM, .ExternalFont=false, .FontType=FontType}.byte);
	LCD_SendCmd(RA_Memory_Read_Write_Command);
	while(*tekst)
	{
		i8080_Write_B(*tekst++);
		RA_WaitForWAIT();
	}
	LCD_CS(1);
	RA_SendCmdWithData(RA_Memory_Write_Control_Register0, (RS8875_MWCR0_Reg){.TextModeEn=false, .MemCursorEn=true, .BlinkEn=true}.byte); //Wróæ do trybu graficznego
}

void RA_SetCGRAMChar(uint8_t charno, uint8_t desc[16])
{
	RA_SendCmdWithData(RA_Memory_Write_Control_Register1, (RS8875_MWCR1_Reg){.Memory=RA_DestWriteCGRAM}.byte);	//Zapis do CGRAM
	RA_SendCmdWithData(RA_CGRAM_Select_Register, charno);
	LCD_SendCmd(RA_Memory_Read_Write_Command);
	for(uint8_t i=0; i<16; i++) i8080_Write_B(desc[i]); //Zapisz now¹ matrycê
	RA_SendCmdWithData(RA_Memory_Write_Control_Register1, (RS8875_MWCR1_Reg){.Memory=RA_DestWriteLayer12}.byte); //Przywróæ zapis do GRAM
}

uint16_t LCD_StoreBMPMonoInGRAM(uint16_t x, uint16_t y, const uint8_t *data, uint8_t Layer)
{
	uint16_t width=*(const uint16_t *)data++; data++;
	uint16_t height=*(const uint16_t *)data++; data++;
	
	RA_WaitForBTEReady();			 //Jeœli aktualnie toczy siê jakaœ operacja to zaczekaj na jej zakoñczenie
	RA_BTE_SetDst(x, y, Layer);		 //Zapisujemy liniowy obszar na pocz¹tku nieu¿ywanej warstwy nr 1

	uint8_t owidth=(width+7)/8;  //Oblicz liczbe bajtów w linii z zaokr¹gleniem w górê

	RA_BTE_SetWidthHeight(owidth + 1, height);	 //Liczba bajtów zajêta przez bitmapê
	RA_SendCmdWithData(RA_BTE_Function_Control_Register1, (RS8875_BECR1_Reg){.ROPOperation=RA_BTEOp_Write, .OpType=12}.byte);		//Zapis z MCU do liniowego bloku
	RA_SendCmdWithData(RA_BTE_Function_Control_Register0, (RS8875_BECR0_Reg){.DstDataType=RA_BTELinear, .SrcDataType=RA_BTELinear, .Enable=true}.byte); //Rozpocznij operacjê
	LCD_SendCmd(RA_Memory_Read_Write_Command);    // Zapis pod wskazan¹ pozycjê

	for(uint16_t oy=0; oy<height; oy++)
	{
		for(uint8_t ox=0; ox<owidth; ox++)
		{
			i8080_Write_B(*data++);
			RA_WaitForWAIT();			//Zaczekaj na wykonanie polecenia
		}

		i8080_Write_B(0);			//Ka¿da linia koñczy siê dodatkowym bajtem, który jest ignorowany przez BTE
		RA_WaitForWAIT();			//Zaczekaj na wykonanie polecenia
	}
	
	return (owidth+1)*height;       //Zwróæ liczbê wykorzystanych bajtów
}

void LCD_MoveBMPMonoFromGRAM(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t fromx, uint16_t fromy, uint16_t color, uint16_t bkgcolor, uint8_t FromLayer, uint8_t ToLayer, _Bool transparency)
{
	RA_WaitForBTEReady();		    //Jeœli aktualnie toczy siê jakaœ operacja to zaczekaj na jej zakoñczenie
	RA_BTE_SetBkgColor(bkgcolor);	//Ustaw kolor t³a bitmapy - niepotrzebne, jeœli korzystamy z przezroczystoœci
	RA_BTE_SetColor(color);			//Ustaw kolor pikseli
	
	RA_BTE_SetSrc(fromx, fromy, FromLayer);
	RA_BTE_SetDst(x, y, ToLayer);
	RA_BTE_SetWidthHeight(width, height);
	
	RA_BTEROPOperationCode func=RA_BTEOp_MoveWithColorExpansion;
	if(transparency) func=RA_BTEOp_MoveWithColorExpansionAndTransparency; //Wyœwietlamy z przezroczystoœci¹
	RA_SendCmdWithData(RA_BTE_Function_Control_Register1, (RS8875_BECR1_Reg){.ROPOperation=func, .OpType=7}.byte);
	RA_SendCmdWithData(RA_BTE_Function_Control_Register0, (RS8875_BECR0_Reg){.DstDataType=RA_BTERectangular, .SrcDataType=RA_BTELinear, .Enable=true}.byte); //Rozpocznij operacjê
}

uint16_t LCD_StoreBMPMonoInGRAM16b(uint16_t x, uint16_t y, const uint8_t *data, uint8_t Layer)
{
	uint16_t width=*(const uint16_t *)data++; data++;
	uint16_t height=*(const uint16_t *)data++; data++;
	
	RA_WaitForBTEReady();			 //Jeœli aktualnie toczy siê jakaœ operacja to zaczekaj na jej zakoñczenie
	RA_BTE_SetDst(x, y, Layer);		 //Zapisujemy liniowy obszar na pocz¹tku nieu¿ywanej warstwy nr 1

	uint8_t owidth=(width+7)/16;  //Oblicz liczbe bajtów w linii z zaokr¹gleniem w górê
	_Bool addbyte=((width+7) % 16) > 7;

	RA_BTE_SetWidthHeight(owidth + addbyte + 1, height);	 //Liczba bajtów zajêta przez bitmapê
	RA_SendCmdWithData(RA_BTE_Function_Control_Register1, (RS8875_BECR1_Reg){.ROPOperation=RA_BTEOp_Write, .OpType=12}.byte);		//Zapis z MCU do liniowego bloku
	RA_SendCmdWithData(RA_BTE_Function_Control_Register0, (RS8875_BECR0_Reg){.DstDataType=RA_BTELinear, .SrcDataType=RA_BTELinear, .Enable=true}.byte); //Rozpocznij operacjê
	LCD_SendCmd(RA_Memory_Read_Write_Command);    // Zapis pod wskazan¹ pozycjê

	for(uint16_t oy=0; oy<height; oy++)
	{
		for(uint8_t ox=0; ox<owidth; ox++)
		{
			uint16_t w=(*data++) << 8;
			w|=*data++;
			i8080_Write_W(w);
			RA_WaitForWAIT();
		}
		
		if(addbyte) i8080_Write_W((*data++) << 8);	//Wyrównanie bitmapy. Jeœli bitmapa ma linie wyrównane do s³owa to mo¿na to pomin¹æ
		
		i8080_Write_W(0xffff);	//Wyrównanie linii
		RA_WaitForWAIT();
	}
	
	return (owidth+addbyte+1)*height;       //Zwróæ liczbê wykorzystanych bajtów
}

void LCD_MoveBMPMonoFromGRAM16b(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t fromx, uint16_t fromy, uint16_t color, uint16_t bkgcolor, uint8_t FromLayer, uint8_t ToLayer, _Bool transparency)
{
	RA_WaitForBTEReady();		    //Jeœli aktualnie toczy siê jakaœ operacja to zaczekaj na jej zakoñczenie
	RA_BTE_SetBkgColor(bkgcolor);	//Ustaw kolor t³a bitmapy - niepotrzebne, jeœli korzystamy z przezroczystoœci
	RA_BTE_SetColor(color);			//Ustaw kolor pikseli
	
	RA_BTE_SetSrc(fromx, fromy, FromLayer);
	RA_BTE_SetDst(x, y, ToLayer);
	RA_BTE_SetWidthHeight(width, height);
	
	RA_BTEROPOperationCode func=RA_BTEOp_MoveWithColorExpansion;
	if(transparency) func=RA_BTEOp_MoveWithColorExpansionAndTransparency; //Wyœwietlamy z przezroczystoœci¹
	RA_SendCmdWithData(RA_BTE_Function_Control_Register1, (RS8875_BECR1_Reg){.ROPOperation=func, .OpType=15}.byte);
	RA_SendCmdWithData(RA_BTE_Function_Control_Register0, (RS8875_BECR0_Reg){.DstDataType=RA_BTERectangular, .SrcDataType=RA_BTELinear, .Enable=true}.byte); //Rozpocznij operacjê
}

uint16_t LCD_StoreBMP565InGRAM(uint16_t x, uint16_t y, const uint16_t *data, uint8_t Layer)
{
	uint16_t width=*data++;
	uint16_t height=*data++;
	
	RA_WaitForBTEReady();			 //Jeœli aktualnie toczy siê jakaœ operacja to zaczekaj na jej zakoñczenie
	RA_BTE_SetDst(x, y, Layer);		 //Zapisujemy liniowy obszar na pocz¹tku nieu¿ywanej warstwy nr 1

	RA_BTE_SetWidthHeight(width, height);	 //Liczba bajtów zajêta przez bitmapê
	RA_SendCmdWithData(RA_BTE_Function_Control_Register1, (RS8875_BECR1_Reg){.ROPOperation=RA_BTEOp_Write, .OpType=12}.byte);		//Zapis z MCU do liniowego bloku
	RA_SendCmdWithData(RA_BTE_Function_Control_Register0, (RS8875_BECR0_Reg){.DstDataType=RA_BTELinear, .SrcDataType=RA_BTELinear, .Enable=true}.byte); //Rozpocznij operacjê
	LCD_SendCmd(RA_Memory_Read_Write_Command);    // Zapis pod wskazan¹ pozycjê

	for(uint16_t oy=0; oy<height; oy++)
	{
		for(uint8_t ox=0; ox<width; ox++)
		{
			i8080_Write_W(*data++);
			RA_WaitForWAIT();			//Zaczekaj na wykonanie polecenia
		}
	}
	return width*height;       //Zwróæ liczbê wykorzystanych bajtów
}

void LCD_MoveBMP565FromGRAM(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t fromx, uint16_t fromy, uint8_t FromLayer, uint8_t ToLayer, _Bool transparency, uint16_t transcolor)
{
	RA_WaitForBTEReady();		    //Jeœli aktualnie toczy siê jakaœ operacja to zaczekaj na jej zakoñczenie
	
	RA_BTE_SetSrc(fromx, fromy, FromLayer);
	RA_BTE_SetDst(x, y, ToLayer);
	RA_BTE_SetWidthHeight(width, height);
	
	RA_BTEROPOperationCode func=RA_BTEOp_MovePositive;
	if(transparency)
	{
		func=RA_BTEOp_TransparentMovePositive; //Wyœwietlamy z przezroczystoœci¹
		RA_BTE_SetBkgColor(transcolor);		   //Kolor przexroczysty
	}
	RA_SendCmdWithData(RA_BTE_Function_Control_Register1, (RS8875_BECR1_Reg){.ROPOperation=func, .OpType=12}.byte);
	RA_SendCmdWithData(RA_BTE_Function_Control_Register0, (RS8875_BECR0_Reg){.DstDataType=RA_BTERectangular, .SrcDataType=RA_BTELinear, .Enable=true}.byte); //Rozpocznij operacjê
}

void LCD_DrawBitmap_Mono8bpp(uint16_t x, uint16_t y, uint16_t color, uint16_t bkgcolor, const uint8_t *data, _Bool transparency, uint8_t Layer)
{
	uint16_t width=*(const uint16_t *)data++; data++;
	uint16_t height=*(const uint16_t *)data++; data++;
	
	RA_WaitForBTEReady();			//Jeœli aktualnie toczy siê jakaœ operacja to zaczekaj na jej zakoñczenie
	RA_BTE_SetBkgColor(bkgcolor);	//Ustaw kolor t³a bitmapy - niepotrzebne, jeœli korzystamy z przezroczystoœci
	RA_BTE_SetColor(color);			//Ustaw kolor pikseli
	
	RA_BTE_SetDst(x, y, 0);
	RA_BTE_SetWidthHeight(width, height);
	
	RA_BTEROPOperationCode func=RA_BTEOp_ColorExpansion;
	if(transparency) func=RA_BTEOp_ColorExpansionWithTransparency; //Wyœwietlamy z przezroczystoœci¹

	RA_SendCmdWithData(RA_BTE_Function_Control_Register1, (RS8875_BECR1_Reg){.ROPOperation=func, .OpType=7}.byte);
	RA_SendCmdWithData(RA_BTE_Function_Control_Register0, (RS8875_BECR0_Reg){.DstDataType=RA_BTERectangular, .Enable=true}.byte); //Rozpocznij operacjê

	LCD_SendCmd(RA_Memory_Read_Write_Command);    // Zapis pod wskazan¹ pozycjê
	
	uint8_t owidth=(width+7)/8;  //Oblicz liczbe bajtów w linii z zaokr¹gleniem w górê
	uint8_t app=0;
	if(owidth & 1) {app=1; owidth&=0xfe;}
	
	for(uint16_t oy=0; oy<height; oy++)
	{
		for(uint8_t ox=0; ox<owidth; ox+=2)
		{
			uint16_t wrd=((uint16_t)(*data++)) << 8;
			wrd|=((uint16_t)*data++);
			i8080_Write_W(wrd);
			RA_WaitForWAIT();			//Zaczekaj na wykonanie polecenia
		}
		if(app)
		{
			uint16_t wrd=((uint16_t)(*data++)) << 8;
			i8080_Write_W(wrd);
			RA_WaitForWAIT();			//Zaczekaj na wykonanie polecenia
		}
	}
	LCD_CS(1);
}