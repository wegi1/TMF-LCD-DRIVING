#ifndef _SAMPLEAPP_H_
#define _SAMPLEAPP_H_


#define SAMAPP_Lenaface40_SIZE (2769)
#define SAMAPP_Mandrill256_SIZE (14368)
#define SAMAPP_Roboto_BoldCondensed_12_SIZE (19348)



/* sample app structure definitions */
typedef struct SAMAPP_Bitmap_header
{
	ft_uint8_t Format;
	ft_int16_t Width;
	ft_int16_t Height;
	ft_int16_t Stride;
	ft_int32_t Arrayoffset;
}SAMAPP_Bitmap_header_t;

extern SAMAPP_Bitmap_header_t  SAMAPP_Bitmap_RawData_Header[];

extern FT_PROGMEM ft_prog_uchar8_t SAMAPP_Bitmap_RawData[];
extern FT_PROGMEM ft_prog_uchar8_t Lenaface40[];
extern FT_PROGMEM ft_prog_uchar8_t Font16[];
extern FT_PROGMEM ft_prog_uchar8_t Mandrill256[];
extern FT_PROGMEM ft_prog_uchar8_t Roboto_BoldCondensed_12[];

ft_void_t SAMAPP_fadeout();
ft_void_t SAMAPP_fadein();
ft_int16_t SAMAPP_qsin(ft_uint16_t a);
ft_int16_t SAMAPP_qcos(ft_uint16_t a);
/* Sample app APIs for graphics primitives */

ft_void_t	SAMAPP_GPU_Points();
ft_void_t	SAMAPP_GPU_Lines();
ft_void_t	SAMAPP_GPU_Rectangles();
ft_void_t	SAMAPP_GPU_Bitmap();
ft_void_t	SAMAPP_GPU_Fonts();
ft_void_t	SAMAPP_GPU_Text8x8();
ft_void_t	SAMAPP_GPU_TextVGA();
ft_void_t	SAMAPP_GPU_Bargraph();
ft_void_t	SAMAPP_GPU_LineStrips();
ft_void_t	SAMAPP_GPU_EdgeStrips();
ft_void_t	SAMAPP_GPU_Scissor();
ft_void_t	SAMAPP_GPU_FtdiString();
ft_void_t	SAMAPP_GPU_StreetMap();
ft_void_t	SAMAPP_GPU_AdditiveBlendText();
ft_void_t	SAMAPP_GPU_MacroUsage();
ft_void_t	SAMAPP_GPU_AdditiveBlendPoints();

/* Sample app APIs for widgets */
ft_void_t SAMAPP_CoPro_Widget_Logo();
ft_void_t SAMAPP_CoPro_Widget_Calibrate();
ft_void_t SAMAPP_CoPro_AppendCmds();
ft_void_t SAMAPP_CoPro_Inflate();
ft_void_t SAMAPP_CoPro_Loadimage();
ft_void_t SAMAPP_CoPro_Widget_Button();
ft_void_t SAMAPP_CoPro_Widget_Clock();
ft_void_t SAMAPP_CoPro_Widget_Guage();
ft_void_t SAMAPP_CoPro_Widget_Gradient();
ft_void_t SAMAPP_CoPro_Widget_Keys();
ft_void_t SAMAPP_CoPro_Widget_Progressbar();
ft_void_t SAMAPP_CoPro_Widget_Scroll();
ft_void_t SAMAPP_CoPro_Widget_Slider();
ft_void_t SAMAPP_CoPro_Widget_Dial();
ft_void_t SAMAPP_CoPro_Widget_Toggle();
ft_void_t SAMAPP_CoPro_Widget_Text();
ft_void_t SAMAPP_CoPro_Widget_Number();
ft_void_t SAMAPP_CoPro_Widget_Spinner();
ft_void_t SAMAPP_CoPro_Screensaver();
ft_void_t SAMAPP_CoPro_Snapshot();
ft_void_t SAMAPP_CoPro_Sketch();
ft_void_t SAMAPP_CoPro_Matrix();
ft_void_t SAMAPP_CoPro_Setfont();
ft_void_t SAMAPP_CoPro_Track();

ft_void_t SAMAPP_PowerMode();
ft_void_t SAMAPP_BootupConfig();

#endif /* _SAMPLEAPP_H_ */

/* Nothing beyond this */








