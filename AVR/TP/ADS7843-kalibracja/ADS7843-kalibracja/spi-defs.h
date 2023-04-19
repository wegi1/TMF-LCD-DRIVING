/*
 * spi_defs.h
 *
 * Created: 2014-02-24 21:18:34
 *  Author: tmf
 */


#ifndef SPI_DEFS_H_
#define SPI_DEFS_H_

//Pod³¹czenie LCD:
// PC4 - RESET
// PC6 - CS
// PC7 - RS
//Pod³¹czenie TC:
// PC5 - INT z uk³adu TC
// PC0 - TPCS
//USART-SPI:
// PC1 - SCK
// PC2 - MISO
// PC3 - MOSI

//#define SD_CS     PIN0_bm       //Sygna³ CS pamiêci karty SD
#define TP_CS       PIN0_bm       //Sygna³ CS kontrolera TP
#define TP_INT      PIN5_bm       //Sygna³ PENIRQ z kontrolera TP
#define LCD_CS      PIN6_bm       //Sygna³ CS kontrolera LCD
#define LCD_RS      PIN7_bm       //Sygna³ RS kontrolera LCD
#define LCD_RESET   PIN4_bm       //Sygna³ RESET kontrolera LCD
#define LCD_SCK     PIN1_bm       //Sygna³ SCK
#define LCD_MISO    PIN2_bm       //Sygna³ MISO
#define LCD_MOSI    PIN3_bm       //Sygna³ MOSI
#define LCD_USART   USARTC0       //Port USART wykorzystywany do komunikacji z LCD
#define LCD_PORT    PORTCFG_VP02MAP_PORTC_gc        //Port do którego pod³¹czony jest kontroler
#define LCD_DMA_TRIG_SRC DMA_CH_TRIGSRC_USARTC0_DRE_gc;

#endif /* SPI_DEFS_H_ */