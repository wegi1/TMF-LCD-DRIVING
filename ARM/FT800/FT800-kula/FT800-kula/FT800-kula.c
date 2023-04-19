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

#include "sam.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "Clk/SetClk.h"
#include "Delay/delay.h"

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
	255,255,255,2,//GPU instruction CLEAR_COLOR_RGB
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

	phost->hal_config.spi_clockrate_khz=30000;	//Zwiêksz taktowanie SPI na maksymalne osi¹galne dla CPU lec znie wiêksze ni¿ 30 MHz
	Ft_Gpu_Hal_Open(phost);
	
	phost->ft_cmd_fifo_wp = Ft_Gpu_Hal_Rd16(phost,REG_CMD_WRITE);
}

#define SAMAPP_ENABLE_APIS_SET0
#ifdef SAMAPP_ENABLE_APIS_SET0

/* Optimized implementation of sin and cos table - precision is 16 bit */
FT_PROGMEM ft_prog_uint16_t sintab[] = {
	0, 402, 804, 1206, 1607, 2009, 2410, 2811, 3211, 3611, 4011, 4409, 4807, 5205, 5601, 5997, 6392,
	6786, 7179, 7571, 7961, 8351, 8739, 9126, 9511, 9895, 10278, 10659, 11038, 11416, 11792, 12166, 12539,
	12909, 13278, 13645, 14009, 14372, 14732, 15090, 15446, 15799, 16150, 16499, 16845, 17189, 17530, 17868,
	18204, 18537, 18867, 19194, 19519, 19840, 20159, 20474, 20787, 21096, 21402, 21705, 22004, 22301, 22594,
	22883, 23169, 23452, 23731, 24006, 24278, 24546, 24811, 25072, 25329, 25582, 25831, 26077, 26318, 26556, 26789,
	27019, 27244, 27466, 27683, 27896, 28105, 28309, 28510, 28706, 28897, 29085, 29268, 29446, 29621, 29790, 29955,
	30116, 30272, 30424, 30571, 30713, 30851, 30984, 31113, 31236, 31356, 31470, 31580, 31684, 31785, 31880, 31970,
	32056, 32137, 32213, 32284, 32350, 32412, 32468, 32520, 32567, 32609, 32646, 32678, 32705, 32727, 32744, 32757,
32764, 32767, 32764};


ft_int16_t qsin(ft_uint16_t a)
{
	ft_uint8_t f;
	ft_int16_t s0,s1;

	if (a & 32768)
	return -qsin(a & 32767);
	if (a & 16384)
	a = 32768 - a;
	f = a & 127;
	s0 = sintab[a >> 7];
	s1 = sintab[(a >> 7) + 1];
	return (s0 + ((ft_int32_t)f * (s1 - s0) >> 7));
}

/* cos funtion */
ft_int16_t qcos(ft_uint16_t a)
{
	return (qsin(a + 16384));
}
#endif

/*****************************************************************************/
/* Example code to display few points at various offsets with various colors */

