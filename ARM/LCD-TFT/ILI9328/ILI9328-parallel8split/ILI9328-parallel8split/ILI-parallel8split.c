/*
 * ILI_parallel.c
 *
 * Created: 2017-04-15 10:52:41
 * Author : tmf
 */ 

#include "sam.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "Clk/SetClk.h"
#include "Delay/delay.h"

#include "GFXDrv.h"
#include "i8080-arm.h"
#include "ILI9328.h"
#include "Fonts/Fonts.h"
#include "Icons.h"

void ILI_VScroll()
{
	uint16_t vscroll=0;
	ILI_SendCmdWithData(ILI9328_BASEIMAGEDISPLAYCONTROL, (ILI9328_BaseImageDisplayControl_Reg){.NDL=0, .REV=1, .VLE=1}.word);

	while(1)
	{
		ILI_SendCmdWithData(ILI9328_BASEIMAGESCROLL, vscroll + 1);
		vscroll=(vscroll+1) % 320;
		delay_ms(20);
	}
}

int main(void)
{
    Set48MHzClk();
    delay_init();

	LCD_Interface_Init();                //Inicjalizacja inerfejsu ��cz�cego z LCD
	LCD_Init262();                       //Inicjalizacja LCD

	LCD_Rect(0, 0, 319, 239, 0x000000ul);  //Skasuj ekran i przy okazji ustaw okno dost�pu do GRAM

	LCD_DrawBitmap_Scale(10, 10, 1, image_data_Browser48x48);

	ILI_SendCmdWithData(ILI9328_ResizingControlRegister, (ILI9328_ResizingControlRegister_Reg){.RSZ=ILI_Resize1to2, .RCH=ILI_0Pixels, .RCV=ILI_0Pixels}.word);
	LCD_DrawBitmap_Scale(60, 10, 2, image_data_Browser48x48);
	
	ILI_SendCmdWithData(ILI9328_ResizingControlRegister, (ILI9328_ResizingControlRegister_Reg){.RSZ=ILI_Resize1to4, .RCH=ILI_0Pixels, .RCV=ILI_0Pixels}.word);
	LCD_DrawBitmap_Scale(90, 10, 4, image_data_Browser48x48);
	//Przywr�� prawid�owe skalowanie
	ILI_SendCmdWithData(ILI9328_ResizingControlRegister, (ILI9328_ResizingControlRegister_Reg){.RSZ=ILI_NoResizing, .RCH=ILI_0Pixels, .RCV=ILI_0Pixels}.word);
	
	LCD_CircleAA(160, 120, 50, 0xfffffful, 0x000000ul);

	ILI_VScroll();

	while(1)
	{}
}
