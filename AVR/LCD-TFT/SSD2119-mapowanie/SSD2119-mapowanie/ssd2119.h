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

#endif /* SSD2119_H_ */