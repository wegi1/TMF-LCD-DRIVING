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
	VPORT0_DIR=0x00;
	VPORT1_DIR=0x00;
	LCD_RS(1);
	LCD_CS(0);
	LCD_RD(0);
	LCD_RD(1);
	status=VPORT0_IN;
	VPORT0_DIR=0xff;
	VPORT1_DIR=0xff;
	LCD_CS(1);
	LCD_RS(0);
	return status;
}

void LCD_Init65k()
{
	RA_RESET();   //Zresetuj kontroler

	RA_SendCmdWithData(RA_PLL_Control_Register1,(RS8875_PLLC1_Reg){.PLLDivM=RA_PLLDivMNoDiv, .PLLMult=12}.byte); //CLKO=CLK*12
    _delay_ms(1);										//Musimy odczekaæ co najmniej 100us na stabilizacjê PLL
	RA_SendCmdWithData(RA_PLL_Control_Register2,(RS8875_PLLC2_Reg){.PLLMult=RA_PLLDivK4}.byte); //SCLK=CLKO/4
    _delay_ms(1);
		
    RA_SendCmdWithData(RA_System_Configuration_Register, (RS8875_SYSR_Reg){.MCUIF=RA_MCUIF_16b, .ColorDepth=RA_ColorDepth_16bpp}.byte);  //65k kolorów, 16b interfejs
    
	RA_SendCmdWithData(RA_Pixel_Clock_Setting_Register,(RS8875_PCSR_Reg){.PCLKInversion=RA_PCLKPhaseNegative, .PCLKDiv=RA_PCLKDiv2}.byte); //Konfiguracja PCLK=SCLK/4, czyli dla CLK=20MHz, SCK=120 MHz, PCK=30 MHz
    _delay_ms(1);

    //Horizontal set
    RA_SendCmdWithData(RA_LCD_Horizontal_Display_Width_Register, LCD_GetMaxX()/8 - 1); //Liczba pikseli w poziomie wg wzoru (HDWR + 1)*8
    RA_SendCmdWithData(RA_Horizontal_Non_Display_Period_Fine_Tuning_Option_Register, 0x03); //Horizontal Non-Display Period Fine Tuning(HNDFT) [3:0]
    RA_SendCmdWithData(RA_LCD_Horizontal_Non_Display_Period_Register, 0x03);//Horizontal Non-Display Period (pixels) = (HNDR + 1)*8
    RA_SendCmdWithData(RA_HSYNC_Start_Position_Register, 0x03);//HSYNC Start Position(PCLK) = (HSTR + 1)*8
    RA_SendCmdWithData(RA_HSYNC_Pulse_Width_Register, 0x00);//HSYNC Width [4:0]   HSYNC Pulse width(PCLK) = (HPWR + 1)*8
    //Vertical set
	RA_SendCmdWithDataW(RA_LCD_Vertical_Display_Height_Register0, LCD_GetMaxY());
	
    RA_SendCmdWithData(RA_LCD_Vertical_Non_Display_Period_Register0, 0x14);//Vertical Non-Display area = (VNDR + 1)
    RA_SendCmdWithData(RA_LCD_Vertical_Non_Display_Period_Register1, 0x00);//Vertical Non-Display area = (VNDR + 1)
    RA_SendCmdWithData(RA_VSYNC_Start_Position_Register0, 0x06);//VSYNC Start Position(PCLK) = (VSTR + 1)
    RA_SendCmdWithData(RA_VSYNC_Start_Position_Register1, 0x00);//VSYNC Start Position(PCLK) = (VSTR + 1)
    RA_SendCmdWithData(RA_VSYNC_Pulse_Width_Register, 0x01);//VSYNC Pulse Width(PCLK) = (VPWR + 1)
    
    RA_SendCmdWithData(RA_PWM1_Duty_Cycle_Register, 0x80); //Jasnoœæ podœwietlenia
	RA_SendCmdWithData(RA_PWM1_Control_Register, (RS8875_PWMCR_Reg){.PWMEn=true, .PWMDiv=RA_PWMDiv2}.byte); //Konfiguracja PWM1
	
	LCD_SetWindow(0, 0, LCD_GetMaxX(), LCD_GetMaxY());	//Ustaw okno dostêpu na ca³y ekran

	RA_SendCmdWithData(RA_Power_and_Display_Control_Register, (RS8875_PWRR_Reg) {.LCDOn=true}.byte);
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

void LCD_Rect(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
	LCD_SetWindow(x1, y1, x2, y2);
	LCD_SetPosition(x1, y1);
	LCD_SendCmd(RA_Memory_Read_Write_Command);    // Zapis pod wskazan¹ pozycjê

	__uint24 val=(1+y2-y1)*(__uint24)(1+x2-x1) + 1;  //Policz z ilu pikseli sk³ada siê prostok¹t
	for(;val;val--)
	{
		i8080_Write_W(color);
	}
	LCD_CS(1);
}
