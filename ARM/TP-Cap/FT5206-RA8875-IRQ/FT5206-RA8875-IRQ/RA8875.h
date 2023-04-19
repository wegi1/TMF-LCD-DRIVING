/*
 * ssd2119.h
 *
 * Created: 2014-01-26 18:37:55
 *  Author: tmf
 */


#ifndef RA8875_H_
#define RA8875_H_

#include <stdint.h>
#include "i8080-defs.h"
#include "sam.h"
#include "Delay/systick_counter.h"

//Rejestr stanu kontrolera
typedef union
{
	struct
	{
		_Bool SerialFLASHBusy  : 1;
		uint8_t IB00           : 3;
		_Bool Sleep  	       : 1;
		_Bool TPEvent          : 1;
		_Bool BTEBusy          : 1;
		_Bool MemoryRWBusy     : 1;
	};
	uint8_t byte;
} RS8875_Status_Reg;

//Rejestr kontroli uk³adu i LCD (0x01)

typedef union
{
	struct
	{
		_Bool RESET			   : 1;		//Programowy reset uk³adu
		_Bool Sleep  	       : 1;		//1 - tryb uœpienia
		uint8_t IB00           : 5;
		_Bool LCDOn	           : 1;		//1 - LCD w³¹czony
	};
	uint8_t byte;
} RS8875_PWRR_Reg;

//Rejestr konfiguracji systemu
typedef enum {RA_MCUIF_8b=0b00, RA_MCUIF_16b=0b11} RA_SYSR_MCUIF;
typedef enum {RA_ColorDepth_16bpp=0b11, RA_ColorDepth_8bpp=0b00} RA_SYSR_ColorDepth;

typedef union
{
	struct
	{
		RA_SYSR_MCUIF MCUIF					: 2;		//Interfejs 8/16 bitowy
		RA_SYSR_ColorDepth ColorDepth       : 2;		//G³êbia kolorów 8/16 bitowa
		uint8_t IB00				        : 3;
	};
	uint8_t byte;
} RS8875_SYSR_Reg;

//Rejestr konfiguracji wyœwietlacza
typedef enum {RA_OneLayer=0x00, RA_TwoLayers=0x01, RA_HDIR_Seg0toN=0x00, RA_HDIR_SegNto0=0x01, RA_VDIR_Seg0toN=0x00, RA_VDIR_SegNto0=0x01} RA_DPCR;

typedef union
{
	struct
	{
		uint8_t IB00	    : 2;
		_Bool VDIR			: 1;		//Skanowanie pionowe
		_Bool HDIR          : 1;		//Skanowanie pionowe
		uint8_t IB01	    : 3;
		_Bool Layers		: 1;		//Liczba warstw (jedna lub dwie)
		
	};
	uint8_t byte;
} RS8875_DPCR_Reg;

//Rejestr kontrolny zapisu pamiêci
typedef enum {RA_MWLeftRightTopDown=0b00, RA_MWRightLeftTopDown=0b01, RA_MWTopDownLeftRight=0b10, RA_MWDownTopLeftRight=0b11} RA_MemDir;

typedef union
{
	struct
	{
		_Bool NoReadAutoIncr   : 1;			//0 - kursor automatycznie siê zwiêksza, 1 - kursor sta³y przy odczycie
		_Bool NoWriteAutoIncr  : 1;			//0 - kursor automatycznie siê zwiêksza, 1 - kursor sta³y przy zapisie
		RA_MemDir Direction    : 2;			//Kierunek zapisu do pamiêci
		_Bool IB00  	       : 1;
		_Bool BlinkEn          : 1;			//1 - kursor miga
		_Bool MemCursorEn      : 1;			//1 - kursor widoczny
		_Bool TextModeEn	   : 1;			//0 - tryb graficzny, 1 - tekstowy
	};
	uint8_t byte;
} RS8875_MWCR0_Reg;

