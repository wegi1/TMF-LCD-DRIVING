/*
 * ssd2119_DMA.c
 *
 * Created: 2014-11-09 16:11:46
 *  Author: tmf
 */ 

#include "sam.h"
#include <stdbool.h>
#include "Clk/SetClk.h"
#include "Delay/delay.h"
#include "SPI/spi.h"
#include "ssd2119.h"
#include "icons.h"

typedef struct
{
	uint8_t reg;        //Numer rejestru uk³¹du SSD
	uint16_t value;     //Wartoœæ rejestru
} SSD_Regs;

const SSD_Regs SSD_Init262[] = {{0x28, 0x0006}, {0x00, 0x0001}, {0x10, 0x0000}, {0x01, 0x72EF}, {0x02, 0x0600}, {0x03, 0x6A38}, {0x11, 0x4870},
{0x0F, 0x0000}, {0x0B, 0x5308}, {0x0C, 0x0003}, {0x0D, 0x000A}, {0x0E, 0x2E00}, {0x1E, 0x00BE}, {0x25, 0x8000},
{0x26, 0x7800}, {0x4E, 0x0000}, {0x4F, 0x0000}, {0x12, 0x08D9}, {0x30, 0x0000}, {0x31, 0x0104}, {0x32, 0x0100},
{0x33, 0x0305}, {0x34, 0x0505}, {0x35, 0x0305}, {0x36, 0x0707}, {0x37, 0x0300}, {0x3A, 0x1200}, {0x3B, 0x0800},
{0x07, 0x0033}};

const SSD_Regs SSD_Init65[] = {{0x28, 0x0006}, {0x00, 0x0001}, {0x10, 0x0000}, {0x01, 0x72EF}, {0x02, 0x0600}, {0x03, 0x6A38}, {0x11, 0x6870},
{0x0F, 0x0000}, {0x0B, 0x5308}, {0x0C, 0x0003}, {0x0D, 0x000A}, {0x0E, 0x2E00}, {0x1E, 0x00BE}, {0x25, 0x8000},
{0x26, 0x7800}, {0x4E, 0x0000}, {0x4F, 0x0000}, {0x12, 0x08D9}, {0x30, 0x0000}, {0x31, 0x0104}, {0x32, 0x0100},
{0x33, 0x0305}, {0x34, 0x0505}, {0x35, 0x0305}, {0x36, 0x0707}, {0x37, 0x0300}, {0x3A, 0x1200}, {0x3B, 0x0800},
{0x07, 0x0033}};

void LCD_Init262()
{
	ssd2119_RESET();   //Zresetuj kontroler

	for(uint8_t indeks=0; indeks < sizeof(SSD_Init262)/sizeof(SSD_Init262[0]); indeks++)
	{
		ssd2119_SendCmd(SSD_Init262[indeks].reg);
		ssd2119_SendDataWord(SSD_Init262[indeks].value);
	}
}

void LCD_Init65()
{
	ssd2119_RESET();   //Zresetuj kontroler

	for(uint8_t indeks=0; indeks < sizeof(SSD_Init65)/sizeof(SSD_Init65[0]); indeks++)
	{
		ssd2119_SendCmd(SSD_Init65[indeks].reg);
		ssd2119_SendDataWord(SSD_Init65[indeks].value);
	}
}

//Ustawia okno w którym mo¿na zapisywaæ dane
void SSD2119_SetWindow(uint16_t x1, uint8_t y1, uint16_t x2, uint8_t y2)
{
	ssd2119_SendCmd(0x44);                //Górny i dolny brzeg obszaru
	ssd2119_SendDataWord(((uint16_t)y1) | (((uint16_t)y2) << 8));
	ssd2119_SendCmd(0x45);                //Lewy brzeg obszaru
	ssd2119_SendDataWord(x1);
	ssd2119_SendCmd(0x46);                //Prawy brzeg obszaru
	ssd2119_SendDataWord(x2);
}

//Wybiera piksel pocz¹wszy od którego bêdzie mo¿na zapisywaæ dane. Pozycja musi le¿eæ w obrêbie wybranego okna
void SSD2119_SetPosition(uint16_t x1, uint8_t y1)
{
	ssd2119_SendCmd(0x4E);    // Pozycja X piksela
	ssd2119_SendDataWord(x1);
	ssd2119_SendCmd(0x4F);    // Pozycja Y piksela
	ssd2119_SendDataWord(y1);
}

