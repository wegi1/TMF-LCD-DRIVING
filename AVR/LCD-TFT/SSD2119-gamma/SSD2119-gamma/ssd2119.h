/*
 * ssd2119.h
 *
 * Created: 2014-01-26 18:37:55
 *  Author: tmf
 */


#ifndef SSD2119_H_
#define SSD2119_H_

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
		uint8_t MUX;
		uint8_t IB8        : 1;
		uint8_t TB         : 1;
		uint8_t SM         : 1;
		uint8_t BGR        : 1;
		uint8_t GD         : 1;
		uint8_t REV        : 1;
		uint8_t RL         : 1;
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
		uint8_t CM         : 1;
		uint8_t DTE        : 1;
		uint8_t GON        : 1;
		uint8_t IB67       : 2;
		uint8_t SPT        : 1;
		uint8_t VLE        : 2;
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

typedef union
{
	struct
	{
		uint8_t blue   : 5;
		uint8_t green  : 6;
		uint8_t red    : 5;
	};
	uint16_t word;
} ssd2119_RGB565;

typedef union
{
	struct
	{
		uint8_t red    : 5;
		uint8_t green  : 6;
		uint8_t blue   : 5;
	};
	uint16_t word;
} ssd2119_BGR565;


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


#endif /* SSD2119_H_ */