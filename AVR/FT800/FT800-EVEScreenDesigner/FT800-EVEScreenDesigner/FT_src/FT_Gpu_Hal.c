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

Author : FTDI 

Revision History: 
0.1 - date 2013.04.24 - Initial Version
0.2 - date 2013.08.19 - added few APIs

*/

//#include "FT_Platform.h"
#include <FT_DataTypes.h>
#include <FT_Gpu_Hal.h>
#include <FT_Gpu.h>
#include <string.h>
#include <util/delay.h>
#include <FT_SPI.h>

/* API to initialize the SPI interface */
ft_bool_t Ft_Gpu_Hal_Init(Ft_Gpu_HalInit_t *halinit)
{
	return TRUE;
}

ft_bool_t Ft_Gpu_Hal_Open(Ft_Gpu_Hal_Context_t *host)
{
	USART_init();		//Zainicjuj wybrany interfejs SPI (realizowany przez USART)

	/* Initialize the context valriables */
	host->ft_cmd_fifo_wp = host->ft_dl_buff_wp = 0;
	host->spinumdummy = 1;//by default ft800/801/810/811 goes with single dummy byte for read
	host->spichannel = 0;
	host->status = FT_GPU_HAL_OPENED;

	return TRUE;
}

ft_void_t Ft_Gpu_Hal_Close(Ft_Gpu_Hal_Context_t *host)
{
	host->status = FT_GPU_HAL_CLOSED;
}

ft_void_t Ft_Gpu_Hal_DeInit()
{
}

/*The APIs for reading/writing transfer continuously only with small buffer system*/
ft_void_t  Ft_Gpu_Hal_StartTransfer(Ft_Gpu_Hal_Context_t *host,FT_GPU_TRANSFERDIR_T rw,ft_uint32_t addr)
{
	if (FT_GPU_READ == rw)
	{
		FT800_CS(true);
		FT800_SPIRW(addr >> 16);
		FT800_SPIRW(addr >> 8);
		FT800_SPIRW(addr & 0xff);
		FT800_SPIRW(0); //Dummy Read Byte

		host->status = FT_GPU_HAL_READING;
	}else
	{
		FT800_CS(true);
		FT800_SPIRW(0x80 | (addr >> 16));
		FT800_SPIRW(addr >> 8);
		FT800_SPIRW(addr & 0xff);

		host->status = FT_GPU_HAL_WRITING;
	}
}

/*The APIs for writing transfer continuously only*/
ft_void_t Ft_Gpu_Hal_StartCmdTransfer(Ft_Gpu_Hal_Context_t *host,FT_GPU_TRANSFERDIR_T rw, ft_uint16_t count)
{
	Ft_Gpu_Hal_StartTransfer(host,rw,host->ft_cmd_fifo_wp + RAM_CMD);
}

ft_uint8_t Ft_Gpu_Hal_TransferString(Ft_Gpu_Hal_Context_t *host,const ft_char8_t *string)
{
    ft_uint16_t length = strlen(string);
    while(length --){
       Ft_Gpu_Hal_Transfer8(host,*string);
       string ++;
    }
    //Append one null as ending flag
    Ft_Gpu_Hal_Transfer8(host,0);
}

ft_uint8_t Ft_Gpu_Hal_Transfer8(Ft_Gpu_Hal_Context_t *host,ft_uint8_t value)
{
	return FT800_SPIRW(value);
}

ft_uint16_t Ft_Gpu_Hal_Transfer16(Ft_Gpu_Hal_Context_t *host,ft_uint16_t value)
{
	ft_uint16_t retVal = 0;

    if (host->status == FT_GPU_HAL_WRITING){
		Ft_Gpu_Hal_Transfer8(host,value & 0xFF);//LSB first
		Ft_Gpu_Hal_Transfer8(host,(value >> 8) & 0xFF);
	}else{
		retVal = Ft_Gpu_Hal_Transfer8(host,0);
		retVal |= (ft_uint16_t)Ft_Gpu_Hal_Transfer8(host,0) << 8;
	}

	return retVal;
}
ft_uint32_t Ft_Gpu_Hal_Transfer32(Ft_Gpu_Hal_Context_t *host,ft_uint32_t value)
{
	ft_uint32_t retVal = 0;
	if (host->status == FT_GPU_HAL_WRITING){
		Ft_Gpu_Hal_Transfer16(host,value & 0xFFFF);//LSB first
		Ft_Gpu_Hal_Transfer16(host,(value >> 16) & 0xFFFF);
	}else{
		retVal = Ft_Gpu_Hal_Transfer16(host,0);
		retVal |= (ft_uint32_t)Ft_Gpu_Hal_Transfer16(host,0) << 16;
	}
	return retVal;
}

