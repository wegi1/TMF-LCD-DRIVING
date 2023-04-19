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



Introduction: 
This fnt_cvt.exe is a utility which runs on Windows to extract characters from the 
font file into FT80X / FT81X specific font metric block as well as the raw bitmap data. 
The characters to be converted are specified in UTF-8 encoding file or ASCII printable 
code set from 32 to 126(both inclusive).

This tool will generate the metric block file as well as L1, L2, L4, L8 format bitmap data. 
The output is one 148 bytes metric block followed by the raw bitmap data. 

The output data of this utility is prepared for 1 bitmap handle.


Enclosed files:
ANSI_32_126.txt - ANSI file when converting the ASCII printable character set.
fnt_cvt.exe - Font conversion utility
README.txt - This file
SampleApp.c - Example HAL source file, see the lower section for usage.
Tuffy.ttf - Example font file


The usage is : 
    fnt_cvt.exe -i FontFileName -s pointsize [-u utf8_file] | [[-t ascii_file] | [-a]] [-c cmds] [-d FT800_address]

Arguments list: 	
'-i' :	Mandatory argument. The input file shall be located at the 
same folder of utility, otherwise fnt_cvt will search the windows system font directory 
for the font file.Users are encouraged to test the different font file format listed at 
http://www.freetype.org/freetype2/#features,although it is not fully tested yet. 

'-s' :  Optional argument. The width in pixel of font characters to be converted. If 
this argument is not present, size 12 is assumed. 

'-u' :  Mandatory argument if the input file is encoding in UTF-8. The utf8_file specifies 
the file name. fnt_cvt.exe will read this file and convert the characters in this file. Users 
can copy the characters to be converted into this file but ensure the file is encoded in UTF-8.
The number of characters in the file shall be not more than 127. 

'-a':   The utility will use the default "ANSI_32_126.txt"  to convert the ASCII printable character set , i.e.
from 32 to 126. There is no input file required but the "ANSI_32_126.txt" file must be in the same directory as the utility.

'-t':    The utility willl use the user specified  ANSI file to convert the ASCII printable character set , i.e.
from 32 to 126.

'-c':  Optional argument.  Valid options are:
	setfont - Generate FT80X compatible font metric table, the default option.
	setfont2 - Generate FT81X compatible font metric table for Cmd_Setfont2 command.  A subset of the printable ASCII characters can be specified in the input file for -u or -t argument.  NOTE: the generated files with this option are not compatible with FT80X.  

'-d':   Optional argument. This argument defines the start address of metric block in FT80X/FT81X RAM.    
If it is not present, fnt_cvt will assume RAM_G is the starting address. Because the bitmap raw data
is following the metric block, the bitmap raw data address is starting from FT800_address + 148

Example : 
1. Convert the ASCII printable characters to Arial bold font and assume the address of output data 
is from RAM_G + 1000 in FT80X / FT81X
    fnt_cvt -i arialbd.ttf -s 24 -a -d 1000
	
2. Convert the characeters in utf8_file into arial bold font and assume the address of output data 
is from RAM_G + 1000 in FT80X / FT81X
	fnt_cvt -i arialbd.ttf -s 24 -u utf8_file -d 1000

3. Convert the utf8_file with only the lower case a to z and the whole data file will be loaded to RAM_G + 1000 in FT81X
	fnt_cvt -i arialbd.ttf -s 24 -u utf8_file -c setfont2 -d 1000
		
		
Output folder:  
All the files generated are under folder FontFileName_utf8_file_pointsize of current working directory.

Note: 
1. "L1" "L2" "L4" "L8" are output folders for different formats. Each folder contains the *.raw file and *.rawh file. 

*.rawh file is in C header format and explains the format of binary in *.raw. 
*.raw is the raw data in binary.
*.rtf is the file defining the character and index mapping relationship, which is encoded in UTF-8 and can be 
openned by word pad application of Windows.

2. Metric block and address of Bitmap raw data.
The metric block is totally 148 bytes and its format can be found in programmer guide. 
Please note the 4 bytes of metric block from offset 144 specifies the address of bitmap 
raw data of characters in FT800 RAM_G(in Little Endian).  We call it AddressInMetricBlk below.

AddressInMetricBlk is determined in different way at '-u' and '-a' argument case: 

'-u':  AddressInMetricBlk = FT800_address + RAM_G + 148 - stride * Height * 1

'-a':  AddressInMetricBlk = FT800_address + RAM_G + 148 - stride * Height * 32

If "setfont2" is specified for the -c optional argument then the AddressInMetricBlk is simply the specified 
FT800_address or 0 if it was not specified. 


3. In the 'UTF-8' format,  to address the character of font, the valid index range is from 1 to 127
   In the 'ASCII' format,  to address the character of font, the valid index range is from 32 to 127
 Pleasse see the *.rtf file for each character 's index. 

4. Please make sure the characters in utf8_file is supported in the FontFileName. Otherwise, it may not 
output the desired font data. 

5. The *.jpg and *.png is in L8 format and viewable at windows photo viewer application. 

6. The file "SampleApp.c" contains one function which tests the output data of utility: 
 fnt_cvt -i simfang.ttf -u FangSong_Chinese.txt -s 30 -d 1000
 Users shall copy the function to the Sample Application Project and try to call it to see 
 the effect. FT800 shows 20 Fang Song Tradinitional Chinese characters. 
 
 The file "FangSong_Chinese.txt" is encoded as UTF-8 and used for user to test. 
 
7. The file "ansi_32_126.txt" is the default file used by utility for '-a' argument. 
   DONOT delete it.


Release history:
0.4 : Added L2 format and FT81X setfont command support.
0.3.1: Fix the deployment issue and introduce the new application icon. 
0.3:   Solve the issue of vertical alignment issue 
0.2.1: Solve the issue that cannot find resource file 
       if font file is under windows font directory only.  
0.2: Fix the issue true type font file has to be installed in windows font directory. 
0.1: Initial Release
	


