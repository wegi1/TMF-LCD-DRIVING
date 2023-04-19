Introduction: 
The aud_cvt.exe is the utility which runs on windows to extract the signed 16 bit PCM 
from wave(Microsoft format) file and convert into FT800 audio format data. 

Note: 
The utility assumes the input wave file contains Mono signed 16 bit PCM data. If user has 
other format data, please use other tool, such as Audacity ,to convert it into signed 
16 bit PCM data in advance. 




The usage is : 
    aud_cvt  -i  inputfilename  -f  format

         format is as follow:
            0 : 8 Bit signed PCM(LINEAR) [default]
            1 : 8 Bit u-Law
            2 : 4 Bit IMA ADPCM 
			
Example : 
    aud_cvt  -i  text2speech_16BitPCM_22050SampleRate.wav  -f 1
	
	
Caution: 
   For format option 1 and 2 , saying 8 bit signed PCM and 8 Bit u-Law, 
we highly recommend to use Audacity to convert.  

   For format 0, saying 4 Bit IMA ADPCM,  FT800 assumes the first sample is
   in bit 0 to 3, while second sample in bit 4 to 7 of the first byte. It is 
   little endian. 
   
   For all the format, the output data are 8 bytes aligned by padding zero if not. 
   If users convert the audio data format by Audacity, please make sure the output 
   data is 8 bytes aligned as per FT800 requirement by padding necessary zero. 


There are 4 types of files are generated under output folder: 
*.raw  : The binary format of converted file, which can be downloaded into FT800 graphics memory directly. 
*.rawh : The header file of converted file, which is in text representation.  
    Programmer can include this file into their program and build it into final binary. 

*.bin  : The compressed binary format of converted file in ZLIB algorithm. Programmer needs download it 
   into graphics memory of FT800 and use CMD_INFLATE to inflate them before using it. 
*.binh : The header file of compressed binary format,which is in text representation of *.bin. 
   Programmer can include this file into their program and build it into final binary. Please note 
   that it is still needed to be inflated by using CMD_INFLATE of FT800
   