//Rejestr kontrolny zapisu pamiêci 1
typedef enum {RA_DestWriteLayer12=0b00, RA_DestWriteCGRAM=0b01, RA_DestWriteGraphicCursor=0b10, RA_DestWritePattern=0b11} RA_DestWrite;
typedef union
{
	struct
	{
		_Bool Layer			   : 1;			//Zapisywana warstwa 0/1
		_Bool IB00  	       : 1;
		RA_DestWrite Memory    : 2;			//Pamiêæ do której odbywa siê zapis
		uint8_t CursorType     : 3;			//Typ kursowa - 0-7
		_Bool GrCursorEn	   : 1;			//1 - kursor graficzny odblokowany
	};
	uint8_t byte;
} RS8875_MWCR1_Reg;

//Rejestr kontrolny odczytu pamiêci
typedef union
{
	struct
	{
		RA_MemDir Direction    : 2;			//Kierunek odczytu pamiêci
		uint8_t IB00  	       : 6;
	};
	uint8_t byte;
} RS8875_MRCD_Reg;

//Rejestr kontroli kasowania pamiêci
typedef union
{
	struct
	{
		uint8_t IB00  	       : 6;
		_Bool CLRArea		   : 1;			//1 - kasujemy tylko aktywny obszar, 0 - kasujemy ca³e okno
		_Bool MCLR			   : 1;			//1 - kasujemy obszar, 0 - uk³ad nic nie robi
	};
	uint8_t byte;
} RS8875_MCLR_Reg;

//Rejestr kontroli PCLK
typedef enum {RA_PCLKDiv1=0b00, RA_PCLKDiv2=0b01, RA_PCLKDiv4=0b10, RA_PCLKDiv8=0b11} RA_PCLKDiv;
typedef enum {RA_PCLKPhasePositive=0b00, RA_PCLKPhaseNegative=0b01} RA_PCLKPhase;

typedef union
{
	struct
	{
		RA_PCLKDiv PCLKDiv			   : 2;
		uint8_t IB00				   : 5;
		RA_PCLKPhase PCLKInversion	   : 1;
	};
	uint8_t byte;
} RS8875_PCSR_Reg;

//Rejestry kontroli PLL
typedef enum {RA_PLLDivMNoDiv=0b00, RA_PLLDivM2=0b01} RA_PLLDivM;

typedef union
{
	struct
	{
		uint8_t		 PLLMult   : 5;		//Mno¿nik PLL 1-31
		uint8_t IB00		   : 2;
		RA_PCLKPhase PLLDivM   : 1;		//Wstêpny podzia³ przez 2
	};
	uint8_t byte;
} RS8875_PLLC1_Reg;

typedef enum {RA_PLLDivK1=0b000, RA_PLLDivK2=0b001, RA_PLLDivK4=0b010, RA_PLLDivK8=0b011, RA_PLLDivK16=0b100, RA_PLLDivK32=0b101, RA_PLLDivK64=0b110, RA_PLLDivK128=0b111} RA_PLLDivK;

typedef union
{
	struct
	{
		RA_PLLDivK	 PLLMult   : 3;		//Podzia³ 1-128
		uint8_t IB00		   : 5;
	};
	uint8_t byte;
} RS8875_PLLC2_Reg;

//Kontrola PWM

typedef enum {RA_PWMDivNoDiv=0, RA_PWMDiv2, RA_PWMDDiv4, RA_PWMDDiv8, RA_PWMDDiv16, RA_PWMDDiv32, RA_PWMDDiv64, RA_PWMDDiv128, RA_PWMDDiv256, RA_PWMDDiv512,
RA_PWMDDiv1024, RA_PWMDDiv2048, RA_PWMDDiv4096, RA_PWMDDiv8192, RA_PWMDDiv16384, RA_PWMDDiv32768} RA_PWMDiv;

typedef enum {RA_PWMNormal=0, RA_PWMFixed} RA_PWMFunc;

typedef union
{
	struct
	{
		RA_PWMDiv	 PWMDiv    : 4;		//Podzia³ zegara PWM
		RA_PWMFunc PWMFunc	   : 1;
		uint8_t IB00		   : 1;
		_Bool PWMDisLevel	   : 1;		//W trybie uœpienia wyjœcie PWM jest w stanie niskim (0) lub wysokim (1)
		_Bool PWMEn			   : 1;     //Odblokuj PWM (1)
	};
	uint8_t byte;
} RS8875_PWMCR_Reg;

