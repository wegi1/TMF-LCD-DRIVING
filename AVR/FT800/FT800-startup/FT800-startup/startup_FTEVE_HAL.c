
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

        ft_uint8_t chipid;
        //Read Register ID to check if FT800 is ready.
        chipid = Ft_Gpu_Hal_Rd8(phost, REG_ID);
        while(chipid != 0x7C)
            {
                chipid = Ft_Gpu_Hal_Rd8(phost, REG_ID);
                Ft_Gpu_Hal_Sleep(100);
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

	phost->hal_config.spi_clockrate_khz=30000;	//Zwiêksz taktowanie SPI na maksymalne osi¹galne dla CPU lec znie wiêksze ni¿ 30 MHz
	Ft_Gpu_Hal_Open(phost);

	phost->ft_cmd_fifo_wp = Ft_Gpu_Hal_Rd16(phost,REG_CMD_WRITE);
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
	Ft_Gpu_CoCmd_Dlstart(phost);
	Ft_App_WrCoCmd_Buffer(phost,CLEAR(1, 1, 1));
	Ft_App_WrCoCmd_Buffer(phost,COLOR_RGB(231, 239, 75));
	Ft_Gpu_CoCmd_Text(phost,16, 131, 28, 0, "Nacisnij kolejne punkty w celu kalibracji panela");
	Ft_Gpu_CoCmd_Calibrate(phost, 0);
	
    Ft_App_WrCoCmd_Buffer(phost, DISPLAY());
    Ft_Gpu_CoCmd_Swap(phost);
    Ft_App_Flush_Co_Buffer(phost);
    Ft_Gpu_Hal_WaitCmdfifo_empty(phost);

    /* Close all the opened handles */
    Ft_Gpu_Hal_Close(phost);
    Ft_Gpu_Hal_DeInit();
	
	while (1);
}

/* Nothing beyond this */
