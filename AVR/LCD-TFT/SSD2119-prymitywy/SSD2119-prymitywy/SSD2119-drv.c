/*
 * SSD2119_drv.c
 *
 * Created: 2014-03-07 17:08:33
 *  Author: tmf
 */

#include <stdbool.h>
#include "ssd2119.h"
#include "SPI/spi.h"
#include "GFXDrv.h"

typedef struct
{
	uint8_t reg;        //Numer rejestru uk³¹du SSD
	uint16_t value;     //Wartoœæ rejestru
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
	_Bool FullScreen       : 1;           //true jeœli zosta³ wybrany do zapisu ca³y ekran
	_Bool Mode565          : 1;           //true jeœli mamy tryb przesy³u 2 bajty na piksel, false jeœli 3 bajty na piksel (tryb 18-bitowy)
	
} LCD_Driver_Status;            //Stan sterownika LCD


void ssd2119_SendCmdWithData(uint8_t cmd, uint16_t data)
{
	ssd2119_SendCmd(cmd);
	ssd2119_SendDataWord(data);
}

void LCD_Init262()
{
	ssd2119_RESET();   //Zresetuj kontroler

	for(uint8_t indeks=0; indeks < sizeof(SSD_Init262)/sizeof(SSD_Init262[0]); indeks++)
	{
		ssd2119_SendCmd(SSD_Init262[indeks].reg);
		ssd2119_SendDataWord(SSD_Init262[indeks].value);
	}
	
	LCD_Driver_Status.Mode565=false;    //Tryb przesy³u 3 bajty/piksel
}

void LCD_Init65()
{
	ssd2119_RESET();   //Zresetuj kontroler

	for(uint8_t indeks=0; indeks < sizeof(SSD_Init65)/sizeof(SSD_Init65[0]); indeks++)
	{
		ssd2119_SendCmd(SSD_Init65[indeks].reg);
		ssd2119_SendDataWord(SSD_Init65[indeks].value);
	}
	
	LCD_Driver_Status.Mode565=true;    //Tryb przesy³u 3 bajty/piksel
}

void SED2119_RGB565()   //Wybierz przesy³ 16-bitowych danych (2 bajty/piksel)
{
	ssd2119_SendCmd(ssd2119_Entry_Mode);  //Wybierz rejestr Entry Mode
	ssd2119_SendDataWord((ssd2119_EntryMode_Reg){.DFM=0b11, .DenMode=1, .TY=0b01, .ID=0b11, .AM=0}.word);
}

void SED2119_RGB666()  //Wybierz przesy³ 18-bitowych danych (3 bajty/piksel)
{
	ssd2119_SendCmd(ssd2119_Entry_Mode);  //Wybierz rejestr Entry Mode
	ssd2119_SendDataWord((ssd2119_EntryMode_Reg){.DFM=0b10, .DenMode=1, .TY=0b01, .ID=0b11, .AM=0}.word);
}

//Ustaw pozycjê (x,y) w GRAM
void LCD_SetPosition(uint16_t x1, uint8_t y1)
{
	ssd2119_SendCmdWithData(ssd2119_Set_GDDRAM_X_address_counter, x1);    // Pozycja X piksela
	ssd2119_SendCmdWithData(ssd2119_Set_GDDRAM_Y_address_counter, y1);    // Pozycja Y piksela
}

//Ustaw okno w pamiêci GRAM
void LCD_SetWindow(uint16_t x1, uint8_t y1, uint16_t x2, uint8_t y2)
{
	ssd2119_SendCmdWithData(ssd2119_Vertical_RAM_address_position, ((uint16_t)y1) | (((uint16_t)y2) << 8)); //Górny i dolny brzeg obszaru
	ssd2119_SendCmdWithData(ssd2119_Horizontal_RAM_address_start_position, x1);    //Lewy brzeg obszaru
	ssd2119_SendCmdWithData(ssd2119_Horizontal_RAM_address_end_position, x2);    //Prawy brzeg obszaru
}

void LCD_Rect(uint16_t x1, uint8_t y1, uint16_t x2, uint8_t y2, __uint24 color)
{
	LCD_SetWindow(x1, y1, x2, y2);

	ssd2119_SendCmdWithData(ssd2119_Set_GDDRAM_X_address_counter, x1);    // Pozycja X piksela
	ssd2119_SendCmdWithData(ssd2119_Set_GDDRAM_Y_address_counter, y1);    // Pozycja Y piksela

	ssd2119_SendCmd(ssd2119_Write_to_GRAM);    // Zapis pod wskazan¹ pozycjê

	ssd2119_CS(0);            //Aktywuj kontroler - DMA przeœle dane do GRAM i automatycznie zdezaktywuje sygna³ CS
	__uint24 val=(1+y2-y1)*(__uint24)(1+x2-x1);  //Policz z ilu pikseli sk³ada siê prostok¹t
	for(;val;val--)
	{
		ssd2119_SendDataByte(color >> 16);
		ssd2119_SendDataByte(color >> 8);
		ssd2119_SendDataByte(color & 0xff);
	}
	ssd2119_CS(1);
}