//Uk³ad BTE

//Rejestr kontroli BTE 0

typedef enum {RA_BTERectangular=0, RA_BTELinear=1} RA_BTEDataType;

typedef union
{
	struct
	{
		uint8_t IB00		   : 5;
		_Bool DstDataType	   : 1;		//Typ danych (liniowy, prostok¹tny)
		_Bool SrcDataType      : 1;
		_Bool Enable           : 1;
	};
	uint8_t byte;
} RS8875_BECR0_Reg;

//Rejestrk kontroli BTE 1

typedef enum {RA_BTEOp_Write=0, RA_BTEOp_Read=1, RA_BTEOp_MovePositive=2, RA_BTEOp_MoveNegative=3, RA_BTEOp_TransparentWrite=4,
	RA_BTEOp_TransparentMovePositive=5, RA_BTEOpPatternFill=6, RA_BTEOp_PatternFillWithTransparency=7,
	RA_BTEOp_ColorExpansion=8, RA_BTEOp_ColorExpansionWithTransparency=9,
RA_BTEOp_MoveWithColorExpansion=10, RA_BTEOp_MoveWithColorExpansionAndTransparency=11, RA_BTEOp_SolidFill=12} RA_BTEROPOperationCode;


typedef union
{
	struct
	{
		RA_BTEROPOperationCode ROPOperation   : 4;
		uint8_t OpType				          : 4;
	};
	uint8_t byte;
} RS8875_BECR1_Reg;

//Rejestr kontroli przezroczystoœci 0

typedef enum {RA_BTELayer12ScrollSimult=0, RA_BTELayer1Scroll=1, RA_BTE_Layer2Scroll=2, RA_BTE_BufferScroll=3} RA_BTELayerScrollMode;
typedef enum {RA_BTELayer1Visible=0, RA_BTELayer2Visible=1, RA_BTE_LightenOverlay=2, RA_BTE_Transparent=3, RA_BTE_BooleanOR=4, RA_BTE_BooleanAND=5,
RA_BTE_FloatingWindow=6} RA_BTELayerDispMode;

typedef union
{
	struct
	{
		RA_BTELayerDispMode DisplayMode   : 3;
		uint8_t IB00					  : 2;
		_Bool FloatinWindowTransparency   : 1;
		RA_BTELayerScrollMode ScrMode	  : 2;
	};
	uint8_t byte;
} RS8875_LTPR0_Reg;

//Rejestr kontroli przezroczystoœci 2

typedef enum {RA_BTELayerOpaque=0, RA_BTELayer7_8=1, RA_BTELayer6_8=2, RA_BTELayer5_8=3, RA_BTELayer4_8=4, RA_BTELayer3_8=5, RA_BTELayer2_8=6,
RA_BTELayer1_8=7, RA_BTELayerDisable=7} RA_BTELayerTransp;

typedef union
{
	struct
	{
		RA_BTELayerTransp Layer1   : 4;
		RA_BTELayerTransp Layer2   : 4;
	};
	uint8_t byte;
} RS8875_LTPR1_Reg;

//Rejestr Ÿród³a przeznaczenia

typedef union
{
	struct
	{
		uint8_t SrcDstPoint   : 1;
		uint8_t IB00		  : 6;
		uint8_t Later		  : 1;
	};
	uint8_t byte;
} RS8875_BTESrcDstPoint_Reg;

//Rejestry kontrolne zwi¹zane z rysowaniem prymitywów

//Rejestr kontrolny

typedef enum {DCR_DrawLine=0, DCR_DrawSquare=16, DCR_DrawTriangle=1} RA_DCRDraw;
typedef enum {DCR_StartCircle=1, DCR_StartLineSquareTriangle=2, DCR_StartBusy=3} RA_DCRStart;

typedef union
{
	struct
	{
		RA_DCRDraw Draw			  : 5;
		_Bool Fill				  : 1;
		RA_DCRStart StartDrawing  : 2;
	};
	uint8_t byte;
} RS8875_DCR_Reg;

