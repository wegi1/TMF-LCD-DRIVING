/*
 * FT800Beginning.c
 *
 * Created: 2015-01-01 22:04:14
 *  Author: tmf
 */ 

#include <stdbool.h>
#include <avr/io.h>
#include <util/delay.h>

#include "FT800.h"
#include "FT_SPI.h"
#include "FT_GC.h"


void HostCommand(uint32_t HostCommand)
{
	uint32_t Addr;
	/* construct host command and send to graphics controller */
	Addr = HostCommand<<16;
	FT800_Read(Addr);//ideally sending 3 bytes is sufficient
	_delay_ms(20);//worst scenario
}


//Domyœlna, pusta lista wyœwietlania
const uint8_t __flash FT_DLCODE_BOOTUP[12] =
{
	0,0,0,2,  //Instrukcja GPU: CLEAR_COLOR_RGB - czarne t³o
	7,0,0,38, //Instrukcja GPU: CLEAR
	0,0,0,0,  //Instrukcja GPU: DISPLAY
};

void FT800_ActiveInternalClock()
{
	HostCommand(FT_ACTIVE); //ObudŸ koprocesor
	_delay_ms(20);
	FT800_WritefromFLASH(FT_RAM_DL,(uint8_t*)&FT_DLCODE_BOOTUP, sizeof(FT_DLCODE_BOOTUP));  //Za³aduj pust¹ listê wyœwietlania	
	FT800_Write(REG_DLSWAP,FT_DLSWAP_FRAME);  //Aktywuj listê przy najbli¿szej ramce
}

void FT800_Init()
{
	FT800_PD(true);   //Zainicjuj FT800
	_delay_ms(20);
	FT800_PD(false);
	_delay_ms(20);
	FT800_PD(true);
	_delay_ms(20);
	
	FT800_SPICLK10M();            //Zanim nie skonfigurujemy zegara taktowanie SPI musi byæ mniejsze ni¿ 10 MHz
	
	FT800_ActiveInternalClock();
	
#ifdef FT_DISPLAY_WQVGA_480x272
	FT800_Write16(REG_VSYNC0, 		FT_DISPLAY_VSYNC0_WQVGA );
	FT800_Write16(REG_VSYNC1, 		FT_DISPLAY_VSYNC1_WQVGA );
	FT800_Write16(REG_VOFFSET, 		FT_DISPLAY_VOFFSET_WQVGA);
	FT800_Write16(REG_VCYCLE, 		FT_DISPLAY_VCYCLE_WQVGA );
	FT800_Write16(REG_HSYNC0, 		FT_DISPLAY_HSYNC0_WQVGA );
	FT800_Write16(REG_HSYNC1, 		FT_DISPLAY_HSYNC1_WQVGA );
	FT800_Write16(REG_HOFFSET, 		FT_DISPLAY_HOFFSET_WQVGA);
	FT800_Write16(REG_HCYCLE, 		FT_DISPLAY_HCYCLE_WQVGA );
	FT800_Write16(REG_HSIZE,		FT_DISPLAY_HSIZE_WQVGA 	);
	FT800_Write16(REG_VSIZE, 		FT_DISPLAY_VSIZE_WQVGA 	);
	FT800_Write16(REG_PCLK_POL, 	FT_DISPLAY_PCLKPOL_WQVGA);
	FT800_Write16(REG_SWIZZLE, 		FT_DISPLAY_SWIZZLE_WQVGA);
	/* after configuring display parameters, configure pclk */
	FT800_Write16(REG_PCLK,			FT_DISPLAY_PCLK_WQVGA	);	
#endif	

#ifdef FT_DISPLAY_QVGA_320x240
	FT800_Write16(REG_VSYNC0, 		FT_DISPLAY_VSYNC0_QVGA );
	FT800_Write16(REG_VSYNC1, 		FT_DISPLAY_VSYNC1_QVGA );
	FT800_Write16(REG_VOFFSET, 		FT_DISPLAY_VOFFSET_QVGA);
	FT800_Write16(REG_VCYCLE, 		FT_DISPLAY_VCYCLE_QVGA );
	FT800_Write16(REG_HSYNC0, 		FT_DISPLAY_HSYNC0_QVGA );
	FT800_Write16(REG_HSYNC1, 		FT_DISPLAY_HSYNC1_QVGA );
	FT800_Write16(REG_HOFFSET, 		FT_DISPLAY_HOFFSET_QVGA);
	FT800_Write16(REG_HCYCLE, 		FT_DISPLAY_HCYCLE_QVGA );
	FT800_Write16(REG_HSIZE,		FT_DISPLAY_HSIZE_QVGA  );
	FT800_Write16(REG_VSIZE, 		FT_DISPLAY_VSIZE_QVGA  );
	FT800_Write16(REG_PCLK_POL, 	FT_DISPLAY_PCLKPOL_QVGA);
	FT800_Write16(REG_SWIZZLE, 		FT_DISPLAY_SWIZZLE_QVGA);
	/* after configuring display parameters, configure pclk */
	FT800_Write16(REG_PCLK,			FT_DISPLAY_PCLK_QVGA   );
#endif
	
	HostCommand(FT_CLKEXT);   //Ustaw zegar na zewnêrzny kwarc 12 MHz
	FT800_SPICLKMAX();        //Od teraz mo¿emy zwiêkszyæ taktowanie SPI nawet do 30 MHz
	//FT800_Write16(REG_ROTATE, 1);		//Obróæ obraz o 180 stopni
}

