/*
 * SSD2119_optymalizacja.c
 *
 * Created: 2014-02-16 11:42:32
 *  Author: tmf
 */

#include "ssd2119.h"

//Pod³¹czenie LCD:
// PC4 - RESET
// PC6 - CS
// PC7 - RS
//Pod³¹czenie TC:
// PC5 - INT z uk³adu TC
// PC0 - TPCS
//USART-SPI:
// PC1 - SCK
// PC2 - MISO
// PC3 - MOSI

#define SD_CS    PIN0_bm       //Sygna³ CS pamiêci karty SD
#define TP_CS   PIN2_bm        //Sygna³ CS kontrolera TP
#define LCD_CS   PIN6_bm       //Sygna³ CS kontrolera LCD
#define LCD_RS   PIN7_bm       //Sygna³ RS kontrolera LCD
#define LCD_RESET   PIN4_bm    //Sygna³ RESET kontrolera LCD
#define LCD_SCK  PIN1_bm       //Sygna³ SCK
#define LCD_MISO PIN2_bm       //Sygna³ MISO
#define LCD_MOSI PIN3_bm       //Sygna³ MOSI
#define LCD_USART USARTC0      //Port USART wykorzystywany do komunikacji z LCD
#define LCD_PORT  PORTCFG_VP02MAP_PORTC_gc        //Port do ktorego pod³¹czony jest kontroler

#include <avr/io.h>
#include <util/delay.h>
#include "Clk/Clk.h"

__attribute__((always_inline)) inline void ssd2119_RS(_Bool state)
{
	if(state) VPORT0_OUT|=LCD_RS;
	  else VPORT0_OUT&=~LCD_RS;
}

__attribute__((always_inline)) inline void ssd2119_CS(_Bool state)
{
	if(state) VPORT0_OUT|=LCD_CS;
	else VPORT0_OUT&=~LCD_CS;
}

__attribute__((always_inline)) inline uint8_t SPI_RW_Byte(uint8_t byte)
{
	LCD_USART.DATA=byte;
	while(!(LCD_USART.STATUS & USART_TXCIF_bm));
	LCD_USART.STATUS=USART_TXCIF_bm;
	return LCD_USART.DATA;
}

static inline void ssd2119_RESET()
{
	VPORT0_OUT&=~LCD_RESET;
	_delay_us(20);               //Sygna³ reset musi trwaæ >20 us
	VPORT0_OUT|=LCD_RESET;
}

__attribute__((always_inline))  inline void ssd2119_SendDataWord(uint16_t data)
{
	ssd2119_CS(0);
	SPI_RW_Byte(data >> 8);  //Wyœlij bardziej znacz¹cy bajt danych
	ssd2119_CS(1);
	ssd2119_CS(0);
	SPI_RW_Byte(data & 0xff);  //Wyœlij mniej znacz¹cy bajt danych
	ssd2119_CS(1);
}

__attribute__((always_inline))  static inline void ssd2119_SendDataByte(uint8_t data)
{
	while(!(LCD_USART.STATUS & USART_DREIF_bm));
	LCD_USART.DATA=data;
}

__attribute__ ((flatten)) void ssd2119_SendCmd(uint8_t cmd)
{
	ssd2119_RS(0);
	ssd2119_CS(0);
	SPI_RW_Byte(cmd);
	ssd2119_CS(1);     //Deaktywuj kontroler
	ssd2119_RS(1);     //Deaktywuj tryb wysy³ania poleceñ
}

__attribute__ ((flatten)) void ssd2119_SendCmdWithData(uint8_t cmd, uint16_t data)
{
	ssd2119_SendCmd(cmd);
	ssd2119_SendDataWord(data);
}

void LCD_Interface_Init()
{
	PORTCFG_VPCTRLA=LCD_PORT;            //Zmapuj port do którego pod³¹czony jest LCD na port wirtualny
	VPORT0_OUT=LCD_CS | LCD_RS | SD_CS;  //Deaktywujemy kontrole i inne urz¹dzenia na magistrali SPI
	VPORT0_DIR=LCD_CS | LCD_RS | SD_CS | LCD_RESET | LCD_SCK | LCD_MOSI; //Ustaw odpowiednie piny jako wyjœcia

	LCD_USART.BAUDCTRLA=0;
	LCD_USART.BAUDCTRLB=0;                     //Fclk=FPER/2 - maksymalne taktowanie SPI
	LCD_USART.CTRLC=USART_CMODE_MSPI_gc;       //Tryb SPI 0
	LCD_USART.CTRLB=USART_TXEN_bm | USART_RXEN_bm;
}