//Rejestr kontrolny rysowania elpis, krzywych i zaokr¹glonych prostok¹tów

typedef enum {DCRC_DrawEllipse=0, DCRC_DrawRoundedSquare=32, DCRC_Draw_EllipseQ1=16+2, DCRC_Draw_EllipseQ2=16+1, DCRC_Draw_EllipseQ3=16, DCRC_Draw_EllipseQ4=16+3} RA_DCRCDraw;

typedef union
{
	struct
	{
		RA_DCRCDraw Draw		  : 6;
		_Bool Fill				  : 1;
		_Bool StartDrawing		  : 1;
	};
	uint8_t byte;
} RS8875_DCRC_Reg;

//Rejestry kontroli fontów

typedef enum {FNTypeISO8859_1=0, FNTypeISO8859_2=1, FNTypeISO8859_3=2, FNTypeISO8859_4=3} RA_FNType;
typedef enum {FNTCGROM=0, FNTCGRAM=1} RA_FNTCG;

typedef union
{
	struct
	{
		RA_FNType FontType		  : 2;
		uint8_t IB00			  : 3;
		_Bool ExternalFont		  : 1;		//True - font spoza kontrolera
		uint8_t IB01			  : 1;
		RA_FNTCG FNTSource		  : 1;
	};
	uint8_t byte;
} RS8875_FNCR0_Reg;

typedef enum {FNTScale_x1=0, FNTScale_x2=1, FNTScale_x3=2, FNTScale_x4=3} RA_FNTScale;

typedef union
{
	struct
	{
		RA_FNTScale VerticalScale    : 2;
		RA_FNTScale HorizontalScale  : 2;
		_Bool FontRotation           : 1;	//True - obrót o 90 stopni
		uint8_t IB01			     : 1;
		_Bool FontTransparency       : 1;
		_Bool FontAlignment			 : 1;
	};
	uint8_t byte;
} RS8875_FNCR1_Reg;

//Przerwania

typedef union
{
	struct
	{
		_Bool BTERWStatus            : 1;
		_Bool BTEComplete			 : 1;
		_Bool TouchPanelFlag		 : 1;
		_Bool DMAStatus   			 : 1;
		_Bool KeyScanStatus			 : 1;
	};
	uint8_t byte;
} RS8875_INTC2_Reg;