static ft_uint8_t home_star_icon[] = {0x78,0x9C,0xE5,0x94,0xBF,0x4E,0xC2,0x40,0x1C,0xC7,0x7F,0x2D,0x04,0x8B,0x20,0x45,0x76,0x14,0x67,0xA3,0xF1,0x0D,0x64,0x75,0xD2,0xD5,0x09,0x27,0x17,0x13,0xE1,0x0D,0xE4,0x0D,0x78,0x04,0x98,0x5D,0x30,0x26,0x0E,0x4A,0xA2,0x3E,0x82,0x0E,0x8E,0x82,0xC1,0x38,0x62,0x51,0x0C,0x0A,0x42,0x7F,0xDE,0xB5,0x77,0xB4,0x77,0x17,0x28,0x21,0x26,0x46,0xFD,0x26,0xCD,0xE5,0xD3,0x7C,0xFB,0xBB,0xFB,0xFD,0xB9,0x02,0xCC,0xA4,0xE8,0x99,0x80,0x61,0xC4,0x8A,0x9F,0xCB,0x6F,0x31,0x3B,0xE3,0x61,0x7A,0x98,0x84,0x7C,0x37,0xF6,0xFC,0xC8,0xDD,0x45,0x00,0xDD,0xBA,0xC4,0x77,0xE6,0xEE,0x40,0xEC,0x0E,0xE6,0x91,0xF1,0xD2,0x00,0x42,0x34,0x5E,0xCE,0xE5,0x08,0x16,0xA0,0x84,0x68,0x67,0xB4,0x86,0xC3,0xD5,0x26,0x2C,0x20,0x51,0x17,0xA2,0xB8,0x03,0xB0,0xFE,0x49,0xDD,0x54,0x15,0xD8,0xEE,0x73,0x37,0x95,0x9D,0xD4,0x1A,0xB7,0xA5,0x26,0xC4,0x91,0xA9,0x0B,0x06,0xEE,0x72,0xB7,0xFB,0xC5,0x16,0x80,0xE9,0xF1,0x07,0x8D,0x3F,0x15,0x5F,0x1C,0x0B,0xFC,0x0A,0x90,0xF0,0xF3,0x09,0xA9,0x90,0xC4,0xC6,0x37,0xB0,0x93,0xBF,0xE1,0x71,0xDB,0xA9,0xD7,0x41,0xAD,0x46,0xEA,0x19,0xA9,0xD5,0xCE,0x93,0xB3,0x35,0x73,0x0A,0x69,0x59,0x91,0xC3,0x0F,0x22,0x1B,0x1D,0x91,0x13,0x3D,0x91,0x73,0x43,0xF1,0x6C,0x55,0xDA,0x3A,0x4F,0xBA,0x25,0xCE,0x4F,0x04,0xF1,0xC5,0xCF,0x71,0xDA,0x3C,0xD7,0xB9,0xB2,0x48,0xB4,0x89,0x38,0x20,0x4B,0x2A,0x95,0x0C,0xD5,0xEF,0x5B,0xAD,0x96,0x45,0x8A,0x41,0x96,0x7A,0x1F,0x60,0x0D,0x7D,0x22,0x75,0x82,0x2B,0x0F,0xFB,0xCE,0x51,0x3D,0x2E,0x3A,0x21,0xF3,0x1C,0xD9,0x38,0x86,0x2C,0xC6,0x05,0xB6,0x7B,0x9A,0x8F,0x0F,0x97,0x1B,0x72,0x6F,0x1C,0xEB,0xAE,0xFF,0xDA,0x97,0x0D,0xBA,0x43,0x32,0xCA,0x66,0x34,0x3D,0x54,0xCB,0x24,0x9B,0x43,0xF2,0x70,0x3E,0x42,0xBB,0xA0,0x95,0x11,0x37,0x46,0xE1,0x4F,0x49,0xC5,0x1B,0xFC,0x3C,0x3A,0x3E,0xD1,0x65,0x0E,0x6F,0x58,0xF8,0x9E,0x5B,0xDB,0x55,0xB6,0x41,0x34,0xCB,0xBE,0xDB,0x87,0x5F,0xA9,0xD1,0x85,0x6B,0xB3,0x17,0x9C,0x61,0x0C,0x9B,0xA2,0x5D,0x61,0x10,0xED,0x2A,0x9B,0xA2,0x5D,0x61,0x10,0xED,0x2A,0x9B,0xA2,0x5D,0x61,0x10,0xED,0x2A,0x9B,0xED,0xC9,0xFC,0xDF,0x14,0x54,0x8F,0x80,0x7A,0x06,0xF5,0x23,0xA0,0x9F,0x41,0xF3,0x10,0x30,0x4F,0x41,0xF3,0x18,0x30,0xCF,0xCA,0xFC,0xFF,0x35,0xC9,0x79,0xC9,0x89,0xFA,0x33,0xD7,0x1D,0xF6,0x5E,0x84,0x5C,0x56,0x6E,0xA7,0xDA,0x1E,0xF9,0xFA,0xAB,0xF5,0x97,0xFF,0x2F,0xED,0x89,0x7E,0x29,0x9E,0xB4,0x9F,0x74,0x1E,0x69,0xDA,0xA4,0x9F,0x81,0x94,0xEF,0x4F,0xF6,0xF9,0x0B,0xF4,0x65,0x51,0x08};

