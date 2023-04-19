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

#endif /* SSD2119_H_ */