//Rejestry konfiguracji systemu
#define RA_STATUS														0x00			//Rejestr stanu
#define RA_Power_and_Display_Control_Register							0x01
#define RA_Memory_Read_Write_Command									0x02
#define RA_Pixel_Clock_Setting_Register									0x04
#define RA_Serial_Flash_ROM_Configuration_Register						0x05
#define RA_Serial_Flash_ROM_CLK_Setting_Register						0x06
#define RA_System_Configuration_Register								0x10
#define RA_GPI															0x12
#define RA_GPO															0x13
#define RA_LCD_Horizontal_Display_Width_Register						0x14
#define RA_Horizontal_Non_Display_Period_Fine_Tuning_Option_Register	0x15
#define RA_LCD_Horizontal_Non_Display_Period_Register					0x16
#define RA_HSYNC_Start_Position_Register								0x17
#define RA_HSYNC_Pulse_Width_Register									0x18
#define RA_LCD_Vertical_Display_Height_Register0						0x19
#define RA_LCD_Vertical_Display_Height_Register1						0x1A
#define RA_LCD_Vertical_Non_Display_Period_Register0					0x1B
#define RA_LCD_Vertical_Non_Display_Period_Register1					0x1C
#define RA_VSYNC_Start_Position_Register0								0x1D
#define RA_VSYNC_Start_Position_Register1								0x1E
#define RA_VSYNC_Pulse_Width_Register									0x1F
//Rejestry konfiguracji LCD
#define RA_Display_Configuration_Register								0x20
#define RA_Font_Control_Register0										0x21
#define RA_Font_Control_Register1										0x22
#define RA_CGRAM_Select_Register										0x23
#define RA_Horizontal_Scroll_Offset_Register0							0x24
#define RA_Horizontal_Scroll_Offset_Register1							0x25
#define RA_Vertical_Scroll_Offset_Register0								0x26
#define RA_Vertical_Scroll_Offset_Register1								0x27
#define RA_Font_Line_Distance_Setting_Register							0x29
#define RA_Font_Write_Cursor_Horizontal_Position_Register0				0x2A
#define RA_Font_Write_Cursor_Horizontal_Position_Register1				0x2B
#define RA_Font_Write_Cursor_Vertical_Position_Register0				0x2C
#define RA_Font_Write_Cursor_Vertical_Position_Register1				0x2D
#define RA_Font_Write_Type_Setting_Register								0x2E
#define RA_Serial_Font_ROM_Setting										0x2F
//Rejestry konfiguracji okna dostêpu
#define RA_Horizontal_Start_Point_0_of_Active_Window					0x30
#define RA_Horizontal_Start_Point_1_of_Active_Window					0x31
#define RA_Vertical_Start_Point_0_of_Active_Window						0x32
#define RA_Vertical_Start_Point_1_of_Active_Window						0x33
#define RA_Horizontal_End_Point_0_of_Active_Window						0x34
#define RA_Horizontal_End_Point_1_of_Active_Window						0x35
#define RA_Vertical_End_Point_of_Active_Window_0						0x36
#define RA_Vertical_End_Point_of_Active_Window_1						0x37
#define RA_Horizontal_Start_Point_0_of_Scroll_Window					0x38
#define RA_Horizontal_Start_Point_1_of_Scroll_Window					0x39
#define RA_Vertical_Start_Point_0_of_Scroll_Window						0x3A
#define RA_Vertical_Start_Point_1_of_Scroll_Window						0x3B
#define RA_Horizontal_End_Point_0_of_Scroll_Window						0x3C
#define RA_Horizontal_End_Point_1_of_Scroll_Window						0x3D
#define RA_Vertical_End_Point_0_of_Scroll_Window						0x3E
#define RA_Vertical_End_Point_1_of_Scroll_Window						0x3F
//Rejestry konfiguracji kursora
#define RA_Memory_Write_Control_Register0								0x40
#define RA_Memory_Write_Control_Register1								0x41
#define RA_Blink_Time_Control_Register									0x44
#define RA_Memory_Read_Cursor_Direction									0x45
#define RA_Memory_Write_Cursor_Horizontal_Position_Register0			0x46
#define RA_Memory_Write_Cursor_Horizontal_Position_Register1			0x47
#define RA_Memory_Write_Cursor_Vertical_Position_Register0				0x48
#define RA_Memory_Write_Cursor_Vertical_Position_Register1				0x49
#define RA_Memory_Read_Cursor_Horizontal_Position_Register0				0x4A
#define RA_Memory_Read_Cursor_Horizontal_Position_Register1				0x4B
#define RA_Memory_Read_Cursor_Vertical_Position_Register0				0x4C
#define RA_Memory_Read_Cursor_Vertical_Position_Register1				0x4D
#define RA_Font_Write_Cursor_and_Memory_Write_Cursor_Horizontal_Size_Register	0x4E
#define RA_Font_Write_Cursor_Vertical_Size_Register						0x4F
//Rejestry konfiguracji uk³adu BTE
#define RA_BTE_Function_Control_Register0								0x50
#define RA_BTE_Function_Control_Register1								0x51
#define RA_Layer_Transparency_Register0									0x52
#define RA_Layer_Transparency_Register1									0x53
#define RA_Horizontal_Source_Point_0_of_BTE								0x54
#define RA_Horizontal_Source_Point_1_of_BTE								0x55
#define RA_Vertical_Source_Point_0_of_BTE								0x56
#define RA_Vertical_Source_Point_1_of_BTE								0x57
#define RA_Horizontal_Destination_Point_0_of_BTE						0x58
#define RA_Horizontal_Destination_Point_1_of_BTE						0x59
#define RA_Vertical_Destination_Point_0_of_BTE							0x5A
#define RA_Vertical_Destination_Point_1_of_BTE							0x5B
#define RA_BTE_Width_Register0											0x5C
#define RA_BTE_Width_Register1											0x5D
#define RA_BTE_Height_Register0											0x5E
#define RA_BTE_Height_Register1											0x5F
#define RA_Background_Color_Register0									0x60
#define RA_Background_Color_Register1									0x61
#define RA_Background_Color_Register2									0x62
#define RA_Foreground_Color_Register0									0x63
#define RA_Foreground_Color_Register1									0x64
#define RA_Foreground_Color_Register2									0x65
#define RA_Pattern_Set_No_for_BTE										0x66
#define RA_Background_Color_Register_for_Transparent0					0x67
#define RA_Background_Color_Register_for_Transparent1					0x68
#define RA_Background_Color_Register_for_Transparent2					0x69
//Rejestry konfiguracji panela dotykowego
#define RA_Touch_Panel_Control_Register0								0x70
#define RA_Touch_Panel_Control_Register1								0x71
#define RA_Touch_Panel_X_High_Byte_Data_Register						0x72
#define RA_Touch_Panel_Y_High_Byte_Data_Register						0x73
#define RA_Touch_Panel_X_Y_Low_Byte_Data_Register						0x74
//Rejestry konfiguracji kursora graficznego
#define RA_Graphic_Cursor_Horizontal_Position_Register0					0x80
#define RA_Graphic_Cursor_Horizontal_Position_Register1					0x81
#define RA_Graphic_Cursor_Vertical_Position_Register0					0x82
#define RA_Graphic_Cursor_Vertical_Position_Register1					0x83
#define RA_Graphic_Cursor_Color0										0x84
#define RA_Graphic_Cursor_Color1										0x85
//Rejestry kontroli PLL
#define RA_PLL_Control_Register1										0x88
#define RA_PLL_Control_Register2										0x89
//Rejestry kontroli PWM
#define RA_PWM1_Control_Register										0x8A
#define RA_PWM1_Duty_Cycle_Register										0x8B
#define RA_PWM2_Control_Register										0x8C
#define RA_PWM2_Duty_Cycle_Register										0x8D
#define RA_Memory_Clear_Control_Register								0x8E
//Rejestry kontroluj¹ce wyœwietlanie prymitywów
#define RA_Draw_Line_Circle_Square_Control_Register						0x90
#define RA_Draw_Line_Square_Horizontal_Start_Address_Register0			0x91
#define RA_Draw_Line_Square_Horizontal_Start_Address_Register1			0x92
#define RA_Draw_Line_Square_Vertical_Start_Address_Register0			0x93
#define RA_Draw_Line_Square_Vertical_Start_Address_Register1			0x94
#define RA_Draw_Line_Square_Horizontal_End_Address_Register0			0x95
#define RA_Draw_Line_Square_Horizontal_End_Address_Register1			0x96
#define RA_Draw_Line_Square_Vertical_End_Address_Register0				0x97
#define RA_Draw_Line_Square_Vertical_End_Address_Register1				0x98
#define RA_Draw_Circle_Center_Horizontal_Address_Register0				0x99
#define RA_Draw_Circle_Center_Horizontal_Address_Register1				0x9A
#define RA_Draw_Circle_Center_Vertical_Address_Register0				0x9B
#define RA_Draw_Circle_Center_Vertical_Address_Register1				0x9C
#define RA_Draw_Circle_Radius_Register									0x9D
#define RA_Draw_Ellipse_Ellipse_Curve_Circle_Square_Control_Register	0xA0
#define RA_Draw_Ellipse_Circle_Square_Long_axis_Setting_Register_A0		0xA1
#define RA_Draw_Ellipse_Circle_Square_Long_axis_Setting_Register_A1		0xA2
#define RA_Draw_Ellipse_Circle_Square_Short_axis_Setting_Register_B0	0xA3
#define RA_Draw_Ellipse_Circle_Square_Short_axis_Setting_Register_B1	0xA4
#define RA_Draw_Ellipse_Circle_Square_Center_Horizontal_Address_Register0	0xA5
#define RA_Draw_Ellipse_Circle_Square_Center_Horizontal_Address_Register1	0xA6
#define RA_Draw_Ellipse_Circle_Square_Center_Vertical_Address_Register0	0xA7
#define RA_Draw_Ellipse_Circle_Square_Center_Vertical_Address_Register1	0xA8
#define RA_Draw_Triangle_Point_2_Horizontal_Address_Register0			0xA9
#define RA_Draw_Triangle_Point_2_Horizontal_Address_Register1			0xAA
#define RA_Draw_Triangle_Point_2_Vertical_Address_Register0				0xAB
#define RA_Draw_Triangle_Point_2_Vertical_Address_Register1				0xAC
//Rejestry DMA
#define RA_Source_Starting_Address_REG0									0xB0
#define RA_Source_Starting_Address_REG1									0xB1
#define RA_Source_Starting_Address_REG2									0xB2
#define RA_Block_Width_REG0												0xB4
#define RA_DMA_Transfer_Number_REG0										0xB4
#define RA_Block_Width_REG1												0xB5
#define RA_Block_Height_REG0											0xB6
#define RA_DMA_Transfer_Number_REG1										0xB6
#define RA_Block_Height_REG1											0xB7
#define RA_Source_Picture_Width_REG0									0xB8
#define RA_DMA_Transfer_Number_REG2										0xB8
#define RA_Source_Picture_Width_REG1									0xB9
#define RA_DMA_Configuration_REG										0xBF