ft_void_t Ft_Gpu_Hal_EndTransfer(Ft_Gpu_Hal_Context_t *host)
{
	FT800_CS(false);
	host->status = FT_GPU_HAL_OPENED;
}

ft_uint8_t Ft_Gpu_Hal_Rd8(Ft_Gpu_Hal_Context_t *host,ft_uint32_t addr)
{
	ft_uint8_t value;
	Ft_Gpu_Hal_StartTransfer(host,FT_GPU_READ,addr);
	value = Ft_Gpu_Hal_Transfer8(host,0);
	Ft_Gpu_Hal_EndTransfer(host);
	return value;
}

ft_uint16_t Ft_Gpu_Hal_Rd16(Ft_Gpu_Hal_Context_t *host,ft_uint32_t addr)
{
	ft_uint16_t value;
	Ft_Gpu_Hal_StartTransfer(host,FT_GPU_READ,addr);
	value = Ft_Gpu_Hal_Transfer16(host,0);
	Ft_Gpu_Hal_EndTransfer(host);
	return value;
}

ft_uint32_t Ft_Gpu_Hal_Rd32(Ft_Gpu_Hal_Context_t *host,ft_uint32_t addr)
{
	ft_uint32_t value;
	Ft_Gpu_Hal_StartTransfer(host,FT_GPU_READ,addr);
	value = Ft_Gpu_Hal_Transfer32(host,0);
	Ft_Gpu_Hal_EndTransfer(host);
	return value;
}

ft_void_t Ft_Gpu_Hal_Wr8(Ft_Gpu_Hal_Context_t *host,ft_uint32_t addr, ft_uint8_t v)
{	
	Ft_Gpu_Hal_StartTransfer(host,FT_GPU_WRITE,addr);
	Ft_Gpu_Hal_Transfer8(host,v);
	Ft_Gpu_Hal_EndTransfer(host);
}

ft_void_t Ft_Gpu_Hal_Wr16(Ft_Gpu_Hal_Context_t *host,ft_uint32_t addr, ft_uint16_t v)
{
	Ft_Gpu_Hal_StartTransfer(host,FT_GPU_WRITE,addr);
	Ft_Gpu_Hal_Transfer16(host,v);
	Ft_Gpu_Hal_EndTransfer(host);
}

ft_void_t Ft_Gpu_Hal_Wr32(Ft_Gpu_Hal_Context_t *host,ft_uint32_t addr, ft_uint32_t v)
{
	Ft_Gpu_Hal_StartTransfer(host,FT_GPU_WRITE,addr);
	Ft_Gpu_Hal_Transfer32(host,v);
	Ft_Gpu_Hal_EndTransfer(host);
}

ft_void_t Ft_Gpu_HostCommand(Ft_Gpu_Hal_Context_t *host,ft_uint8_t cmd)
{
  FT800_CS(true);
  FT800_SPIRW(cmd);
  FT800_SPIRW(0);
  FT800_SPIRW(0);
  FT800_CS(false);
}

ft_void_t Ft_Gpu_ClockSelect(Ft_Gpu_Hal_Context_t *host,FT_GPU_PLL_SOURCE_T pllsource)
{
   Ft_Gpu_HostCommand(host,pllsource);
}

ft_void_t Ft_Gpu_PLL_FreqSelect(Ft_Gpu_Hal_Context_t *host,FT_GPU_PLL_FREQ_T freq)
{
   Ft_Gpu_HostCommand(host,freq);
}

ft_void_t Ft_Gpu_PowerModeSwitch(Ft_Gpu_Hal_Context_t *host,FT_GPU_POWER_MODE_T pwrmode)
{
   Ft_Gpu_HostCommand(host,pwrmode);
}

ft_void_t Ft_Gpu_CoreReset(Ft_Gpu_Hal_Context_t *host)
{
   Ft_Gpu_HostCommand(host,FT_GPU_CORE_RESET);
}