void SSD2119_Rect(uint16_t x1, uint8_t y1, uint16_t x2, uint8_t y2, uint32_t color)
{
	SSD2119_SetWindow(x1, y1, x2, y2);
	SSD2119_SetPosition(x1, y1);

	ssd2119_SendCmd(0x22);    // Zapis pod wskazan¹ pozycjê
	ssd2119_CS(0);
	for(uint8_t y=y1; y<=y2; y++)
	for(uint16_t x=x1; x<=x2; x++)
	{
		ssd2119_SendDataByte(color & 0xff);  //Blue
		ssd2119_SendDataByte((color >> 8) & 0xff);  //Green
		ssd2119_SendDataByte((color >> 16) & 0xff);  //Red
	}
	ssd2119_CS(1);
}

void SSD2119_Rect565(uint16_t x1, uint8_t y1, uint16_t x2, uint8_t y2, uint16_t color)
{
	SSD2119_SetWindow(x1, y1, x2, y2);
	SSD2119_SetPosition(x1, y1);

	ssd2119_SendCmd(0x22);    // Zapis pod wskazan¹ pozycjê
	ssd2119_CS(0);
	for(uint8_t y=y1; y<=y2; y++)
	for(uint16_t x=x1; x<=x2; x++)
	{
		ssd2119_SendDataByte(color & 0xff);
		ssd2119_SendDataByte((color >> 8) & 0xff);
	}
	ssd2119_CS(1);
}

void SED2119_RGB565()
{
	ssd2119_SendCmd(ssd2119_Entry_Mode);  //Wybierz rejestr Entry Mode
	ssd2119_SendDataWord((ssd2119_EntryMode_Reg){.DFM=0b11, .DenMode=1, .TY=0b11, .ID=0b11, .AM=0}.word);
}

void SED2119_RGB262()
{
	ssd2119_SendCmd(ssd2119_Entry_Mode);  //Wybierz rejestr Entry Mode
	ssd2119_SendDataWord((ssd2119_EntryMode_Reg){.DFM=0b10, .DenMode=1, .TY=0b01, .ID=0b11, .AM=0}.word);
}

void SED2119_BGR()
{
	ssd2119_SendCmd(ssd2119_Driver_Output_Control);  //Wybierz rejestr Entry Mode
	ssd2119_SendDataWord((ssd2119_DriverOutputCtrl_Reg){.RL=1, .REV=1, .GD=1, .BGR=1, .TB=1, .MUX=239}.word);
}

void SED2119_RGB()
{
	ssd2119_SendCmd(ssd2119_Driver_Output_Control);  //Wybierz rejestr Entry Mode
	ssd2119_SendDataWord((ssd2119_DriverOutputCtrl_Reg){.RL=1, .REV=1, .GD=1, .TB=1, .MUX=239}.word);
}

void SSD2119_DrawBar565(uint16_t x1, uint8_t y1)
{
	void DrawLine(uint8_t hlen, uint16_t color)
	{
		for(uint8_t x=0; x<hlen; x++)
		{
			ssd2119_SendDataByte(color >> 8);
			ssd2119_SendDataByte(color & 0xff);
		}
	}

	SSD2119_SetWindow(x1, y1, x1+71, y1+127);
	SSD2119_SetPosition(x1, y1);
	ssd2119_SendCmd(0x22);    // Zapis pod wskazan¹ pozycjê

	ssd2119_RGB565 r={{0,0,0}};
	ssd2119_RGB565 g={{0,0,0}};
	ssd2119_RGB565 b={{0,0,0}};

	ssd2119_CS(0);
	for(uint8_t y=y1; y<=y1+127; y++)
	{
		DrawLine(20, r.word); DrawLine(6, (ssd2119_RGB565){{0, 0, 0}}.word);
		DrawLine(20, g.word); DrawLine(6, (ssd2119_RGB565){{0, 0, 0}}.word);
		DrawLine(20, b.word);
		if((y % 4) == 3)
		{
			r.red++;
			b.blue++;
		}
		if (y & 0x01) g.green++;
	}
	ssd2119_CS(1);
}

void SSD2119_DrawBar262(uint16_t x1, uint8_t y1)
{
	void DrawLine(uint8_t hlen, uint32_t color)
	{
		for(uint8_t x=0; x<hlen; x++)
		{
			ssd2119_SendDataByte(color >> 16);
			ssd2119_SendDataByte(color >> 8);
			ssd2119_SendDataByte(color & 0xff);
		}
	}

	SSD2119_SetWindow(x1, y1, x1+71, y1+127);
	SSD2119_SetPosition(x1, y1);
	ssd2119_SendCmd(0x22);    // Zapis pod wskazan¹ pozycjê

	uint32_t r=0;
	uint32_t g=0;
	uint32_t b=0;

	ssd2119_CS(0);
	for(uint8_t y=y1; y<=y1+127; y++)
	{
		DrawLine(20, r); DrawLine(6, 0);
		DrawLine(20, g); DrawLine(6, 0);
		DrawLine(20, b);
		if (y & 0x01)
		{
			g+=0x000400;
			r+=0x040000;
			b+=0x000004;
		}
	}
	ssd2119_CS(1);
}