//Rejestry kontroli p³ywaj¹cego okna (PIP)
#define RA_Floating_Windows_Start_Address_XA0							0xD0
#define RA_Floating_Windows_Start_Address_XA1							0xD1
#define RA_Floating_Windows_Start_Address_YA0							0xD2
#define RA_Floating_Windows_Start_Address_YA1							0xD3
#define RA_Floating_Windows_Width0										0xD4
#define RA_Floating_Windows_Width1										0xD5
#define RA_Floating_Windows_Height0										0xD6
#define RA_Floating_Windows_Height1										0xD7
#define RA_Floating_Windows_Display_X_Address0							0xD8
#define RA_Floating_Windows_Display_X_Address1							0xD9
#define RA_Floating_Windows_Display_Y_Address0							0xDA
#define RA_Floating_Windows_Display_Y_Address1							0xDB
//Rejestry kontroli zewnêtrznej pamiêci szeregowej
#define RA_Serial_Flash_ROM_Direct_Access_Mode							0xE0
#define RA_Serial_Flash_ROM_Direct_Access_Mode_Address					0xE1
#define RA_Serial_Flash_ROM_Direct_Access_Data_Read						0xE2
//Rejestry kontroli przerwañ
#define RA_Interrupt_Control_Register1									0xF0
#define RA_Interrupt_Control_Register2									0xF1