#ifdef FT_81X_ENABLE
//This API can only be called when PLL is stopped(SLEEP mode).  For compatibility, set frequency to the FT_GPU_12MHZ option in the FT_GPU_SETPLLSP1_T table.
ft_void_t Ft_Gpu_81X_SelectSysCLK(Ft_Gpu_Hal_Context_t *host, FT_GPU_81X_PLL_FREQ_T freq){
		if(FT_GPU_SYSCLK_72M == freq)
			Ft_Gpu_HostCommand_Ext3(host, (ft_uint32_t)0x61 | (0x40 << 8) | (0x06 << 8)); 
		else if(FT_GPU_SYSCLK_60M == freq)
			Ft_Gpu_HostCommand_Ext3(host, (ft_uint32_t)0x61 | (0x40 << 8) | (0x05 << 8)); 
		else if(FT_GPU_SYSCLK_48M == freq)
			Ft_Gpu_HostCommand_Ext3(host, (ft_uint32_t)0x61 | (0x40 << 8) | (0x04 << 8)); 
		else if(FT_GPU_SYSCLK_36M == freq)
			Ft_Gpu_HostCommand_Ext3(host, (ft_uint32_t)0x61 | (0x03 << 8)); 
		else if(FT_GPU_SYSCLK_24M == freq)
			Ft_Gpu_HostCommand_Ext3(host, (ft_uint32_t)0x61 | (0x02 << 8)); 
		else if(FT_GPU_SYSCLK_DEFAULT == freq)//default clock
			Ft_Gpu_HostCommand_Ext3(host, 0x61); 
}

//Power down or up ROMs and ADCs.  Specified one or more elements in the FT_GPU_81X_ROM_AND_ADC_T table to power down, unspecified elements will be powered up.  The application must retain the state of the ROMs and ADCs as they're not readable from the device.
ft_void_t Ft_GPU_81X_PowerOffComponents(Ft_Gpu_Hal_Context_t *host, ft_uint8_t val){
		Ft_Gpu_HostCommand_Ext3(host, (ft_uint32_t)0x49 | (val<<8));
}

//this API sets the current strength of supported GPIO/IO group(s)
ft_void_t Ft_GPU_81X_PadDriveStrength(Ft_Gpu_Hal_Context_t *host, FT_GPU_81X_GPIO_DRIVE_STRENGTH_T strength, FT_GPU_81X_GPIO_GROUP_T group){
		Ft_Gpu_HostCommand_Ext3(host, (ft_uint32_t)0x70 | (group << 8) | (strength << 8));
}

//this API will hold the system reset active, Ft_Gpu_81X_ResetRemoval() must be called to release the system reset.
ft_void_t Ft_Gpu_81X_ResetActive(Ft_Gpu_Hal_Context_t *host){
	Ft_Gpu_HostCommand_Ext3(host, FT_GPU_81X_RESET_ACTIVE); 
}

//This API will release the system reset, and the system will exit reset and behave as after POR, settings done through SPI commands will not be affected.
ft_void_t Ft_Gpu_81X_ResetRemoval(Ft_Gpu_Hal_Context_t *host){
	Ft_Gpu_HostCommand_Ext3(host, FT_GPU_81X_RESET_REMOVAL); 
}
#endif

//This API sends a 3byte command to the host
ft_void_t Ft_Gpu_HostCommand_Ext3(Ft_Gpu_Hal_Context_t *host,ft_uint32_t cmd)
{
	  FT800_CS(true);
	  FT800_SPIRW(cmd);
	  FT800_SPIRW((cmd>>8) & 0xff);
	  FT800_SPIRW((cmd>>16) & 0xff);
	  FT800_CS(true);
}

ft_void_t Ft_Gpu_Hal_Updatecmdfifo(Ft_Gpu_Hal_Context_t *host,ft_uint32_t count)
{
	host->ft_cmd_fifo_wp  = (host->ft_cmd_fifo_wp + count) & 4095;

	//4 byte alignment
	host->ft_cmd_fifo_wp = (host->ft_cmd_fifo_wp + 3) & 0xffc;
	Ft_Gpu_Hal_Wr16(host,REG_CMD_WRITE,host->ft_cmd_fifo_wp);
}

ft_uint16_t Ft_Gpu_Cmdfifo_Freespace(Ft_Gpu_Hal_Context_t *host)
{
	ft_uint16_t fullness,retval;

	//host->ft_cmd_fifo_wp = Ft_Gpu_Hal_Rd16(host,REG_CMD_WRITE);

	fullness = (host->ft_cmd_fifo_wp - Ft_Gpu_Hal_Rd16(host,REG_CMD_READ)) & 4095;
	retval = (FT_CMD_FIFO_SIZE - 4) - fullness;
	return (retval);
}