void ssd2119_DrawBitmapLine_Mono(uint16_t x, uint8_t y, uint16_t width,  uint8_t line, const uint8_t *data)
{
	SSD2119_SetWindow(x, y, x + width -1, y);
	SSD2119_SetPosition(x, y);
	ssd2119_SendCmd(0x22);    // Zapis pod wskazan¹ pozycjê
	ssd2119_CS(0);
	width/=8;
	uint16_t ofs=width * line;
	data+=ofs;
	for(uint16_t ox=0; ox < width; ox++)
	{
		uint8_t kolor;
		uint8_t bajt=*data++;
		for(uint8_t b=0; b<8; b++)
		{
			if(bajt & 0x80) kolor=0xff; else kolor=0;
			ssd2119_SendDataByte(kolor);
			ssd2119_SendDataByte(kolor);
			bajt<<=1;
		}
	}
	ssd2119_CS(1);
}

void ssd2119_DrawBitmap_Mono(uint16_t x, uint8_t y, uint16_t width, uint8_t height, const uint8_t *data)
{
	SSD2119_SetWindow(x, y, x + width -1, y +  height - 1);
	SSD2119_SetPosition(x, y);
	ssd2119_SendCmd(0x22);    // Zapis pod wskazan¹ pozycjê
	ssd2119_CS(0);
	height/=8;

	for(uint16_t ox=0; ox < width*height; ox++)
	{
		uint8_t kolor;
		uint8_t bajt=*data++;
		for(uint8_t b=0; b<8; b++)
		{
			if(bajt & 0x80) kolor=0xff; else kolor=0;
			ssd2119_SendDataByte(kolor);
			ssd2119_SendDataByte(kolor);
			bajt<<=1;
		}
	}
	ssd2119_CS(1);
}

void ssd2119_DrawBitmapLine_565(uint16_t x, uint8_t y, uint16_t width, uint8_t line, const uint16_t *data)
{
	SSD2119_SetWindow(x, y, x + width -1, y);
	SSD2119_SetPosition(x, y);
	ssd2119_SendCmd(0x22);    // Zapis pod wskazan¹ pozycjê
	ssd2119_CS(0);

	uint16_t ofs=width * line;
	data+=ofs;

	for(uint16_t ox=0; ox < width; ox++)
	{
		ssd2119_SendDataWord(*data++);
	}
	ssd2119_CS(1);
}

void ssd2119_DrawBitmap_565(uint16_t x, uint8_t y, uint16_t width, uint8_t height, const uint16_t *data)
{
	SSD2119_SetWindow(x, y, x + width -1, y +  height - 1);
	SSD2119_SetPosition(x, y);
	ssd2119_SendCmd(0x22);    // Zapis pod wskazan¹ pozycjê
	ssd2119_CS(0);

	for(uint16_t ox=0; ox < width*height; ox++)
	{
		ssd2119_SendDataWord(*data++);
	}
	ssd2119_CS(1);
}

int main(void)
{
	Set48MHzClk();
	
	LCD_Interface_Init();
	LCD_Init262();
	
	SSD2119_Rect(0, 0, 319, 239, 0x000000ul); //Wyczyœæ ekran
	ssd2119_SendCmdWithData(ssd2119_Driver_Output_Control, (ssd2119_DriverOutputCtrl_Reg){.RL=1, .REV=1, .GD=1, .TB=1, .MUX=239}.word);
	ssd2119_SendCmdWithData(ssd2119_Display_Control,(ssd2119_DisplayCtrl_Reg){.D0=1, .D1=1, .DTE=1, .GON=1, .VLE=0b01}.word);

	SED2119_RGB565();  //W³¹cz tryb RGB 5-6-5

	ssd2119_DrawBitmap_Mono(0, 0, 320, 240, image_data_ikony);

	uint8_t y=0, dir=1;
	while(1)
	{
		ssd2119_SendCmdWithData(ssd2119_First_Screen_Driving_Position_Start, y); //Drugi ekran - pocz¹tek
		ssd2119_SendCmdWithData(ssd2119_First_Screen_Driving_Position_End, y+48);
		//		ssd2119_SendCmdWithData(ssd2119_Vertical_Scroll_Control_Screen1, 240-y);
		_delay_ms(30);
		y+=dir;
		if(y>=192) dir=-1;  //Zmieñ kierunek przesuwu na w górê
		if(y==0) dir=1;     //Zmieñ kierunek przesuwu na w dó³
	}

}