ft_void_t home_setup()
{
	Ft_Gpu_Hal_WrCmd32(phost,CMD_INFLATE);
	Ft_Gpu_Hal_WrCmd32(phost,250*1024L);
	Ft_Gpu_Hal_WrCmdBuf(phost,home_star_icon,sizeof(home_star_icon));
	
	Ft_Gpu_CoCmd_Dlstart(phost);        // start
	Ft_App_WrCoCmd_Buffer(phost,CLEAR(1,1,1));
	Ft_App_WrCoCmd_Buffer(phost,COLOR_RGB(255, 255, 255));
	Ft_App_WrCoCmd_Buffer(phost,BITMAP_HANDLE(13));    // handle for background stars
	Ft_App_WrCoCmd_Buffer(phost,BITMAP_SOURCE(250*1024L));      // Starting address in gram
	Ft_App_WrCoCmd_Buffer(phost,BITMAP_LAYOUT(L4, 16, 32));  // format
	Ft_App_WrCoCmd_Buffer(phost,BITMAP_SIZE(NEAREST, REPEAT, REPEAT, 512, 512  ));
	Ft_App_WrCoCmd_Buffer(phost,BITMAP_HANDLE(14));    // handle for background stars
	Ft_App_WrCoCmd_Buffer(phost,BITMAP_SOURCE(250*1024L));      // Starting address in gram
	Ft_App_WrCoCmd_Buffer(phost,BITMAP_LAYOUT(L4, 16, 32));  // format
	Ft_App_WrCoCmd_Buffer(phost,BITMAP_SIZE(NEAREST, BORDER, BORDER, 32, 32  ));
	Ft_App_WrCoCmd_Buffer(phost,DISPLAY());
	Ft_Gpu_CoCmd_Swap(phost);
	Ft_App_Flush_Co_Buffer(phost);
	Ft_Gpu_Hal_WaitCmdfifo_empty(phost);
}

static ft_uint8_t sk=0;
ft_uint8_t Read_Keys()
{
	static ft_uint8_t Read_tag=0,temp_tag=0,ret_tag=0;
	Read_tag = Ft_Gpu_Hal_Rd8(phost,REG_TOUCH_TAG);
	ret_tag = 0;
	if(Read_tag != 0)								// Allow if the Key is released
	{
		if(temp_tag!=Read_tag)
		{
			temp_tag = Read_tag;
			sk = Read_tag;										// Load the Read tag to temp variable
		}
	}
	else
	{
		if(temp_tag!=0)
		{
			ret_tag = temp_tag;
		}
		sk = 0;
	}
	return ret_tag;
}

char *info[] = {  "FT800 MetaBalls Application",
	"APP to demonstrate interactive random MetaBalls,",
	"using Points & Bitmaps"
};

