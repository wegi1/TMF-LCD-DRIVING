/*
 * spi_defs.h
 *
 * Created: 2014-02-24 21:18:34
 *  Author: tmf
 */


#ifndef SPI_DEFS_H_
#define SPI_DEFS_H_

//Pod��czenie LCD:
// PC4 - RESET
// PC6 - CS
// PC7 - RS
//Pod��czenie TC:
// PC5 - INT z uk�adu TC
// PC0 - TPCS
//USART-SPI:
// PC1 - SCK
// PC2 - MISO
// PC3 - MOSI

#define SD_CS    PIN0_bm       //Sygna� CS pami�ci karty SD
#define TP_CS   PIN2_bm        //Sygna� CS kontrolera TP
#define LCD_CS   PIN6_bm       //Sygna� CS kontrolera LCD
#define LCD_RS   PIN7_bm       //Sygna� RS kontrolera LCD
#define LCD_RESET   PIN4_bm    //Sygna� RESET kontrolera LCD
#define LCD_SCK  PIN1_bm       //Sygna� SCK
#define LCD_MISO PIN2_bm       //Sygna� MISO
#define LCD_MOSI PIN3_bm       //Sygna� MOSI
#define LCD_USART USARTC0      //Port USART wykorzystywany do komunikacji z LCD
#define LCD_PORT  PORTC        //Port do ktorego pod��czony jest kontroler

#endif /* SPI_DEFS_H_ */