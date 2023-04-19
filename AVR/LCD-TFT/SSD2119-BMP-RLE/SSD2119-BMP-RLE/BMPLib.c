/*
 * BMPLib.c
 *
 * Created: 2014-03-28 21:02:39
 *  Author: tmf
 */ 

#include <stdint.h>
#include "GFXDrv.h"
#include "ssd2119.h"
#include "xmega-spi.h"

typedef struct             //Nag³ówek pliku BMP
{  
 uint16_t bfType;          //Magiczna liczba
 uint32_t bfSize;          //D³ugoœæ pliku  
 uint32_t bfReserved;      //Zarezerwowane
 uint32_t bfOffBits;       //Ofset do danych
} BITMAPFILEHEADER;  
  
#define BF_TYPE 0x424D     //Magiczna liczba -BM dla pliku BMP  
  
typedef struct                   //Nag³ówek zawieraj¹cy informacje o obrazie
{  
    uint32_t   biHdrSize;        //D³ugoœæ nag³ówka
    int32_t    biWidth;          //Szerokoœæ obrazu  
    int32_t    biHeight;         //Wysokoœæ obrazu  
    uint16_t   biPlanes;         //Liczba planów  
    uint16_t   biBitCount;       //Iloœæ bitów na piksel  
    uint32_t   biCompression;    //Typ kompresji  
    uint32_t   biSizeImage;      //Rozmiar obrazu w bajtach  
    uint32_t   biXPelsPerMeter;  //Rozdzielczoœæ pozioma (pikseli/m)
    uint32_t   biYPelsPerMeter;  //Rozdzielczoœæ pionowa (pikseli/m)
    uint32_t   biClrUsed;        //Liczba u¿ytych kolorów
    uint32_t   biClrImportant;   //Liczba istotnych kolorów  
} BITMAPINFOHEADER;  
  
 
#define BI_RGB       0             //Brak kompresji
#define BI_RLE8      1             //RLE 8 bitów/piksel 
#define BI_RLE4      2             //RLE 4 bity na piksel 
#define BI_BITFIELDS 3             //Obraz RGB 

typedef struct                     // Tablica kolorów  
{  
    uint8_t  rgbBlue;          // Niebieski
    uint8_t  rgbGreen;         // Zielony  
    uint8_t  rgbRed;           // Czerwony  
    uint8_t  rgbReserved;      // Zarezerwowane/alfa  
} RGBQUAD;  
  
typedef struct                       //Dane o bitmapie
{  
	BITMAPFILEHEADER bmiFileHdr;     //Nag³ówek BMP
    BITMAPINFOHEADER bmiHeader;      //Nag³ówek  
    RGBQUAD          bmiColors[256]; //Mapa kolorów
} BITMAPINFO; 
	