ft_void_t Info()
{
	ft_uint16_t dloffset = 0;
	Ft_CmdBuffer_Index = 0;
	
	Ft_Gpu_CoCmd_Dlstart(phost);
	Ft_App_WrCoCmd_Buffer(phost,CLEAR(1,1,1));
	Ft_App_WrCoCmd_Buffer(phost,COLOR_RGB(255,255,255));
	Ft_Gpu_CoCmd_Text(phost,FT_DispWidth/2,FT_DispHeight/2,26,OPT_CENTERX|OPT_CENTERY,"Please tap on a dot");
	Ft_Gpu_CoCmd_Calibrate(phost,0);
	Ft_App_WrCoCmd_Buffer(phost,DISPLAY());
	Ft_Gpu_CoCmd_Swap(phost);
	Ft_App_Flush_Co_Buffer(phost);
	Ft_Gpu_Hal_WaitCmdfifo_empty(phost);
	
	Ft_Gpu_CoCmd_Logo(phost);
	Ft_App_Flush_Co_Buffer(phost);
	Ft_Gpu_Hal_WaitCmdfifo_empty(phost);
	while(0!=Ft_Gpu_Hal_Rd16(phost,REG_CMD_READ));
	dloffset = Ft_Gpu_Hal_Rd16(phost,REG_CMD_DL);
	dloffset -=4;
	Ft_Gpu_Hal_WrCmd32(phost,CMD_MEMCPY);
	Ft_Gpu_Hal_WrCmd32(phost,100000L);
	Ft_Gpu_Hal_WrCmd32(phost,RAM_DL);
	Ft_Gpu_Hal_WrCmd32(phost,dloffset);
	do
	{
		Ft_Gpu_CoCmd_Dlstart(phost);
		Ft_Gpu_CoCmd_Append(phost,100000L,dloffset);
		Ft_App_WrCoCmd_Buffer(phost,BITMAP_TRANSFORM_A(256));
		Ft_App_WrCoCmd_Buffer(phost,BITMAP_TRANSFORM_A(256));
		Ft_App_WrCoCmd_Buffer(phost,BITMAP_TRANSFORM_B(0));
		Ft_App_WrCoCmd_Buffer(phost,BITMAP_TRANSFORM_C(0));
		Ft_App_WrCoCmd_Buffer(phost,BITMAP_TRANSFORM_D(0));
		Ft_App_WrCoCmd_Buffer(phost,BITMAP_TRANSFORM_E(256));
		Ft_App_WrCoCmd_Buffer(phost,BITMAP_TRANSFORM_F(0));
		Ft_App_WrCoCmd_Buffer(phost,SAVE_CONTEXT());
		Ft_App_WrCoCmd_Buffer(phost,COLOR_RGB(219,180,150));
		Ft_App_WrCoCmd_Buffer(phost,COLOR_A(220));
		Ft_App_WrCoCmd_Buffer(phost,BEGIN(EDGE_STRIP_A));
		Ft_App_WrCoCmd_Buffer(phost,VERTEX2F(0,FT_DispHeight*16));
		Ft_App_WrCoCmd_Buffer(phost,VERTEX2F(FT_DispWidth*16,FT_DispHeight*16));
		Ft_App_WrCoCmd_Buffer(phost,COLOR_A(255));
		Ft_App_WrCoCmd_Buffer(phost,RESTORE_CONTEXT());
		Ft_App_WrCoCmd_Buffer(phost,COLOR_RGB(0,0,0));
		// INFORMATION
		Ft_Gpu_CoCmd_Text(phost,FT_DispWidth/2,20,28,OPT_CENTERX|OPT_CENTERY,info[0]);
		Ft_Gpu_CoCmd_Text(phost,FT_DispWidth/2,60,26,OPT_CENTERX|OPT_CENTERY,info[1]);
		Ft_Gpu_CoCmd_Text(phost,FT_DispWidth/2,90,26,OPT_CENTERX|OPT_CENTERY,info[2]);
		//    Ft_Gpu_CoCmd_Text(phost,FT_DispWidth/2,120,26,OPT_CENTERX|OPT_CENTERY,info[3]);
		Ft_Gpu_CoCmd_Text(phost,FT_DispWidth/2,FT_DispHeight-30,26,OPT_CENTERX|OPT_CENTERY,"Click to play");
		if(sk!='P')
		Ft_App_WrCoCmd_Buffer(phost,COLOR_RGB(255,255,255));
		else
		Ft_App_WrCoCmd_Buffer(phost,COLOR_RGB(100,100,100));
		Ft_App_WrCoCmd_Buffer(phost,BEGIN(FTPOINTS));
		Ft_App_WrCoCmd_Buffer(phost,POINT_SIZE(20*16));
		Ft_App_WrCoCmd_Buffer(phost,TAG('P'));
		Ft_App_WrCoCmd_Buffer(phost,VERTEX2F((FT_DispWidth/2)*16,(FT_DispHeight-60)*16));
		Ft_App_WrCoCmd_Buffer(phost,COLOR_RGB(180,35,35));
		Ft_App_WrCoCmd_Buffer(phost,BEGIN(BITMAPS));
		Ft_App_WrCoCmd_Buffer(phost,VERTEX2II((FT_DispWidth/2)-14,(FT_DispHeight-75),14,4));
		Ft_App_WrCoCmd_Buffer(phost,DISPLAY());
		Ft_Gpu_CoCmd_Swap(phost);
		Ft_App_Flush_Co_Buffer(phost);
		Ft_Gpu_Hal_WaitCmdfifo_empty(phost);
	}while(Read_Keys()!='P');
}