ft_void_t Ft_Gpu_Hal_WrCmdBuf(Ft_Gpu_Hal_Context_t *host,ft_uint8_t *buffer,ft_uint32_t count)
{
	ft_int32_t length =0, availablefreesize, SizeTransfered;   

#define MAX_CMD_FIFO_TRANSFER   Ft_Gpu_Cmdfifo_Freespace(host)  
	do {                
		length = count;
		availablefreesize = MAX_CMD_FIFO_TRANSFER;

		if (length > availablefreesize)
		{
		    length = availablefreesize;
		}
      	        Ft_Gpu_Hal_CheckCmdBuffer(host,length);

                Ft_Gpu_Hal_StartCmdTransfer(host,FT_GPU_WRITE,length);

                SizeTransfered = 0;
		while (length--) {
                    Ft_Gpu_Hal_Transfer8(host,*buffer);
		    buffer++;
                    SizeTransfered ++;
		}
                length = SizeTransfered;

		Ft_Gpu_Hal_EndTransfer(host);
		Ft_Gpu_Hal_Updatecmdfifo(host,length);

		Ft_Gpu_Hal_WaitCmdfifo_empty(host);

		count -= length;
	}while (count > 0);
}

ft_void_t Ft_Gpu_Hal_WrCmdBufFromFlash(Ft_Gpu_Hal_Context_t *host, ft_prog_uchar8_t *buffer,ft_uint32_t count)
{
	ft_uint32_t length =0, SizeTransfered = 0;   

#define MAX_CMD_FIFO_TRANSFER   Ft_Gpu_Cmdfifo_Freespace(host)  
	do {                
		length = count;
		if (length > MAX_CMD_FIFO_TRANSFER){
		    length = MAX_CMD_FIFO_TRANSFER;
		}
      	        Ft_Gpu_Hal_CheckCmdBuffer(host,length);

                Ft_Gpu_Hal_StartCmdTransfer(host,FT_GPU_WRITE,length);


                SizeTransfered = 0;
		while (length--) {
                    Ft_Gpu_Hal_Transfer8(host,ft_pgm_read_byte_near(buffer));
		    buffer++;
                    SizeTransfered ++;
		}
                length = SizeTransfered;

    	        Ft_Gpu_Hal_EndTransfer(host);
		Ft_Gpu_Hal_Updatecmdfifo(host,length);

		Ft_Gpu_Hal_WaitCmdfifo_empty(host);

		count -= length;
	}while (count > 0);
}

ft_void_t Ft_Gpu_Hal_CheckCmdBuffer(Ft_Gpu_Hal_Context_t *host,ft_uint32_t count)
{
   ft_uint16_t getfreespace;
   do{
        getfreespace = Ft_Gpu_Cmdfifo_Freespace(host);
   }while(getfreespace < count);
}
ft_void_t Ft_Gpu_Hal_WaitCmdfifo_empty(Ft_Gpu_Hal_Context_t *host)
{
   while(Ft_Gpu_Hal_Rd16(host,REG_CMD_READ) != Ft_Gpu_Hal_Rd16(host,REG_CMD_WRITE));
   
   host->ft_cmd_fifo_wp = Ft_Gpu_Hal_Rd16(host,REG_CMD_WRITE);
}

ft_void_t Ft_Gpu_Hal_WrCmdBuf_nowait(Ft_Gpu_Hal_Context_t *host,ft_uint8_t *buffer,ft_uint32_t count)
{
	ft_uint32_t length =0, SizeTransfered;   

#define MAX_CMD_FIFO_TRANSFER   Ft_Gpu_Cmdfifo_Freespace(host)  
	do {                
		length = count;
		if (length > MAX_CMD_FIFO_TRANSFER){
		    length = MAX_CMD_FIFO_TRANSFER;
		}
      	        Ft_Gpu_Hal_CheckCmdBuffer(host,length);

                Ft_Gpu_Hal_StartCmdTransfer(host,FT_GPU_WRITE,length);

                SizeTransfered = 0;
		while (length--) {
                    Ft_Gpu_Hal_Transfer8(host,*buffer);
		    buffer++;
                    SizeTransfered ++;
		}
                length = SizeTransfered;

		Ft_Gpu_Hal_EndTransfer(host);
		Ft_Gpu_Hal_Updatecmdfifo(host,length);

	//	Ft_Gpu_Hal_WaitCmdfifo_empty(host);

		count -= length;
	}while (count > 0);
}