typedef struct
{
	uint8_t reg;        //Numer rejestru uk³¹du SSD
	uint16_t value;     //Wartoœæ rejestru
} SSD_Regs;

const SSD_Regs __flash SSD_Init262[] = {{0x28, 0x0006}, {0x00, 0x0001}, {0x10, 0x0000}, {0x01, 0x32EF}, {0x02, 0x0600}, {0x03, 0x6A38}, {0x11, 0x4870},
	                                    {0x0F, 0x0000}, {0x0B, 0x5308}, {0x0C, 0x0003}, {0x0D, 0x000A}, {0x0E, 0x2E00}, {0x1E, 0x00BE}, {0x25, 0x8000},
								        {0x26, 0x7800}, {0x4E, 0x0000}, {0x4F, 0x0000}, {0x12, 0x08D9}, {0x30, 0x0000}, {0x31, 0x0104}, {0x32, 0x0100},
									    {0x33, 0x0305}, {0x34, 0x0505}, {0x35, 0x0305}, {0x36, 0x0707}, {0x37, 0x0300}, {0x3A, 0x1200}, {0x3B, 0x0800},
									    {0x07, 0x0033}};

const SSD_Regs __flash SSD_Init65[] = {{0x28, 0x0006}, {0x00, 0x0001}, {0x10, 0x0000}, {0x01, 0x32EF}, {0x02, 0x0600}, {0x03, 0x6A38}, {0x11, 0x6870},
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

const __flash uint8_t tryby[] = {0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff};

void LCD_Test262()
{
	uint8_t tryb=0;

	ssd2119_SendCmdWithData(ssd2119_Set_GDDRAM_X_address_counter, 0x0000);    // Pozycja X piksela
	ssd2119_SendCmdWithData(ssd2119_Set_GDDRAM_Y_address_counter, 0x0000);    // Pozycja Y piksela

	ssd2119_SendCmdWithData(ssd2119_Vertical_RAM_address_position, 0xEF00);
	ssd2119_SendCmdWithData(ssd2119_Horizontal_RAM_address_start_position, 0x0000);
	ssd2119_SendCmdWithData(ssd2119_Horizontal_RAM_address_end_position, 0x013F);

	ssd2119_SendCmd(ssd2119_Write_to_GRAM);    // Zapis pod wskazan¹ pozycjê

	while(1)
	{
		uint8_t r=tryby[tryb];
		uint8_t g=tryby[tryb+1];
		uint8_t b=tryby[tryb+2];
		ssd2119_CS(0);
		for(uint8_t y=0; y<240; y++)
		for(uint16_t x=0; x<320; x++)
		{
			ssd2119_SendDataByte(r);  //Red
			ssd2119_SendDataByte(g);  //Green
			ssd2119_SendDataByte(b);  //Blue
		}
		tryb+=3;
		tryb%=sizeof(tryby);
	}
}

typedef union
{
	struct
	{
		uint8_t blue  : 5;
		uint8_t green : 6;
		uint8_t red   : 5;
	};
	uint8_t byte[2];
} RGB565;

void LCD_Test65()
{
	uint8_t tryb=0;

	ssd2119_SendCmdWithData(ssd2119_Set_GDDRAM_X_address_counter, 0x0000);    // Pozycja X piksela
	ssd2119_SendCmdWithData(ssd2119_Set_GDDRAM_Y_address_counter, 0x0000);    // Pozycja Y piksela

	ssd2119_SendCmdWithData(ssd2119_Vertical_RAM_address_position, 0xEF00);
	ssd2119_SendCmdWithData(ssd2119_Horizontal_RAM_address_start_position, 0x0000);
	ssd2119_SendCmdWithData(ssd2119_Horizontal_RAM_address_end_position, 0x013F);

	ssd2119_SendCmd(ssd2119_Write_to_GRAM);    // Zapis pod wskazan¹ pozycjê

	while(1)
	{
		RGB565 rgb;
		rgb.red=tryby[tryb];
		rgb.green=tryby[tryb+1];
		rgb.blue=tryby[tryb+2];
		ssd2119_CS(0);
		for(uint8_t y=0; y<240; y++)
		for(uint16_t x=0; x<320; x++)
		{
			ssd2119_SendDataByte(rgb.byte[1]);
			ssd2119_SendDataByte(rgb.byte[0]);
		}
		tryb+=3;
		tryb%=sizeof(tryby);
	}
}

int main(void)
{
	Set32MHzClk();
	LCD_Interface_Init();
	//LCD_Init262();
	//LCD_Test262();
	LCD_Init65();
	LCD_Test65();
    while(1)
    {
        //TODO:: Please write your application code
    }
}