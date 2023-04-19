
/*

Copyright (c) Future Technology Devices International 2014

THIS SOFTWARE IS PROVIDED BY FUTURE TECHNOLOGY DEVICES INTERNATIONAL LIMITED "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
FUTURE TECHNOLOGY DEVICES INTERNATIONAL LIMITED BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
OF SUBSTITUTE GOODS OR SERVICES LOSS OF USE, DATA, OR PROFITS OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

FTDI DRIVERS MAY BE USED ONLY IN CONJUNCTION WITH PRODUCTS BASED ON FTDI PARTS.

FTDI DRIVERS MAY BE DISTRIBUTED IN ANY FORM AS LONG AS LICENSE INFORMATION IS NOT MODIFIED.

IF A CUSTOM VENDOR ID AND/OR PRODUCT ID OR DESCRIPTION STRING ARE USED, IT IS THE
RESPONSIBILITY OF THE PRODUCT MANUFACTURER TO MAINTAIN ANY CHANGES AND SUBSEQUENT WHQL
RE-CERTIFICATION AS A RESULT OF MAKING THESE CHANGES.


*/

#include "FT_Platform.h"
#include "usbconv1.jpg.h"

//#define F16(s)        ((ft_int32_t)((s) * 65536))
#define WRITE2CMD(a) Ft_Gpu_Hal_WrCmdBuf(phost,a,sizeof(a))
#define SCRATCH_BUFF_SZ 2048 //increase this value will increase the performance but will use more host RAM space.

/* Global variables for display resolution to support various display panels */
/* Default is WQVGA - 480x272 */
#define FT_DispWidth 480
#define FT_DispHeight 272
#define FT_DispHCycle 548
#define FT_DispHOffset 43
#define FT_DispHSync0 0
#define FT_DispHSync1 41
#define FT_DispVCycle 292
#define FT_DispVOffset 12
#define FT_DispVSync0 0
#define FT_DispVSync1 10
#define FT_DispPCLK 5
#define FT_DispSwizzle 0
#define FT_DispPCLKPol 1
#define FT_DispCSpread 1
#define FT_DispDither 1

/* Global used for buffer optimization */
Ft_Gpu_Hal_Context_t host,*phost;

ft_uint32_t Ft_CmdBuffer_Index;
ft_uint32_t Ft_DlBuffer_Index;

#ifdef BUFFER_OPTIMIZATION
ft_uint8_t  Ft_DlBuffer[FT_DL_SIZE];
ft_uint8_t  Ft_CmdBuffer[FT_CMD_FIFO_SIZE];
#endif

/* Boot up for FT800 followed by graphics primitive sample cases */
/* Initial boot up DL - make the back ground green color */
const ft_uint8_t FT_PROGMEM FT_DLCODE_BOOTUP[12] =
{
    255,0,0,2,//GPU instruction CLEAR_COLOR_RGB
    7,0,0,38, //GPU instruction CLEAR
    0,0,0,0,  //GPU instruction DISPLAY
};

ft_void_t Ft_App_WrCoCmd_Buffer(Ft_Gpu_Hal_Context_t *phost,ft_uint32_t cmd)
{
#ifdef  BUFFER_OPTIMIZATION
   /* Copy the command instruction into buffer */
   ft_uint32_t *pBuffcmd;
   pBuffcmd =(ft_uint32_t*)&Ft_CmdBuffer[Ft_CmdBuffer_Index];
   *pBuffcmd = cmd;
#endif
   Ft_Gpu_Hal_WrCmd32(phost,cmd);
   /* Increment the command index */
   Ft_CmdBuffer_Index += FT_CMD_SIZE;
}

ft_void_t Ft_App_WrDlCmd_Buffer(Ft_Gpu_Hal_Context_t *phost,ft_uint32_t cmd)
{
#ifdef BUFFER_OPTIMIZATION
   /* Copy the command instruction into buffer */
   ft_uint32_t *pBuffcmd;
   pBuffcmd =(ft_uint32_t*)&Ft_DlBuffer[Ft_DlBuffer_Index];
   *pBuffcmd = cmd;
#endif

   Ft_Gpu_Hal_Wr32(phost,(RAM_DL+Ft_DlBuffer_Index),cmd);
   /* Increment the command index */
   Ft_DlBuffer_Index += FT_CMD_SIZE;
}