ft_uint8_t Ft_Gpu_Hal_WaitCmdfifo_empty_status(Ft_Gpu_Hal_Context_t *host)
{
   if(Ft_Gpu_Hal_Rd16(host,REG_CMD_READ) != Ft_Gpu_Hal_Rd16(host,REG_CMD_WRITE))
   {
     return 0;
   }
   else
   {
     host->ft_cmd_fifo_wp = Ft_Gpu_Hal_Rd16(host,REG_CMD_WRITE);
     return 1;
   }  
}

ft_void_t Ft_Gpu_Hal_WaitLogo_Finish(Ft_Gpu_Hal_Context_t *host)
{
    ft_int16_t cmdrdptr,cmdwrptr;

    do{
         cmdrdptr = Ft_Gpu_Hal_Rd16(host,REG_CMD_READ);
         cmdwrptr = Ft_Gpu_Hal_Rd16(host,REG_CMD_WRITE);
    }while ((cmdwrptr != cmdrdptr) || (cmdrdptr != 0));
    host->ft_cmd_fifo_wp = 0;
}

ft_void_t Ft_Gpu_Hal_ResetCmdFifo(Ft_Gpu_Hal_Context_t *host)
{
   host->ft_cmd_fifo_wp = 0;
}

ft_void_t Ft_Gpu_Hal_WrCmd32(Ft_Gpu_Hal_Context_t *host,ft_uint32_t cmd)
{
         Ft_Gpu_Hal_CheckCmdBuffer(host,sizeof(cmd));
         Ft_Gpu_Hal_Wr32(host,RAM_CMD + host->ft_cmd_fifo_wp,cmd);
         Ft_Gpu_Hal_Updatecmdfifo(host,sizeof(cmd));
}

ft_void_t Ft_Gpu_Hal_ResetDLBuffer(Ft_Gpu_Hal_Context_t *host)
{
           host->ft_dl_buff_wp = 0;
}

/* Toggle PD_N pin of FT800 board for a power cycle*/
ft_void_t Ft_Gpu_Hal_Powercycle(Ft_Gpu_Hal_Context_t *host, ft_bool_t up)
{
	if (up)
	{
            FT800_PD(true);
            Ft_Gpu_Hal_Sleep(20);

            FT800_PD(false);
            Ft_Gpu_Hal_Sleep(20);
			
			FT800_PD(true);
			_delay_ms(20);
	}else
	{
            FT800_PD(false);
            Ft_Gpu_Hal_Sleep(20);
            
            FT800_PD(true);
            Ft_Gpu_Hal_Sleep(20);
	}
}

ft_void_t Ft_Gpu_Hal_WrMemFromFlash(Ft_Gpu_Hal_Context_t *host,ft_uint32_t addr,const ft_prog_uchar8_t *buffer, ft_uint32_t length)
{
	Ft_Gpu_Hal_StartTransfer(host,FT_GPU_WRITE,addr);

	while (length--) {
            Ft_Gpu_Hal_Transfer8(host,ft_pgm_read_byte_near(buffer));
	    buffer++;
	}

	Ft_Gpu_Hal_EndTransfer(host);
}

ft_void_t Ft_Gpu_Hal_WrMem(Ft_Gpu_Hal_Context_t *host,ft_uint32_t addr,const ft_uint8_t *buffer, ft_uint32_t length)
{
	Ft_Gpu_Hal_StartTransfer(host,FT_GPU_WRITE,addr);
	while (length--) {
            Ft_Gpu_Hal_Transfer8(host,*buffer);
	    buffer++;
	}

	Ft_Gpu_Hal_EndTransfer(host);
}


ft_void_t Ft_Gpu_Hal_RdMem(Ft_Gpu_Hal_Context_t *host,ft_uint32_t addr, ft_uint8_t *buffer, ft_uint32_t length)
{
	Ft_Gpu_Hal_StartTransfer(host,FT_GPU_READ,addr);
	while (length--) {
	   *buffer = Ft_Gpu_Hal_Transfer8(host,0);
	   buffer++;
	}

	Ft_Gpu_Hal_EndTransfer(host);
}