static ft_int16_t v()
{
	return ft_random(64)-32;
}

static ft_uint8_t istouch()
{
	return !(Ft_Gpu_Hal_Rd16(phost,REG_TOUCH_RAW_XY) & 0x8000);
}

void Metaball()
{
	ft_uint8_t w = 31,h = 18,numBlobs = 80,
	*recip,fadein,f,temp[31];
	
	ft_int32_t
	centerx = (16 * 16 * (w / 2)),
	centery = (16 * 16 * (h / 2)),
	touching,
	recipsz = (w*w + h*h) / 4 + 1,
	tval,tval1,tval2,
	sx,sy,
	VEL,
	m,d,bx,by,dx,dy;

	struct
	{
		ft_int16_t x, y;
		ft_schar8_t dx, dy;
	} blobs[80];
	
	
	for (tval=0; tval<numBlobs; ++tval)
	{
		blobs[tval].x = ft_random(16 * FT_DispWidth);
		blobs[tval].y = ft_random(16 * FT_DispHeight);
		blobs[tval].dx = v();
		blobs[tval].dy = v();
	}

	recip = (ft_uint8_t*)malloc(recipsz);
	for (tval = 0; tval < recipsz; tval++)
	{
		if (tval == 0 )
		{
			recip[tval] = 200;
		}else
		{
			recip[tval] = MIN(200, (FT_DispWidth * 10) / (4 * tval));
		}
	}
	fadein = 255;
	f = 0;

	while(1)
	{
		//   Ft_Gpu_Hal_WaitCmdfifo_empty(phost);
		{
			touching = istouch();
			if (touching)
			{
				sx = Ft_Gpu_Hal_Rd16(phost,REG_TOUCH_SCREEN_XY + 2);
				sy = Ft_Gpu_Hal_Rd16(phost,REG_TOUCH_SCREEN_XY);
				centerx = 16 * sx;
				centery = 16 * sy;
			}
			else
			{
				centerx = FT_DispWidth * 16 / 2;
				centery = FT_DispHeight * 16 / 2;
			}
		}
		VEL = touching ? 8 : 2;
		for (tval=0; tval<numBlobs; ++tval)
		{
			if (blobs[tval].x < centerx)   blobs[tval].dx += VEL;    else
			blobs[tval].dx -= VEL;
			if (blobs[tval].y < centery)   blobs[tval].dy += VEL;   else
			blobs[tval].dy -= VEL;
			blobs[tval].x += blobs[tval].dx << 3;
			blobs[tval].y += blobs[tval].dy << 3;
		}
		blobs[ft_random(numBlobs)].dx = v();
		blobs[ft_random(numBlobs)].dy = v();
		for (tval1 = 0; tval1 < h; tval1++)
		{
			for (tval2 = 0; tval2 < w; tval2++)
			{
				m = fadein;
				for (tval = 0; tval < 3; tval++)
				{
					bx = blobs[tval].x >> 8;
					by = blobs[tval].y >> 8;
					dx = bx - tval2;
					dy = by - tval1;
					d = SQ(dx) + SQ(dy);
					m += recip[MIN(d >> 2, recipsz - 1)];
				}
				temp[tval2] = MIN(m,255);
			}
			Ft_Gpu_Hal_WrMem(phost,(f << 12) + (tval1 << 6),temp,w);
		}
		
		Ft_Gpu_CoCmd_Dlstart(phost);        // start
		Ft_App_WrCoCmd_Buffer(phost, CLEAR(1,1,1));
		Ft_App_WrCoCmd_Buffer(phost, BITMAP_SOURCE(f << 12));
		Ft_App_WrCoCmd_Buffer(phost, BITMAP_LAYOUT(L8, 64, 64));
		Ft_App_WrCoCmd_Buffer(phost, BITMAP_SIZE(BILINEAR, BORDER, BORDER, FT_DispWidth, FT_DispHeight));
		Ft_App_WrCoCmd_Buffer(phost, SAVE_CONTEXT());
		Ft_App_WrCoCmd_Buffer(phost, BITMAP_TRANSFORM_A(0x100 / 16));
		Ft_App_WrCoCmd_Buffer(phost, BITMAP_TRANSFORM_E(0x100 / 16));
		Ft_App_WrCoCmd_Buffer(phost, BEGIN(BITMAPS));

		Ft_App_WrCoCmd_Buffer(phost, BLEND_FUNC(SRC_ALPHA, ZERO));
		Ft_App_WrCoCmd_Buffer(phost, COLOR_RGB(255, 0, 0));
		Ft_App_WrCoCmd_Buffer(phost, VERTEX2II(0,0,0,0));
		Ft_App_WrCoCmd_Buffer(phost, BLEND_FUNC(SRC_ALPHA, ONE));

		Ft_App_WrCoCmd_Buffer(phost, COLOR_RGB(255, 255, 0));
		Ft_App_WrCoCmd_Buffer(phost, VERTEX2II(0,0,0,0));
		Ft_App_WrCoCmd_Buffer(phost, RESTORE_CONTEXT());
		Ft_App_WrCoCmd_Buffer(phost, COLOR_RGB(0, 0, 0));
		Ft_App_WrCoCmd_Buffer(phost, BEGIN(FTPOINTS));
		for (tval = 3; tval < numBlobs; tval++)
		{
			Ft_App_WrCoCmd_Buffer(phost,POINT_SIZE(3 * tval));
			Ft_App_WrCoCmd_Buffer(phost,VERTEX2F(blobs[tval].x, blobs[tval].y));
		}
		Ft_App_WrCoCmd_Buffer(phost,DISPLAY());
		Ft_Gpu_CoCmd_Swap(phost);
		Ft_App_Flush_Co_Buffer(phost);
		Ft_Gpu_Hal_WaitCmdfifo_empty(phost);
		fadein = MAX(fadein - 3, 1);
		f = (f + 1) & 3;
	}
}