//Ustaw jasnoœæ podœwietlenia - czêstotliwoœæ PWM i wype³enienie
void FT800_SetPWM(uint16_t freq, uint8_t duty)
{
	FT800_Write16(REG_PWM_HZ, freq);   //Ustaw czêstotliwoœæ PWM
	FT800_Write(REG_PWM_DUTY, duty);   //Ustaw wype³nienie - jasnoœæ
}

void Test_DL()
{
	FT800_StartWrite(FT_RAM_DL);   //Rozpocznij ³adowanie nowej listy
	FT800_Write32b(CLEAR(1, 1, 1)); // clear screen
	FT800_Write32b(BEGIN(FT_BITMAPS)); // start drawing bitmaps
	FT800_Write32b(VERTEX2II(220, 110, 31, 'F')); // ascii F in font 31
	FT800_Write32b(VERTEX2II(244, 110, 31, 'T')); // ascii T
	FT800_Write32b(VERTEX2II(270, 110, 31, 'D')); // ascii D
	FT800_Write32b(VERTEX2II(299, 110, 31, 'I')); // ascii I
	FT800_Write32b(END());
	FT800_Write32b(COLOR_RGB(160, 22, 22)); // change color to red
	FT800_Write32b(POINT_SIZE(320)); // set point size to 20 pixels in radius
	FT800_Write32b(BEGIN(FT_POINTS)); // start drawing points
	FT800_Write32b(VERTEX2II(192, 133, 0, 0)); // red point
	FT800_Write32b(DISPLAY()); // display the image

	FT800_CS(false);  //Koniec ³adowania listy
	FT800_Write(REG_DLSWAP,FT_DLSWAP_FRAME);  //Aktywuj listê przy najbli¿szej ramce
}

void TouchPoint_CalibrationDL()
{
	FT800_StartWrite(FT_RAM_DL);   //Rozpocznij ³adowanie nowej listy
	FT800_Write32b(CLEAR(1, 1, 1)); // clear screen
	FT800_Write32b(COLOR_RGB(255, 255, 255)); // change color to white
	
	//FT800_Write32b(CMD_TEXT(236, 120, 28, FT_OPT_CENTER, "Dotknij ekran we wskazanych miejscach"));

	FT800_Write32b(CMD_CALIBRATE);
	FT800_Write32b(DISPLAY()); // display the image
	FT800_CS(false);  //Koniec ³adowania listy
	FT800_Write(REG_DLSWAP,FT_DLSWAP_FRAME);  //Aktywuj listê przy najbli¿szej ramce
}

int main(void)
{
	USART_init();      //Zainicjuj obs³ugê USART-SPI do komunikacji z FT800
	FT800_Init();      //Zainicjuj FT800
	
	DisplayOn();	
	
	//TouchPoint_CalibrationDL();
	//while(1);
	
	Test_DL();
	

while(1)
{
	for (int8_t i = 100; i >= 0; i -= 3)
	{
		FT800_SetPWM(0x00fa, i);
		_delay_ms(20);
	}
}
		
	
}