/* Helper api for dec to ascii */
ft_int32_t Ft_Gpu_Hal_Dec2Ascii(ft_char8_t *pSrc,ft_int32_t value)
{
	ft_int16_t Length;
	ft_char8_t *pdst,charval;
	ft_int32_t CurrVal = value,tmpval,i;
	ft_char8_t tmparray[16];
	ft_uint8_t idx = 0;

	Length = strlen(pSrc);
	pdst = pSrc + Length;

	if(0 == value)
	{
		*pdst++ = '0';
		*pdst++ = '\0';
		return 0;
	}

	if(CurrVal < 0)
	{
		*pdst++ = '-';
		CurrVal = - CurrVal;
	}
	/* insert the value */
	while(CurrVal > 0){
		tmpval = CurrVal;
		CurrVal /= 10;
		tmpval = tmpval - CurrVal*10;
		charval = '0' + tmpval;
		tmparray[idx++] = charval;
	}

	for(i=0;i<idx;i++)
	{
		*pdst++ = tmparray[idx - i - 1];
	}
	*pdst++ = '\0';

	return 0;
}

ft_void_t Ft_Gpu_Hal_Sleep(ft_uint32_t ms)
{
	for(ft_uint32_t i=ms; i>0; i--) _delay_ms(1);
}

#ifdef FT_81X_ENABLE
ft_int16_t Ft_Gpu_Hal_SetSPI(Ft_Gpu_Hal_Context_t *host,FT_GPU_SPI_NUMCHANNELS_T numchnls,FT_GPU_SPI_NUMDUMMYBYTES numdummy)
{
	ft_uint8_t writebyte = 0;
	/* error check */
	if((numchnls > FT_GPU_SPI_QUAD_CHANNEL) || (numdummy > FT_GPU_SPI_TWODUMMY) || (numdummy < FT_GPU_SPI_ONEDUMMY))
	{
		return -1;//error
	}

	host->spichannel = numchnls;
	writebyte = host->spichannel;
	host->spinumdummy = numdummy;

	if(FT_GPU_SPI_TWODUMMY == host->spinumdummy)
	{
		writebyte |= FT_SPI_TWO_DUMMY_BYTE;
	}
	Ft_Gpu_Hal_Wr8(host,REG_SPI_WIDTH,writebyte);
	/* set the parameters in hal context and also set into ft81x */
	return 0;
}
#endif

/* FIFO related apis */
//Init all the parameters of fifo buffer
ft_void_t Ft_Fifo_Init(Ft_Fifo_t *pFifo,ft_uint32_t StartAddress,ft_uint32_t Length,ft_uint32_t HWReadRegAddress,ft_uint32_t HWWriteRegAddress)
{
	/* update the context parameters */
	pFifo->fifo_buff = StartAddress;
	pFifo->fifo_len = Length;
	pFifo->fifo_rp = pFifo->fifo_wp = 0;

	/* update the hardware register addresses - specific to FT800 series chips */
	pFifo->HW_Read_Reg = HWReadRegAddress;
	pFifo->HW_Write_Reg = HWWriteRegAddress;
}

//update both the read and write pointers
ft_void_t Ft_Fifo_Update(Ft_Gpu_Hal_Context_t *host,Ft_Fifo_t *pFifo)
{
	pFifo->fifo_rp = Ft_Gpu_Hal_Rd32(host,pFifo->HW_Read_Reg);
	//Ft_Gpu_Hal_Wr32(host,pFifo->HW_Write_Reg,pFifo->fifo_wp);
}