void RA_SendCmdWithData(uint8_t cmd, uint8_t data)  __attribute__ ((flatten)); //Wyœlij polecenie i jego 8-bitowy parametr do kontrolera
void RA_SendCmdWithDataW(uint8_t cmd, uint16_t data)  __attribute__ ((flatten)); //Wyœlij polecenie i jego 16-bitowy parametr do dwóch kolejnych rejestrów po podanym
uint16_t RA_SendCmdReadData(uint8_t cmd)  __attribute__ ((flatten)); //Wyœlij poecenie i odczytaj parametr z rejestru kontrolera
uint8_t RA_ReadStatus();			//Odczytaj rejestr stanu uk³adu
void LCD_SetWindow(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);		//Ustaw okno dostêpu do GRAM
void LCD_SetPosition(uint16_t x, uint16_t y);					//Ustaw pozycjê zapisu (x,y) w GRAM

//Funkcje specyficzne dla BTE
void RA_BTE_SetBkgColor(uint16_t color);	//Ustaw kolor t³a w rejestrze koloru BTE
void RA_BTE_SetColor(uint16_t color);		//Ustaw kolor pierwszego planu w rejestrze koloru BTE

//Funkcje specyficzne dla BTE
static inline void RA_WaitForBTEReady()
{while(RA_SendCmdReadData(RA_BTE_Function_Control_Register0) & (RS8875_BECR0_Reg){.Enable=1}.byte);}                 //Zaczekaj na koniec operacji BTE
static inline void RA_WaitForWAIT() {delay_cycles(4); while(!(i8080_SIGNALS.IN.reg & RA8875_WAIT));}  //Zaczekaj na deaktywacjê sygna³u WAIT z kontrolera

