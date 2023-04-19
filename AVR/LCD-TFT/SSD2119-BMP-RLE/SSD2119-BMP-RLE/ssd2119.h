/*
 * ssd2119.h
 *
 * Created: 2014-01-26 18:37:55
 *  Author: tmf
 */


#ifndef SSD2119_H_
#define SSD2119_H_

#include <stdint.h>

//Definicja pól rejestru Entry Mode

typedef union
{
	struct
	{
		uint8_t IB02       : 3;    //Bity o wartoœci 0
		uint8_t AM         : 1;
		uint8_t ID         : 2;
		uint8_t TY         : 2;
		uint8_t DMode      : 1;
		uint8_t NoSync     : 1;
		uint8_t WMode      : 1;
		uint8_t DenMode    : 1;
		uint8_t IB12       : 1;
		uint8_t DFM        : 2;
		uint8_t VSMode     : 1;
	};
	uint16_t word;
} ssd2119_EntryMode_Reg;

typedef union
{
	struct
	{
		uint8_t NW;                //Liczba linii po jakich nastêpuje inwersja sygna³u steruj¹cego
		uint8_t WSMD       : 1;    //Okreœla sposób sterowania sygna³em WSYNC
		uint8_t EOR        : 1;    //Powoduje, ¿e inwersja sygna³u nastêpuje zgodnie z wyœwietlaniem ramek parzystych/nieparzystych
		uint8_t BC         : 1;    //Umo¿liwia inwersjê sygna³u steruj¹cego matryc¹ VCOM
		uint8_t ENWS       : 1;    //Odblokowuje sygna³ WSYNC
		uint8_t FLD        : 1;    //Zmienia sterowanie matryc¹ na 3-polowe co umo¿liwia redukcjê migania matrycy
	};
uint16_t word;
} ssd2119_LCD_Driving_Waveform_Control_Reg;

typedef union
{
	struct
	{
		uint8_t MUX;               //Liczba wyœwietlanych linii (1-240)
		uint8_t IB8        : 1;
		uint8_t TB         : 1;    //Skanowanie G1 to G240 (TB=1) lub G240 do G1 (TB=0)
		uint8_t SM         : 1;
		uint8_t BGR        : 1;    //Format RGB lub BGR
		uint8_t GD         : 1;
		uint8_t REV        : 1;    //Wyœwietl obraz na matrycy w negacji
		uint8_t RL         : 1;    //Wyœwietlanie S1 do S960 (RL=1), lub S960 do S1 (RL=0)
		uint8_t IB15       : 1;
	};
	uint16_t word;
} ssd2119_DriverOutputCtrl_Reg;

typedef union
{
	struct
	{
		uint8_t D0         : 1;
		uint8_t D1         : 1;
		uint8_t IB2        : 1;
		uint8_t CM         : 1;   //W³¹czenie trybu 8-kolorowego
		uint8_t DTE        : 1;
		uint8_t GON        : 1;
		uint8_t IB67       : 2;
		uint8_t SPT        : 1;   //Zezwolenie na podzia³ ekranu na dwa podekrany
		uint8_t VLE        : 2;   //Zewzwoleniena przewijanie w pionie ekranu 1 lub 2
		uint8_t PT         : 2;
		uint8_t IB1315     : 3;
	};
	uint16_t word;
} ssd2119_DisplayCtrl_Reg;

typedef union
{
	struct
	{
		union
		{
			uint8_t PK0PN      : 3;
			uint8_t PR0PN      : 3;
		};
		uint8_t            : 5;
		union
		{
			uint8_t PK1PN      : 3;
			uint8_t PR1PN      : 3;
		};
		uint8_t            : 5;
	};
	uint16_t word;
} ssd2119_GammaCtrl_Reg;

typedef union
{
	struct
	{
		uint8_t VR0PN      : 4;
		uint8_t            : 4;
		uint8_t VR1PN      : 5;
		uint8_t            : 3;
	};
	uint16_t word;
} ssd2119_AmplificationAdj_Reg;

typedef enum {VCore_13V, VCore_14V, VCore15V, VCore16V, VCore17V, VCore18V, VCore19V, VCore20V} VCore;  //Napiêcie rdzenia 1,4-2,0 V, domyœlnie 1,9V

typedef union
{
	struct
	{
		uint16_t Signature   : 9;    //Powinny mieæ wartoœæ 0b110011001
		VCore VCore          : 3;    //Napiêcie rdzenia kontrolera (1,3-2,0 V)
		uint8_t IB12         : 1;    //Ma byæ równy 0
		uint8_t DSLP         : 1;    //W³¹czenie trybu g³êbokiego uœpienia
	};
	uint16_t word;
} ssd2119_SleepMode2_Reg;

typedef union
{
	struct
	{
		uint8_t ZERO         : 8;    //Pole powinno mieæ wartoœæ 0
		uint8_t VDV          : 5;    //Amplituda VCOM
		uint8_t VCOMG        : 1;    //Jeœli 1 to mo¿emy sterowaæ napiêciem VCOMG, jeœli 0 VVOML jest równe Hiz
	};
	uint16_t word;
} ssd2119_PowerCtrl4_Reg;

typedef union
{
	struct
	{
		uint8_t VCM          : 6;    //Amplituda VCOMH
		uint8_t Res          : 1;    //Powinien mieæ wartoœæ 0
		uint8_t nOTP         : 1;    //Jeœli 1 to mo¿emy sterowaæ napiêciem VCOMH
	};
	uint16_t word;
} ssd2119_PowerCtrl5_Reg;