//just write and update the write register
ft_uint32_t Ft_Fifo_Write(Ft_Gpu_Hal_Context_t *host,Ft_Fifo_t *pFifo,ft_uint8_t *buffer,ft_uint32_t NumbytetoWrite)
{
	ft_uint32_t FreeSpace = Ft_Fifo_GetFreeSpace(host,pFifo),TotalBytes = NumbytetoWrite;

	if(NumbytetoWrite > FreeSpace)
	{
		/* update the read pointer and get the free space */
		Ft_Fifo_Update(host,pFifo);
		FreeSpace = Ft_Fifo_GetFreeSpace(host,pFifo);

		if(NumbytetoWrite > FreeSpace)
		{
			TotalBytes = FreeSpace;
		}
	}

	/* sanity check */
	if(TotalBytes <= 0)	return 0;//error condition

	/* check for the loopback conditions */
	if(pFifo->fifo_wp + TotalBytes >= pFifo->fifo_len)
	{
		ft_uint32_t partialchunk = pFifo->fifo_len - pFifo->fifo_wp,secpartialchunk = TotalBytes - partialchunk;

		Ft_Gpu_Hal_WrMem(host,pFifo->fifo_buff + pFifo->fifo_wp,buffer,partialchunk);
		if(secpartialchunk > 0)
		{
			Ft_Gpu_Hal_WrMem(host,pFifo->fifo_buff,buffer + partialchunk,secpartialchunk);
		}
		pFifo->fifo_wp = secpartialchunk;

	}
	else
	{
		Ft_Gpu_Hal_WrMem(host,pFifo->fifo_buff + pFifo->fifo_wp,buffer,TotalBytes);
		pFifo->fifo_wp += TotalBytes;
	}

	/* update the write pointer address in write register */
	Ft_Gpu_Hal_Wr32(host,pFifo->HW_Write_Reg,pFifo->fifo_wp);

	return TotalBytes;
}
//just write one word and update the write register
ft_void_t Ft_Fifo_Write32(Ft_Gpu_Hal_Context_t *host,Ft_Fifo_t *pFifo,ft_uint32_t WriteWord)
{
	Ft_Fifo_Write(host,pFifo,(ft_uint8_t *)&WriteWord,4);
}

//write and wait for the fifo to be empty. handle cases even if the Numbytes are more than freespace
ft_void_t Ft_Fifo_WriteWait(Ft_Gpu_Hal_Context_t *host,Ft_Fifo_t *pFifo,ft_uint8_t *buffer,ft_uint32_t Numbyte)
{
	ft_uint32_t TotalBytes = Numbyte,currchunk = 0,FreeSpace;
	ft_uint8_t *pbuff = buffer;
	/* blocking call, manage to check for the error case and break in case of error */
	while(TotalBytes > 0)
	{
		currchunk = TotalBytes;
		FreeSpace = Ft_Fifo_GetFreeSpace(host,pFifo);
		if(currchunk > FreeSpace)
		{
			currchunk = FreeSpace;
		}

		Ft_Fifo_Write(host,pFifo,pbuff,currchunk);
		pbuff += currchunk;
		TotalBytes -= currchunk;
	}
}

//get the free space in the fifo - make sure the return value is maximum of (LENGTH - 4)
ft_uint32_t Ft_Fifo_GetFreeSpace(Ft_Gpu_Hal_Context_t *host,Ft_Fifo_t *pFifo)
{
	ft_uint32_t FreeSpace = 0;

	Ft_Fifo_Update(host,pFifo);

	if(pFifo->fifo_wp >= pFifo->fifo_rp)
	{
		FreeSpace = pFifo->fifo_len - pFifo->fifo_wp + pFifo->fifo_rp;
	}
	else
	{
		FreeSpace = pFifo->fifo_rp - pFifo->fifo_wp;
	}

	if(FreeSpace >= 4)
	{
		FreeSpace -= 4;//make sure 1 word space is maintained between rd and wr pointers
	}
	return FreeSpace;
}

ft_uint32_t Ft_Gpu_CurrentFrequency(Ft_Gpu_Hal_Context_t *host)
{
    ft_uint32_t t0, t1;

    t0 = Ft_Gpu_Hal_Rd32(host,REG_CLOCK); /* t0 read */
               
	_delay_us(15625);

    t1 = Ft_Gpu_Hal_Rd32(host,REG_CLOCK); /* t1 read */
    return ((t1 - t0) * 64); /* bitshift 6 places is the same as multiplying 64 */
}

ft_int32_t Ft_Gpu_ClockTrimming(Ft_Gpu_Hal_Context_t *host,ft_int32_t LowFreq)
{
   ft_uint32_t f;
   ft_uint8_t i;

  /* Trim the internal clock by increase the REG_TRIM register till the measured frequency is within the acceptable range.*/
   for (i=0; (i < 31) && ((f= Ft_Gpu_CurrentFrequency(host)) < LowFreq); i++)
   {
	   Ft_Gpu_Hal_Wr8(host,REG_TRIM, i);  /* increase the REG_TRIM register value automatically increases the internal clock */
   }

   Ft_Gpu_Hal_Wr32(host,REG_FREQUENCY,f);  /* Set the final frequency to be used for internal operations */

   return f;
}