void RA_BTE_SetSrc(uint16_t x, uint16_t y, uint8_t Layer);		//Ustaw pozycjê Ÿród³a danych dla BTE
void RA_BTE_SetDst(uint16_t x, uint16_t y, uint8_t Layer);		//Ustaw pozycjê danych docelowych BTE
void RA_BTE_SetWidthHeight(uint16_t width, uint16_t height);	//Ustaw szerokoœæ i wysokoœæ(liczbê pikseli) dla danych BTE
void RA_BTE_Move(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t width, uint16_t height, uint8_t srcLayer, uint8_t dstLayer, uint8_t direction); //Kopiuj bloki pamiêci w obrêbie GRAM

void RA_SetCGRAMChar(uint8_t charno, uint8_t desc[16]);			//Zdefiniuj znak w CGRAM
static inline void LCD_SetTextAttrs(_Bool transparency, _Bool align, RA_FNTScale xscale, RA_FNTScale yscale)	//Ustal przezroczystoœæ i powiêkszenie znaku wyœwietlanego sprzêtowo
{
	RA_SendCmdWithData(RA_Font_Control_Register1, (RS8875_FNCR1_Reg){.VerticalScale=yscale, .HorizontalScale=xscale, .FontTransparency=transparency, .FontAlignment=align}.byte);
}

void LCD_SetText(uint16_t x, uint16_t y, const char *tekst, RA_FNType FontType, _Bool CGRAM);  //Wyœwietl podany ³añcuch sprzêtowo

//Zapisz monochromatyczn¹ bitmapê w GRAM korzystaj¹c z ekspansji kolorów BTE
void LCD_DrawBitmap_Mono8bpp(uint16_t x, uint16_t y, uint16_t color, uint16_t bkgcolor, const uint8_t *data, _Bool transparency, uint8_t Layer);
uint16_t LCD_StoreBMPMonoInGRAM(uint16_t x, uint16_t y, const uint8_t *data, uint8_t Layer);  //Zapisuje monochromatyczn¹ bitmapê w GRAM na wybranej warstwie - dzia³a w trybie 8bpp
//Przesy³a bitmapê z GRAM na wyœwietlan¹ warstwê - dzia³a tylko w trybie 8 bpp
void LCD_MoveBMPMonoFromGRAM(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t fromx, uint16_t fromy, uint16_t color, uint16_t bkgcolor, uint8_t FromLayer, uint8_t ToLayer, _Bool transparency);
//Zapisuje monochromatyczn¹ bitmapê w GRAM na wybranej warstwie - dzia³a w trybie 16bpp
uint16_t LCD_StoreBMPMonoInGRAM16b(uint16_t x, uint16_t y, const uint8_t *data, uint8_t Layer);
//Przesy³a bitmapê z GRAM na wyœwietlan¹ warstwê - dzia³a tylko w trybie 16 bpp
void LCD_MoveBMPMonoFromGRAM16b(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t fromx, uint16_t fromy, uint16_t color, uint16_t bkgcolor, uint8_t FromLayer, uint8_t ToLayer, _Bool transparency);
//Zapisuje kolorow¹ bitmapê w formacie 565 w GRAM na wybranej warstwie - dzia³a w trybie 16bpp
uint16_t LCD_StoreBMP565InGRAM(uint16_t x, uint16_t y, const uint16_t *data, uint8_t Layer);
//Przesy³a bitmapê w formacie 565 z GRAM na wyœwietlan¹ warstwê - dzia³a tylko w trybie 16 bpp
void LCD_MoveBMP565FromGRAM(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t fromx, uint16_t fromy, uint8_t FromLayer, uint8_t ToLayer, _Bool transparency, uint16_t transcolor);
void RA_DrawBitmap332(uint16_t x, uint16_t y, const uint8_t *data, uint8_t layer); //Wyœwietl obraz w formacie 332 na ekranie w trybie 8bpp

#endif /* RA8875_H_ */