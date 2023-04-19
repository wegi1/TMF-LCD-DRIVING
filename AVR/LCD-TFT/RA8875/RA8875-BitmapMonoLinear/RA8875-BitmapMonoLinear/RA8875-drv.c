/*
 * SSD2119_drv.c
 *
 * Created: 2014-03-07 17:08:33
 *  Author: tmf
 */

#include <stdbool.h>
#include "RA8875.h"
#include "i8080-xmega.h"
#include "GFXDrv.h"
#include <util/delay.h>
#include <stdlib.h>

void RA_SendCmdWithData(uint8_t cmd, uint8_t data)
{
	LCD_SendCmd(cmd);
	i8080_Write_W8(data);			//W RA8875 wszelkie rejestry s¹ tylko 8 bitowe niezale¿nie od wybanej szerokoœci magistrali
	LCD_CS(1);     //Deaktywuj kontroler
}

void RA_SendCmdWithDataW(uint8_t cmd, uint16_t data)
{
	LCD_SendCmd(cmd);
	i8080_Write_W8(data & 0xFF);		//Wyœlij mniej znacz¹c¹ po³owê argumentu
	LCD_SendCmd(cmd+1);				//Wyœlij bardziej znacz¹c¹ po³owê argumentu
	i8080_Write_W8(data >> 8);
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

#ifdef LCD_P320x240

const uint8_t __flash RA_Init[]={0x88, 0x0a, 0xFF, 0x89, 0x02, 0xFF, 0x10, 0x0f, 0xFF, 0x04, 0x03, 0x14, 0x27, 0x15, 0x00, 0x16, 0x05, 0x17, 0x04, 0x18, 0x03, 0x19, 0xef, 0x1a, 0x00, 0x1b, 0x05, 0x1c, 0x00,
0x1d, 0x0e, 0x1e, 0x00, 0x1f, 0x02};

#endif

#ifdef LCD_P480x272

const uint8_t __flash RA_Init[]={0x88, 0x0a, 0xFF, 0x89, 0x02, 0xFF, 0x10, 0x0f, 0xFF, 0x04, 0x82, 0x14, 0x3b, 0x15, 0x00, 0x16, 0x01, 0x17, 0x00, 0x18, 0x05, 0x19, 0x0f, 0x1a, 0x01, 0x1b, 0x02, 0x1c, 0x00,
0x1d, 0x07, 0x1e, 0x00, 0x1f, 0x09};

#endif

#ifdef LCD_P640x480

const uint8_t __flash RA_Init[]={0x88, 0x0b, 0xFF, 0x89, 0x02, 0xFF, 0x10, 0x0f, 0xFF, 0x04, 0x01, 0x14, 0x4f, 0x15, 0x05, 0x16, 0x0f, 0x17, 0x01, 0x18, 0x00, 0x19, 0xdf, 0x1a, 0x01, 0x1b, 0x0a, 0x1c, 0x00,
0x1d, 0x0e, 0x1e, 0x00, 0x1f, 0x01};

#endif

#ifdef LCD_P800x480

const uint8_t __flash RA_Init[]={0x88, 0x0b, 0xFF, 0x89, 0x02, 0xFF, 0x10, 0x0f, 0xFF, 0x04, 0x81, 0x14, 0x63, 0x15, 0x03, 0x16, 0x03, 0x17, 0x02, 0x18, 0x00, 0x19, 0xdf, 0x1a, 0x01, 0x1b, 0x14, 0x1c, 0x00,
0x1d, 0x06, 0x1e, 0x00, 0x1f, 0x01};

#endif

void LCD_Init65k()
{
	RA_RESET();   //Zresetuj kontroler

	for(uint8_t i=0; i < (sizeof(RA_Init) / sizeof(RA_Init[0])); i++)
	{
		if(RA_Init[i]==0xff) _delay_ms(1);
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

void LCD_DrawBitmap_Mono(uint16_t x, uint16_t y, uint16_t color, uint16_t bkgcolor, const uint8_t __flash *data)
{
	uint16_t width=*(const uint16_t __flash *)data++; data++;
	uint16_t height=*(const uint16_t __flash *)data++; data++;

	LCD_SetWindow(x, y, x + width -1, y + height - 1);
	LCD_SetPosition(x, y);
	LCD_SendCmd(RA_Memory_Read_Write_Command);    // Zapis pod wskazan¹ pozycjê
	//height/=8;

	uint16_t tmpcolor;

	for(uint32_t ox=0; ox < (width*height/8); ox++)
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
}

void LCD_DrawBitmap_565(uint16_t x, uint16_t y, const uint16_t __flash *data)
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