#define ssd2119_Oscillator_OSCEN     1      //W³¹cz oscylator
#define ssd2119_Oscillator_OSCDIS    0      //Wy³¹cz oscylator
#define ssd2119_Sleep_SLP_Enter      1      //WejdŸ w tryb uœpienia
#define ssd2119_Sleep_SLP_Exit       0      //WyjdŸ z trybu uœpienia

#define ssd2119_Frame_Freq_50        0x0000      //Oscylator 295k, ramka 50 Hz
#define ssd2119_Frame_Freq_55        0x2000      //Oscylator 325k, ramka 55 Hz
#define ssd2119_Frame_Freq_60        0x5000      //Oscylator 354k, ramka 60 Hz
#define ssd2119_Frame_Freq_65        0x8000      //Oscylator 380k, ramka 65 Hz
#define ssd2119_Frame_Freq_70        0xA000      //Oscylator 413k, ramka 70 Hz
#define ssd2119_Frame_Freq_75        0xC000      //Oscylator 443k, ramka 75 Hz
#define ssd2119_Frame_Freq_80        0xE000      //Oscylator 472k, ramka 80 Hz

#define ssd2119_Oscillator                                  0x00        //Rejestr oscylatora
#define ssd2119_Driver_Output_Control                       0x01        //Rejestr kontroli sterownika
#define ssd2119_LCD_Driving_Waveform_Control                0x02        //Rejestr kontroli matrycy
#define ssd2119_Display_Control                             0x07        //Rejestr kontroli wyœwietlacza

#define ssd2119_PowerControl_3                              0x0D        //Rejestr kontroli wzmocnienia napiêcia VLCD63
#define ssd2119_PowerControl_4                              0x0E        //Rejestr kontroli wzmocnienia napiêcia segmentów wspólnych (VCOML)
#define ssd2119_PowerControl_5                              0x1E        //Rejestr kontroli wzmocnienia napiêcia segmentów wspólnych (VCOMH)

#define ssd2119_GateScan_Position                           0x0F        //Numer pierwszej wyœwietlanej linii
#define ssd2119_Entry_Mode									0x11        //Rejestr trybu Entry Mode
#define ssd2119_SleepMode1                                  0x10        //Pierwszy rejestr uœpienia
#define ssd2119_SleepMode2                                  0x12        //Drugi rejestr uœpienia
#define ssd2119_Write_to_GRAM								0x22        //Zapis do GRAM

#define ssd2119_Frame_Frequency_Control                     0x25        //Rejestr kontroli czêstotliwoœci ramki

#define ssd2119_Gamma_microadjustment_positive_01_reg		0x30		//Rejestr mikrokorekcji gamma dla polaryzacji dodatniej
#define ssd2119_Gamma_microadjustment_positive_23_reg		0x31
#define ssd2119_Gamma_microadjustment_positive_45_reg		0x32
#define ssd2119_Gamma_adjustment_positive_01_reg			0x33		//Rejestr korekcji gamma dla polaryzacji dodatniej

#define ssd2119_Gamma_microadjustment_negative_01_reg		0x34		//Rejestr mikrokorekcji gamma dla polaryzacji ujemnej
#define ssd2119_Gamma_microadjustment_negative_23_reg		0x35
#define ssd2119_Gamma_microadjustment_negative_45_reg		0x36
#define ssd2119_Gamma_adjustment_negative_01_reg			0x37		//Rejestr korekcji gamma dla polaryzacji ujemnej

#define ssd2119_Gamma_amplification_positive_reg			0x3A		//Rejestr wzmocnienia gamma dla polaryzacji dodatniej
#define ssd2119_Gamma_amplification_negative_reg			0x3B		//Rejestr wzmocnienia gamma dla polaryzacji ujemnej

#define ssd2119_Vertical_Scroll_Control_Screen1             0x41        //Rejestr przewijania pionowego dla ekranu pierwszego
#define ssd2119_Vertical_Scroll_Control_Screen2             0x42        //Rejestr przewijania pionowego dla ekranu drugiego

#define ssd2119_First_Screen_Driving_Position_Start         0x48        //Pierwsza wyœwietlana linia dla pierwszego ekranu
#define ssd2119_First_Screen_Driving_Position_End           0x49        //Ostatnia wyœwietlana linia dla pierwszego ekranu
#define ssd2119_Second_Screen_Driving_Position_Start        0x4A        //Pierwsza wyœwietlana linia dla drugiego ekranu
#define ssd2119_Second_Screen_Driving_Position_End          0x4B        //Ostatnia wyœwietlana linia dla drugiego ekranu

#define ssd2119_Vertical_RAM_address_position               0x44        //Wsp. Y góry io do³u okna
#define ssd2119_Horizontal_RAM_address_start_position       0x45        //Wsp. X pocz¹tku okna
#define ssd2119_Horizontal_RAM_address_end_position         0x46        //Wsp. X koñca okna
#define ssd2119_Set_GDDRAM_X_address_counter                0x4E        //Wsp. X zapisywanego piksela
#define ssd2119_Set_GDDRAM_Y_address_counter                0x4F        //Wsp. Y zapisywanego piksela


void ssd2119_SendCmdWithData(uint8_t cmd, uint16_t data)  __attribute__ ((flatten)); //Wyœlij poecenie i jego parametr do kontrolera


void LCD_SetPosition(uint16_t x1, uint8_t y1);                          //Ustaw pozycjê (x,y) w GRAM
void LCD_SetWindow(uint16_t x1, uint8_t y1, uint16_t x2, uint8_t y2);   //Ustaw okno w pamiêci GRAM
#endif /* SSD2119_H_ */