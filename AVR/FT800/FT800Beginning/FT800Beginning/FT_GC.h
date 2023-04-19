/*
 * FT_GC.h
 *
 * Created: 2015-01-02 20:21:17
 *  Author: tmf
 */ 


#ifndef FT_GC_H_
#define FT_GC_H_

/* Definitions used for debug. Uncomment the below to enable debug from graphics controller library */
#define FT_GCDEBUG0 255 //switchoff debug
#define FT_GCDEBUG1 1 //most critical debug information
#define FT_GCDEBUG2 2 //mid critical debug information
#define FT_GCDEBUG3 3 //least critical debug information

/* Change the below statement wrt debug criticality */
#define FT_GCDEBUG FT_GCDEBUG0

/* Version number of FT_GC */
#define FT_GC_MAJOR				1
#define FT_GC_MINOR				2
#define FT_GC_BUILD				0

/* FT_GC status enum - used for api return type, error type etc */
typedef enum FT_Status
{	
	/* Common enums */
	FT_OK = 0,
	FT_ERROR = 1,
	FT_WARNING = 2,
	FT_ERROR_INIT = 3,
	FT_ERROR_CHIPID = 4,
	
	/* Library related enums */
	FT_ERROR_NOPINASSIGNED = 50,
}FT_Status;

/* Audio coprocessor related enums */
typedef enum FT_AEStatus
{		
	FT_AE_OK = 0,	
	FT_AE_ERROR_FORMAT = 1,
	FT_AE_ERROR_SAMPLINGFREQ_OUTOFRANGE = 2,	//assert for boundary
	FT_AE_PLAYBACK_STOPPED = 3,
	FT_AE_PLAYBACK_CONTINUE = 4,
}FT_AEStatus;

/* Status enums for graphics engine */
typedef enum FT_GEStatus
{	
	FT_GE_OK = 0,	
	FT_GE_BUSY = 1,
	FT_GE_FINISHED = 2,
	
	/* Graphics related error enums */
	FT_GE_ERROR_INVALID_PRIMITIVE = 20,
	FT_GE_ERROR_INVALID_BITMAP_FORMAT = 21,
	FT_GE_ERROR_INVALID_BITMAP_HANDLENUM = 22,
	FT_GE_ERROR_VERTEX_OUTOFRANGE = 23,

	/* Coprocessor related enums */
	FT_GE_ERROR = 50,						//undefined error
	FT_GE_ERROR_JPEG = 51,					//erranious jpeg data
	FT_GE_ERROR_DEFLATE = 52,				//erranious deflated data
	FT_GE_ERROR_DISPLAYLIST_OVERFLOW = 53,	//DL buffer overflow
	FT_GE_ERROR_INVALID_WIDGET_PARAMS = 54,	//invalid input parameters - out of bound
	
	/* Display parameters error */
	FT_GE_ERROR_DISPLAYPARAMS = 100,//error in the display parameters
}FT_GEStatus;

/* Touch coprocessor related enums */
typedef enum FT_TEStatus
{	
	FT_TE_OK = 0,	
	FT_TE_ERROR_RZTHRESHOLD = 1,		//threshold out of bound
	FT_TE_ERROR_FILTERPARAM = 2,		//filter out of bound
	FT_TE_ERROR_MODE = 3,				//mode out of range
	FT_TE_ERROR_INVALIDPARAM = 4,		//generic invalid param
}FT_TEStatus;

 
/************************************************************************************************************************************************************
Display parameters used for various options are

FT_GC_DisplayResolution			Width 	Height	Swizzle	Polarity	PClk	HCycle	Hoffset		Hsync0		Hsync1		VCycle	Voffset		Vsync0	Vsync1	
FT_DISPLAY_QVGA_320x240   		320		240		3		0			8		408		70			0			10			263			13		0		2
FT_DISPLAY_WQVGA_480x272		480		272		0		1			5		548		43			0			41			292			12		0		10
*************************************************************************************************************************************************************/
/*
typedef enum FT_GC_DispRes
{
	FT_DISPLAY_QVGA_320x240 = 0,
	FT_DISPLAY_WQVGA_480x272 = 1,
}FT_GC_DispRes;

//Zdefiniuj globalnie jeden z powy¿szych symboli, odpowiadaj¹cy u¿ywanemu LCD

*/
//#define		FT_DISPLAY_QVGA_320x240		0UL
//#define		FT_DISPLAY_WQVGA_480x272 	1UL
/* Structure definitions */

typedef struct sTagXY
{
	int16_t y;		//y coordinate of touch object
	int16_t x;		//x coordinate of touch object
	uint16_t tag;	//TAG value of the object
}sTagXY;

typedef struct sTrackTag
{
	uint16_t tag;	//TAG value of the object
	uint16_t track;	//track value of the object	
}sTrackTag;

/* FT80x font table structure */
/* Font table address in ROM can be found by reading 32bit value from FT_FONT_TABLE_POINTER location. */
/* 16 font tables are present at the address read from location FT_FONT_TABLE_POINTER */
typedef struct FT_Fonts
{
	/* All the values are in bytes */
	/* Width of each character font from 0 to 127 */
	uint8_t	FontWidth[FT_NUMCHAR_PERFONT];
	/* Bitmap format of font wrt bitmap formats supported by FT800 - L1, L4, L8 */
	uint32_t	FontBitmapFormat;
	/* Font line stride in FT800 ROM */
	uint32_t	FontLineStride;
	/* Font width in pixels */
	uint32_t	FontWidthInPixels;
	/* Font height in pixels */
	uint32_t	FontHeightInPixels;
	/* Pointer to font graphics raw data */
	uint32_t	PointerToFontGraphicsData;
}FT_Fonts_t;


//W³¹cz wyœwietlacz - ustaw GPIO07
void DisplayOn()
{
	FT800_Write(REG_GPIO,(1 << FT_GPIO7) | FT800_Read(REG_GPIO));
	
}

//Wy³¹cz wyœwietlacz
void DisplayOff()
{
	FT800_Write(REG_GPIO,(~(1 << FT_GPIO7)) & FT800_Read(REG_GPIO));
}



#endif /* FT_GC_H_ */