ft_void_t loadDataToCoprocessorCMDfifo(ft_char8_t* fileName){
	FILE *fp;
	ft_uint32_t fileLen;
	ft_uint8_t pBuff[SCRATCH_BUFF_SZ];
	fp = fopen(fileName, "rb+");

	if(fp){

		fseek(fp,0,SEEK_END);
		fileLen = ftell(fp);
		fseek(fp,0,SEEK_SET);
		while(fileLen > 0)
		{
			ft_uint32_t blocklen = fileLen>SCRATCH_BUFF_SZ?SCRATCH_BUFF_SZ:fileLen;
			fread(pBuff,1,blocklen,fp);
			fileLen -= blocklen;
			Ft_Gpu_Hal_WrCmdBuf(phost,pBuff, blocklen);//alignment is already taken care by this api
		}
		fclose(fp);
		}else{
		printf("Unable to open file: %s\n",fileName);
		//exit(1);
	}
}
int main(void)
{
    //Set48MHzClk();
    delay_init();
	

Ft_Gpu_HalInit_t halinit;

halinit.TotalChannelNum = 1;


Ft_Gpu_Hal_Init(&halinit);
host.hal_config.channel_no = 0;
host.hal_config.spi_clockrate_khz = 4000; //in KHz

Ft_Gpu_Hal_Open(&host);

phost = &host;

Ft_BootupConfig();

/*It is optional to clear the screen here*/
Ft_Gpu_Hal_WrMem(phost, RAM_DL,(ft_uint8_t *)FT_DLCODE_BOOTUP,sizeof(FT_DLCODE_BOOTUP));
Ft_Gpu_Hal_Wr8(phost, REG_DLSWAP,DLSWAP_FRAME);

Ft_Gpu_Hal_Sleep(1000);//Show the booting up screen.

home_setup();
Info();
Metaball();
/* Close all the opened handles */
Ft_Gpu_Hal_Close(phost);
Ft_Gpu_Hal_DeInit();

while (1);
}