void LCD_SetPixel(uint16_t x1, uint8_t y1, __uint24 color)
{
	ssd2119_SendCmdWithData(ssd2119_Set_GDDRAM_X_address_counter, x1);    // Pozycja X piksela
	ssd2119_SendCmdWithData(ssd2119_Set_GDDRAM_Y_address_counter, y1);    // Pozycja Y piksela

	ssd2119_SendCmd(ssd2119_Write_to_GRAM);    // Zapis pod wskazan¹ pozycjê
	ssd2119_CS(0);
	ssd2119_SendDataByte(color >> 16);
	ssd2119_SendDataByte(color >> 8);
	SPI_RW_Byte(color & 0xff);                 //Czekamy na wys³anie wszystkich danych
	ssd2119_CS(1);                             //i dezaktywujemy CS
}

void LCD_LineTo(uint16_t x, uint8_t y, uint16_t x2, uint8_t y2, __uint24 color)
{
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

void LCD_SetText(uint16_t x, uint8_t y, const __memx char *tekst, const uint8_t __flash * const __flash font[], __uint24 color, __uint24 bkgcolor, _Bool transparent)
{
	uint8_t rows=(uint8_t)(uint16_t)font[0]; //Pobierz wysokoœæ fontu
	char ch;
	y+=rows-1;

	while((ch=*tekst++))  //Wyœwietl kolejne znaki a¿ do koñca tekstu (znaku NUL)
	{
		const uint8_t __flash *znak=font[ch-30]; //Adres pocz¹tku opisu znaku
		uint8_t col=*znak++;                     //Szerokoœæ znaku w pikselach
		uint8_t coldesc=coldesc;                 //Bajt opisu znaku
		uint8_t pixelcnt=0;                      //Licznik pikseli w znaku

		for(uint8_t ox=0; ox < col; ox++)        //Wyœwietlamy kolejne kolumny tworz¹ce znak
		{
			for(uint8_t oy=0; oy < rows; oy++)   //Narysuj jedn¹ kolumnê znaku
			{
				if((pixelcnt & 0b111) == 0) coldesc=*znak++;     //Pobierz bajt opisu znaku
				pixelcnt++;
				if(coldesc & 0x80) LCD_SetPixel(x, y-oy, color);
				else if(!transparent) LCD_SetPixel(x, y-oy, bkgcolor); //Narysuj t³o tylko jeœli nie wyœwietlamy przeŸroczystego tekstu
				coldesc<<=1;
			}
			x++;  //Rysujemy kolejn¹ kolumnê
			if(x >= LCD_GetMaxX()) return;   //Jesteœmy poza obszarem kreœlenia
		}
	}
}

void LCD_DrawBitmap_Mono(uint16_t x, uint8_t y, uint16_t color, uint16_t bkgcolor,const uint8_t __flash *data)
{
	if(LCD_Driver_Status.Mode565 == false) SED2119_RGB565();     //Prze³¹cz siê na tryb przesy³ania 2 bajty na piksel
	
	uint16_t width=*(const uint16_t __flash *)data++; data++;
	uint16_t height=*(const uint16_t __flash *)data++; data++;
	
	LCD_SetWindow(x, y, x + width -1, y +  height - 1);
	LCD_SetPosition(x, y);
	ssd2119_SendCmd(ssd2119_Write_to_GRAM);    // Zapis pod wskazan¹ pozycjê
	ssd2119_CS(0);
	height/=8;
	
	uint16_t tmpcolor;

	for(uint16_t ox=0; ox < width*height; ox++)
	{
		uint8_t bajt=*data++;
		for(uint8_t b=0; b<8; b++)
		{
			if(bajt & 0x80) tmpcolor=color; else tmpcolor=bkgcolor;
			ssd2119_SendDataWord(tmpcolor);
			bajt<<=1;
		}
	}
	ssd2119_CS(1);
	if(LCD_Driver_Status.Mode565 == false) SED2119_RGB666();  //Wróæ do trybu 3 bajty/piksel
}

void LCD_DrawBitmap_565(uint16_t x, uint8_t y, const uint16_t __flash *data)
{
	if(LCD_Driver_Status.Mode565 == false) SED2119_RGB565();     //Prze³¹cz siê na tryb przesy³ania 2 bajty na piksel
	
	uint16_t width=*data++;
	uint8_t height=*data++;
	
	LCD_SetWindow(x, y, x + width -1, y +  height - 1);
	LCD_SetPosition(x, y);
	ssd2119_SendCmd(ssd2119_Write_to_GRAM);    // Zapis pod wskazan¹ pozycjê
	ssd2119_CS(0);

	for(uint16_t ox=0; ox < width*height; ox++)
	{
		ssd2119_SendDataWord(*data++);
	}
	ssd2119_CS(1);
	if(LCD_Driver_Status.Mode565 == false) SED2119_RGB666();  //Wróæ do trybu 3 bajty/piksel
}