ft_void_t Ft_App_WrCoStr_Buffer(Ft_Gpu_Hal_Context_t *phost,const ft_char8_t *s)
{
#ifdef  BUFFER_OPTIMIZATION
  ft_uint16_t length = 0;
  length = strlen(s) + 1;//last for the null termination

  strcpy(&Ft_CmdBuffer[Ft_CmdBuffer_Index],s);

  /* increment the length and align it by 4 bytes */
  Ft_CmdBuffer_Index += ((length + 3) & ~3);
#endif
}

ft_void_t Ft_App_Flush_DL_Buffer(Ft_Gpu_Hal_Context_t *phost)
{
#ifdef  BUFFER_OPTIMIZATION
   if (Ft_DlBuffer_Index> 0)
     Ft_Gpu_Hal_WrMem(phost,RAM_DL,Ft_DlBuffer,Ft_DlBuffer_Index);
#endif
   Ft_DlBuffer_Index = 0;
}

ft_void_t Ft_App_Flush_Co_Buffer(Ft_Gpu_Hal_Context_t *phost)
{
#ifdef  BUFFER_OPTIMIZATION
   if (Ft_CmdBuffer_Index > 0)
     Ft_Gpu_Hal_WrCmdBuf(phost,Ft_CmdBuffer,Ft_CmdBuffer_Index);
#endif
   Ft_CmdBuffer_Index = 0;
}

/* API to give fadeout effect by changing the display PWM from 100 till 0 */
ft_void_t SAMAPP_fadeout()
{
   ft_int32_t i;

    for (i = 100; i >= 0; i -= 3)
    {
        Ft_Gpu_Hal_Wr8(phost,REG_PWM_DUTY,i);

        Ft_Gpu_Hal_Sleep(2);//sleep for 2 ms
    }
}

/* API to perform display fadein effect by changing the display PWM from 0 till 100 and finally 128 */
ft_void_t SAMAPP_fadein()
{
    ft_int32_t i;

    for (i = 0; i <=100 ; i += 3)
    {
        Ft_Gpu_Hal_Wr8(phost,REG_PWM_DUTY,i);
        Ft_Gpu_Hal_Sleep(2);//sleep for 2 ms
    }
    /* Finally make the PWM 100% */
    i = 128;
    Ft_Gpu_Hal_Wr8(phost,REG_PWM_DUTY,i);
}

/* API to check the status of previous DLSWAP and perform DLSWAP of new DL */
/* Check for the status of previous DLSWAP and if still not done wait for few ms and check again */
ft_void_t SAMAPP_GPU_DLSwap(ft_uint8_t DL_Swap_Type)
{
    ft_uint8_t Swap_Type = DLSWAP_FRAME,Swap_Done = DLSWAP_FRAME;

    if(DL_Swap_Type == DLSWAP_LINE)
    {
        Swap_Type = DLSWAP_LINE;
    }

    /* Perform a new DL swap */
    Ft_Gpu_Hal_Wr8(phost,REG_DLSWAP,Swap_Type);

    /* Wait till the swap is done */
    while(Swap_Done)
    {
        Swap_Done = Ft_Gpu_Hal_Rd8(phost,REG_DLSWAP);

        if(DLSWAP_DONE != Swap_Done)
        {
            Ft_Gpu_Hal_Sleep(10);//wait for 10ms
        }
    }
}