void LCD_BMP(const uint8_t __memx *Image, uint16_t Xpos, uint8_t Ypos)
{
    const uint8_t __memx *ImageData;  //WskaŸnik do danych binarnych o obrazie
	const BITMAPINFO __memx *ImageHDR=(const BITMAPINFO __memx*)Image;
	const uint8_t __memx *rgb=(const __memx uint8_t*)ImageHDR;
	rgb+=__builtin_offsetof(BITMAPINFO, bmiColors);
	const RGBQUAD __memx *rgbtable=(const RGBQUAD __memx *)rgb;

    uint16_t LineWidth;               //Szerokoœæ bitmapy w bajtach (wielokrotnoœæ 4 bajtów)
	uint16_t DisplayWidth;            //Szerokoœæ bitmapy na ekranie (jeœli wiêksza ni¿ pozosta³a szerokoœæ ekranu)
    uint8_t DisplayHeight;            //Wysokoœæ bitmapy na ekranie

    void DecompressRGB()
	{
		for(uint8_t dy=0; dy < DisplayHeight; dy++)
		{
			const uint8_t __memx *tmpimgdata=ImageData;
			for(uint16_t dx=0; dx <= DisplayWidth; dx++)
			{
				uint8_t index=*tmpimgdata++;    //Bajt zawiera indeks do tabeli kolorów
				ssd2119_SendDataByte(rgbtable[index].rgbRed);    //Wyœlij poszczególne sk³¹dowe koloru
				ssd2119_SendDataByte(rgbtable[index].rgbGreen);
				ssd2119_SendDataByte(rgbtable[index].rgbBlue);
			}
			ImageData+=LineWidth;  //Kolejna linia obrazu
		}
	}
	
	void SkipLinesRLE8(uint8_t lines)  //Funkcja pomija wskazan¹ liczbê linii skompresowanej bitmapy
	{
		while(lines)
		{
			uint8_t rlecnt=*ImageData++;       //Ile powtórzeñ
			if(rlecnt) ImageData++;            //Pomijamy powtarzany bajt
			else
			{
				uint8_t unique=*ImageData++;       //Kod escape
				if(unique >= 2) ImageData+=((unique + 1) & 0b11111110); //Liczba niepowtarzaj¹cych siê bajtów wyrównana do granicy s³owa
				else if(unique == 0) lines--;
			}
		}
	}
	
	void DecompressRLE8()
	{
		for(uint8_t dy=DisplayHeight; dy > 0; dy--)
		{
			uint16_t pixcnt=0;   //Licznik wyœwietlonych pikseli w linii
			uint8_t cnt;         //Licznik powtórzonych bajtów
			
			while(pixcnt <= DisplayWidth)
			{
				uint8_t rlecnt=*ImageData++;  //Ile powtórzeñ
				if(rlecnt)
				{
					uint8_t pixel=*ImageData++;          //Powtarzany piksel
					if((pixcnt + rlecnt) > DisplayWidth) //Czy mieœcimy siê na LCD?
					 {
						 rlecnt=DisplayWidth-pixcnt+1;   //Jeœli nie to ile pikseli mo¿na wyœwietliæ?
						 while(rlecnt--)
						 {
							ssd2119_SendDataByte(rgbtable[pixel].rgbRed);    //Wyœlij poszczególne sk³adowe koloru
							ssd2119_SendDataByte(rgbtable[pixel].rgbGreen);
							ssd2119_SendDataByte(rgbtable[pixel].rgbBlue);
						 }
						 SkipLinesRLE8(1);         //Pomijamy piksele poza matryc¹
						 pixcnt=-1;
					 } else
					 {
						pixcnt+=rlecnt;     //Dodajemy liczbê powtórzeñ piksela
						while(rlecnt--)
						{
							ssd2119_SendDataByte(rgbtable[pixel].rgbRed);    //Wyœlij poszczególne sk³adowe koloru
							ssd2119_SendDataByte(rgbtable[pixel].rgbGreen);
							ssd2119_SendDataByte(rgbtable[pixel].rgbBlue);
						}
					 }
				} else
				{    //Mamy ci¹g niepowtarzaj¹cych siê bajtów
					uint8_t unique=*ImageData++;  //Ile bajtów siê nie powtarza?
					switch(unique)
					{
						case 0 :  pixcnt=-1;  //Znak koñca bie¿¹cej linii
						          LCD_SetPosition(Xpos, Ypos + dy);   //Pozycja bitmapy - przejdŸ do kolejnej linii
						          ssd2119_SendCmd(ssd2119_Write_to_GRAM);  //Kontynuuj zapis do GRAM
						          break;
						case 1 :  return;  //Koniec bitmapy
						          break;
						case 2 :  break;  //Funkcja przejœcia do wskazanej pozycji - nie implementujemy jej
						default:  
								  cnt=0;
						          while(cnt < unique)
						          {
							       pixcnt++; cnt++;
							       uint8_t pixel=*ImageData++; //Unikalny piksel
							       ssd2119_SendDataByte(rgbtable[pixel].rgbRed);    //Wyœlij poszczególne sk³adowe koloru
							       ssd2119_SendDataByte(rgbtable[pixel].rgbGreen);
							       ssd2119_SendDataByte(rgbtable[pixel].rgbBlue);
								   if(pixcnt > DisplayWidth) 
								   {
									   unique=((unique + 1) & 0b11111110);
									   ImageData+=(unique-cnt); //Pomiñ pozosta³e bajty
									   SkipLinesRLE8(1);
									   pixcnt=-1;
									   break;
								   }
						          }
						          if((unique & 1) && (pixcnt!=-1)) ImageData++;  //Na koñcu mamy zawsze bajt o wartoœci 0
						          break;
					}
				}
			}
		}		
	}
	
	void SkipLinesRLE4(uint8_t lines)  //Funkcja pomija wskazan¹ liczbê linii skompresowanej bitmapy
	{
		while(lines)
		{
			uint8_t rlecnt=*ImageData++;       //Ile powtórzeñ
			if(rlecnt) ImageData++;            //Pomijamy powtarzany bajt
			else
			{
				uint8_t unique=*ImageData++;       //Kod escape
				if(unique >= 2) ImageData+=(((unique+3)/2) & 0b11111110); //Liczba niepowtarzaj¹cych siê bajtów wyrównana do granicy s³owa
				else if(unique == 0) lines--;
			}
		}
	}
	
	void  DecompressRLE4()
	{
		for(uint8_t dy=DisplayHeight; dy > 0; dy--)
		{
			uint16_t pixcnt=0;   //Licznik wyœwietlonych pikseli w linii
			uint8_t cnt;         //Licznik powtórzonych bajtów
			
			while(pixcnt <= DisplayWidth)
			{
				uint8_t rlecnt=*ImageData++;  //Ile powtórzeñ
				if(rlecnt)
				{
					uint8_t pixel=*ImageData++;          //Powtarzany piksel
					if((pixcnt + rlecnt) > DisplayWidth) //Czy mieœcimy siê na LCD?
					{
						rlecnt=DisplayWidth-pixcnt+1;   //Jeœli nie to ile pikseli mo¿na wyœwietliæ?
						while(rlecnt--)
						{
							pixel=__builtin_avr_swap(pixel);                  //Zmieñ kolejnoœæ tetrad
							uint8_t nibble=pixel & 0x0f;                      //M³odsza tetrada okreœla indeks do tablicy kolorów
							ssd2119_SendDataByte(rgbtable[nibble].rgbRed);    //Wyœlij poszczególne sk³adowe koloru
							ssd2119_SendDataByte(rgbtable[nibble].rgbGreen);
							ssd2119_SendDataByte(rgbtable[nibble].rgbBlue);
						}
						SkipLinesRLE4(1);         //Pomijamy piksele poza matryc¹
						pixcnt=-1;
					} else
					{
						pixcnt+=rlecnt;     //Dodajemy liczbê powtórzeñ piksela
						while(rlecnt--)
						{
							pixel=__builtin_avr_swap(pixel);                  //Zmieñ kolejnoœæ tetrad
							uint8_t nibble=pixel & 0x0f;                      //M³odsza tetrada okreœla indeks do tablicy kolorów
							ssd2119_SendDataByte(rgbtable[nibble].rgbRed);    //Wyœlij poszczególne sk³adowe koloru
							ssd2119_SendDataByte(rgbtable[nibble].rgbGreen);
							ssd2119_SendDataByte(rgbtable[nibble].rgbBlue);
						}
					}
				} else
				{    //Mamy ci¹g niepowtarzaj¹cych siê bajtów
					uint8_t unique=*ImageData++;  //Ile bajtów siê nie powtarza?
					switch(unique)
					{
						case 0 :  pixcnt=-1;  //Znak koñca bie¿¹cej linii
						          LCD_SetPosition(Xpos, Ypos + dy);   //Pozycja bitmapy - przejdŸ do kolejnej linii
						          ssd2119_SendCmd(ssd2119_Write_to_GRAM);  //Kontynuuj zapis do GRAM
						          break;
						case 1 :  return;  //Koniec bitmapy
						          break;
						case 2 :  break;  //Funkcja przejœcia do wskazanej pozycji - nie implementujemy jej
						default:  cnt=0;
						          while(cnt < unique)
						          {
							         uint8_t pixel=pixel;                   //Unikamy ostrze¿enia o potencjalnie niezainicjalizowanej zmiennej
									 if((cnt & 1) == 0) pixel=*ImageData++; //Za³aduj dane o dwóch pikselach
									 pixel=__builtin_avr_swap(pixel);                 //Zmieñ kolejnoœæ tetrad
									 uint8_t nibble=pixel & 0x0f;                     //M³odsza tetrada okreœla indeks do tablicy kolorów
							         ssd2119_SendDataByte(rgbtable[nibble].rgbRed);   //Wyœlij poszczególne sk³adowe koloru
							         ssd2119_SendDataByte(rgbtable[nibble].rgbGreen);
							         ssd2119_SendDataByte(rgbtable[nibble].rgbBlue);
									 pixcnt++; cnt++;
							         if(pixcnt > DisplayWidth)
							         {
										 ImageData+=(unique - cnt + 1)/2; 
										 if(((unique & 0b11) == 0b01) || ((unique & 0b11) == 0b10)) ImageData++;
								         SkipLinesRLE4(1);
								         pixcnt=-1;
								         break;
							         }
						          }
						          if((((unique & 0b11) == 0b01) || ((unique & 0b11) == 0b10)) && (pixcnt!=-1)) ImageData++;  //Na koñcu mamy zawsze bajt o wartoœci 0
						          break;
					}
				}
			}
		}
	}
	 
    ImageData=Image;    //WskaŸnik do danych bitmapy
	ImageData+=ImageHDR->bmiFileHdr.bfOffBits;     //Wylicz adres danych na podstawie przemieszczenia w nag³ówku
	LineWidth=ImageHDR->bmiHeader.biWidth;
	LineWidth=(LineWidth + 3) & (~0x03);           //Zaokr¹glij w górê szerokoœæ w bajtach, tak aby by³a podzielna przez 4

	DisplayWidth=ImageHDR->bmiHeader.biWidth;
	if((LCD_GetMaxX()-Xpos) < ImageHDR->bmiHeader.biWidth)    //Bitmapa wystaje poza ekran
		DisplayWidth=LCD_GetMaxX()-Xpos-1;

	DisplayHeight=ImageHDR->bmiHeader.biHeight;
	if((LCD_GetMaxY()-Ypos) < ImageHDR->bmiHeader.biHeight)    //Bitmapa wystaje poza ekran
		DisplayHeight=LCD_GetMaxY()-Ypos-1;
	
	LCD_SetWindow(Xpos, Ypos, Xpos + DisplayWidth, Ypos + DisplayHeight);
	LCD_SetPosition(Xpos, Ypos + DisplayHeight);   //Pozycja bitmapy
	
	ssd2119_SendCmdWithData(ssd2119_Entry_Mode, (ssd2119_EntryMode_Reg){.DFM=0b10, .DenMode=1, .TY=0b01, .ID=0b01, .AM=0}.word); //Zapisujemy od do³u - tak zapisywane s¹ pliki BMP
	ssd2119_SendCmd(ssd2119_Write_to_GRAM);        //Zapis danych o bitmapie pod wskazan¹ pozycjê

    switch (ImageHDR->bmiHeader.biCompression)
	{
		case BI_RGB:       ImageData+=LineWidth*(ImageHDR->bmiHeader.biHeight - DisplayHeight); //Poniewa¿ bitmapa zapisana jest od do³u musimy pomin¹æ odpowiedni¹ liczbê "dolnych" linii
		                   DecompressRGB(); break;    //zdekompresuj nieskompresowan¹ mapê bitow¹
		case BI_RLE8:      SkipLinesRLE8(ImageHDR->bmiHeader.biHeight - DisplayHeight);           //Pomiñ podan¹ liczbê linii w pliku BMP
		                   DecompressRLE8(); break;   //zdekompresuj bitmapê skompresowan¹ RLE8
		case BI_RLE4:      SkipLinesRLE4(ImageHDR->bmiHeader.biHeight - DisplayHeight);           //Pomiñ podan¹ liczbê linii w pliku BMP
		                   DecompressRLE4(); break;   //zdekompresuj bitmapê skompresowan¹ RLE4
		default:           break;            //Nieznany typ kompresji wiêc nic nie robimy
	};
	
	_delay_loop_1(2);   //Zaczekaj na opró¿nienie bufora nadajnika USART (2*3 takty + coœtam z poprzedzaj¹cego kodu)
	ssd2119_CS(1);
	ssd2119_SendCmdWithData(ssd2119_Entry_Mode, (ssd2119_EntryMode_Reg){.DFM=0b10, .DenMode=1, .TY=0b01, .ID=0b11, .AM=0}.word); //Przywróæ domyœlne parametry zapisu do GRAM
}