ft_void_t Ft_BootupConfig()
{
    Ft_Gpu_Hal_Powercycle(phost,FT_TRUE);

        /* Access address 0 to wake up the FT800 */
        Ft_Gpu_HostCommand(phost,FT_GPU_ACTIVE_M);
        Ft_Gpu_Hal_Sleep(20);
		
        /* Set the clk to external clock */
#ifndef ME800A_HV35R
        Ft_Gpu_HostCommand(phost,FT_GPU_EXTERNAL_OSC);
        Ft_Gpu_Hal_Sleep(10);
#endif

        {
            ft_uint8_t chipid;
            //Read Register ID to check if FT800 is ready.
            chipid = Ft_Gpu_Hal_Rd8(phost, REG_ID);
            while(chipid != 0x7C)
            {
                chipid = Ft_Gpu_Hal_Rd8(phost, REG_ID);
                Ft_Gpu_Hal_Sleep(100);
            }

    }
    /* Configuration of LCD display */
#ifdef DISPLAY_RESOLUTION_QVGA
    /* Values specific to QVGA LCD display */
#define    FT_DispWidth 320
#define    FT_DispHeight 240
#define    FT_DispHCycle 408
#define    FT_DispHOffset 70
#define    FT_DispHSync0 0
#define    FT_DispHSync1 10
#define    FT_DispVCycle 263
#define    FT_DispVOffset 13
#define    FT_DispVSync0 0
#define    FT_DispVSync1 2
#define    FT_DispPCLK 8
#define    FT_DispSwizzle 2
#define    FT_DispPCLKPol 0
#define    FT_DispCSpread 1
#define    FT_DispDither 1
#endif

#ifdef DISPLAY_RESOLUTION_WVGA
    /* Values specific to QVGA LCD display */
#define    FT_DispWidth 800
#define    FT_DispHeight 480
#define    FT_DispHCycle 928
#define    FT_DispHOffset 88
#define    FT_DispHSync0 0
#define    FT_DispHSync1 48
#define    FT_DispVCycle 525
#define    FT_DispVOffset 32
#define    FT_DispVSync0 0
#define    FT_DispVSync1 3
#define    FT_DispPCLK 2
#define    FT_DispSwizzle 0
#define    FT_DispPCLKPol 1
#define    FT_DispCSpread 0
#define    FT_DispDither 1
#endif

#ifdef DISPLAY_RESOLUTION_HVGA_PORTRAIT
    /* Values specific to HVGA LCD display */
#define    FT_DispWidth 320
#define    FT_DispHeight 480
#define    FT_DispHCycle 400
#define    FT_DispHOffset 40
#define    FT_DispHSync0 0
#define    FT_DispHSync1 10
#define    FT_DispVCycle 500
#define    FT_DispVOffset 10
#define    FT_DispVSync0 0
#define    FT_DispVSync1 5
#define    FT_DispPCLK 4
#define    FT_DispSwizzle 2
#define    FT_DispPCLKPol 1
#define    FT_DispCSpread 1
#define    FT_DispDither 1
#endif

#ifdef ME800A_HV35R
    /* After recognizing the type of chip, perform the trimming if necessary */
    Ft_Gpu_ClockTrimming(phost,LOW_FREQ_BOUND);
#endif

    Ft_Gpu_Hal_Wr16(phost, REG_HCYCLE, FT_DispHCycle);
    Ft_Gpu_Hal_Wr16(phost, REG_HOFFSET, FT_DispHOffset);
    Ft_Gpu_Hal_Wr16(phost, REG_HSYNC0, FT_DispHSync0);
    Ft_Gpu_Hal_Wr16(phost, REG_HSYNC1, FT_DispHSync1);
    Ft_Gpu_Hal_Wr16(phost, REG_VCYCLE, FT_DispVCycle);
    Ft_Gpu_Hal_Wr16(phost, REG_VOFFSET, FT_DispVOffset);
    Ft_Gpu_Hal_Wr16(phost, REG_VSYNC0, FT_DispVSync0);
    Ft_Gpu_Hal_Wr16(phost, REG_VSYNC1, FT_DispVSync1);
    Ft_Gpu_Hal_Wr8(phost, REG_SWIZZLE, FT_DispSwizzle);
    Ft_Gpu_Hal_Wr8(phost, REG_PCLK_POL, FT_DispPCLKPol);
    Ft_Gpu_Hal_Wr16(phost, REG_HSIZE, FT_DispWidth);
    Ft_Gpu_Hal_Wr16(phost, REG_VSIZE, FT_DispHeight);
    Ft_Gpu_Hal_Wr16(phost, REG_CSPREAD, FT_DispCSpread);
    Ft_Gpu_Hal_Wr16(phost, REG_DITHER, FT_DispDither);

#if (defined(ENABLE_FT_800) || defined(ENABLE_FT_810) ||defined(ENABLE_FT_812))
    /* Touch configuration - configure the resistance value to 1200 - this value is specific to customer requirement and derived by experiment */
    Ft_Gpu_Hal_Wr16(phost, REG_TOUCH_RZTHRESH,RESISTANCE_THRESHOLD);
#endif
    Ft_Gpu_Hal_Wr8(phost, REG_GPIO_DIR,0x80);
    Ft_Gpu_Hal_Wr8(phost, REG_GPIO,0x80);


    /*It is optional to clear the screen here*/
    Ft_Gpu_Hal_WrMemFromFlash(phost, RAM_DL,(ft_uint8_t *)FT_DLCODE_BOOTUP,sizeof(FT_DLCODE_BOOTUP));
    Ft_Gpu_Hal_Wr8(phost, REG_DLSWAP,DLSWAP_FRAME);

    Ft_Gpu_Hal_Wr8(phost, REG_PCLK,FT_DispPCLK);//after this display is visible on the LCD

	phost->ft_cmd_fifo_wp = Ft_Gpu_Hal_Rd16(phost,REG_CMD_WRITE);
}

void FT_LoadBitmap_Inflate(Ft_Gpu_Hal_Context_t *phost, const uint8_t *bmp, uint32_t addr, uint32_t len)
{
	Ft_Gpu_CoCmd_Inflate(phost, addr);	//Przegraj z dekompresj� dane ZLIB
	Ft_Gpu_Hal_WrCmdBufFromFlash(phost, bmp, len);	//Wy�lij dane o obrazie
}

uint32_t FT_GetPtr(Ft_Gpu_Hal_Context_t *phost)
{
	uint16_t wrptr=Ft_Gpu_Hal_Rd16(phost, REG_CMD_WRITE);	//Odczytaj bie��cy wska�nik zapisu w koprocesorze
	Ft_Gpu_CoCmd_GetPtr(phost, 0L);	//Wy�lij polecenie do GPU odczytu wska�nika
	return Ft_Gpu_Hal_Rd32(phost, RAM_CMD + wrptr + 4);  //Odczytaj wynik polecenia CMD_GETPTR
}

void FT_LoadBitmap_JPEG(Ft_Gpu_Hal_Context_t *phost, const uint8_t *bmp, uint32_t addr, uint32_t len, uint32_t options)
{
	Ft_Gpu_CoCmd_LoadImage(phost, addr, options);	//Wy�lij polecenie dekompresji jpeg
	Ft_Gpu_Hal_WrCmdBufFromFlash(phost, bmp, len);	//Wy�lij dane o obrazie
}

void FT_GetProps(Ft_Gpu_Hal_Context_t *phost, uint32_t *ptr, uint32_t *width, uint32_t *height)
{
	uint16_t wrptr=Ft_Gpu_Hal_Rd16(phost, REG_CMD_WRITE);	//Odczytaj bie��cy wska�nik zapisu w koprocesorze
	Ft_Gpu_CoCmd_GetProps(phost, *ptr, *width, *height);    //Wy�lij polecenie odczytu parametr�w JPEG
	*ptr=Ft_Gpu_Hal_Rd32(phost, RAM_CMD + wrptr + 4);      //Odczytaj wska�nik
	*width=Ft_Gpu_Hal_Rd32(phost, RAM_CMD + wrptr + 8);    //Odczytaj szeroko��
	*height=Ft_Gpu_Hal_Rd32(phost, RAM_CMD + wrptr + 12);  //Odczytaj wysoko��
}

void FT_LoadBitmap_Pateltted(Ft_Gpu_Hal_Context_t *phost, const uint8_t *bmp, const uint8_t *lut, uint32_t addr, uint32_t len)
{
	Ft_Gpu_Hal_WrMemFromFlash(phost, RAM_PAL, lut, 1024); //Wczytaj LUT - LUT ma 1024 bajty
	FT_LoadBitmap_Inflate(phost, bmp, addr, len);  //Wczytaj dane o obrazie
}

void FT_Reset_CoPro(Ft_Gpu_Hal_Context_t *phost)
{
	Ft_Gpu_Hal_Wr32(phost, REG_CPURESET, 1);   //Zresetuj koprocesor
	Ft_Gpu_Hal_Wr32(phost, REG_CMD_READ, 0);   //Rejestr odczytu koprocesora
	Ft_Gpu_Hal_Wr32(phost, REG_CMD_WRITE, 0);  //Rejestr zap;isu koprocesora
	Ft_Gpu_Hal_Wr32(phost, REG_CPURESET, 0);   //Normalna praca koprocesora
}

#define REG_SCREENSHOT_EN    0x102410        //Odblokuj uk��d przechwytywania
#define REG_SCREENSHOT_Y     0x102414        //Przechwytywana linia obrazu (Y)
#define REG_SCREENSHOT_START 0x102418        //Wyzw�l przechwycenie linii
#define REG_SCREENSHOT_READ  0x102554        //Odblokuj przechwytywanie liniii
#define REG_SCREENSHOT_BUSY  0x1024D8        //Czy ci�gle trwa przechwytywanie?
#define RAM_SCREENSHOT       0x1C2000        //Adres bufora RAM na screenshot


void FT_ScreenShot(Ft_Gpu_Hal_Context_t *phost, uint32_t *ptr)
{
	Ft_Gpu_Hal_Wr8(phost, REG_SCREENSHOT_EN, 1);  //Odblokuj przechwytywanie
	for (int ly = 0; ly < FT_DispHeight; ly++)
	{
		Ft_Gpu_Hal_Wr16(phost, REG_SCREENSHOT_Y, ly);     //Wpisz kolejn� przechwytywan� lini� Y
		Ft_Gpu_Hal_Wr8(phost, REG_SCREENSHOT_START, 1);   //Wyzw�l przechwycenie
		while (Ft_Gpu_Hal_Rd32(phost, REG_SCREENSHOT_BUSY) | Ft_Gpu_Hal_Rd32(phost, REG_SCREENSHOT_BUSY + 4));  //Zaczekaj na koniec przechwycenia linii
		Ft_Gpu_Hal_Wr8(phost, REG_SCREENSHOT_READ , 1);   //Rozpocznij odczyt bufora
		for (int lx = 0; lx < FT_DispWidth; lx ++)        //Odczytaj kolejne przechwycone dane o linii
		{
			*ptr++ = Ft_Gpu_Hal_Rd32(phost, RAM_SCREENSHOT + lx*4);
		}
		Ft_Gpu_Hal_Wr8(phost, REG_SCREENSHOT_READ, 0);
	}
	Ft_Gpu_Hal_Wr8(phost, REG_SCREENSHOT_EN, 0);   //Zablokuj uk��d przechwytywania
}

int main()
{
    Ft_Gpu_HalInit_t halinit;
    halinit.TotalChannelNum = 1;

    Ft_Gpu_Hal_Init(&halinit);
    host.hal_config.channel_no = 0;

    host.hal_config.spi_clockrate_khz = 4000; //in KHz
    Ft_Gpu_Hal_Open(&host);
    phost = &host;

    Ft_BootupConfig();

    Ft_Gpu_Hal_Sleep(100);//Show the booting up screen.
	
	FT_LoadBitmap_JPEG(phost, usbconv1_jpg, 37980, 27216, OPT_NODL);  //Za�aduj obrazek do GRAM
	
	Ft_Gpu_CoCmd_Dlstart(phost);
	Ft_App_WrCoCmd_Buffer(phost, CLEAR_COLOR_RGB(0,0,0));
	Ft_App_WrCoCmd_Buffer(phost, CLEAR(1,1,1));
	
	Ft_Gpu_CoCmd_ScreenSaver(phost);   //W��cz wygaszacz
	
	Ft_App_WrCoCmd_Buffer(phost, BITMAP_HANDLE(1));
	Ft_App_WrCoCmd_Buffer(phost, BITMAP_SOURCE(37980));
	Ft_App_WrCoCmd_Buffer(phost, BITMAP_LAYOUT(RGB565, 502, 260));
	Ft_App_WrCoCmd_Buffer(phost, BITMAP_SIZE(BILINEAR,BORDER,BORDER,251,260));
	Ft_Gpu_CoCmd_LoadIdentity(phost);
	Ft_Gpu_CoCmd_Scale(phost, 0x80ff, 0x80ff);
	Ft_Gpu_CoCmd_SetMatrix(phost);
	Ft_App_WrCoCmd_Buffer(phost, BEGIN(BITMAPS));
	
	Ft_App_WrCoCmd_Buffer(phost, MACRO(0));  //Tu zostanie wstawione VERTEX2F
	Ft_App_WrCoCmd_Buffer(phost, END());	

	Ft_App_WrCoCmd_Buffer(phost, DISPLAY());
	Ft_Gpu_CoCmd_Swap(phost);

    Ft_App_Flush_Co_Buffer(phost);
    Ft_Gpu_Hal_WaitCmdfifo_empty(phost);

    Ft_Gpu_Hal_Close(phost);
    Ft_Gpu_Hal_DeInit();
	
	while (1);
}

/* Nothing beyond this */
