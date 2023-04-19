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

typedef struct             //Nag��wek pliku BMP
{  
 uint16_t bfType;          //Magiczna liczba
 uint32_t bfSize;          //D�ugo�� pliku  
 uint32_t bfReserved;      //Zarezerwowane
 uint32_t bfOffBits;       //Ofset do danych
} BITMAPFILEHEADER;  
  
#define BF_TYPE 0x424D     //Magiczna liczba -BM dla pliku BMP  
  
typedef struct                   //Nag��wek zawieraj�cy informacje o obrazie
{  
    uint32_t   biHdrSize;        //D�ugo�� nag��wka
    int32_t    biWidth;          //Szeroko�� obrazu  
    int32_t    biHeight;         //Wysoko�� obrazu  
    uint16_t   biPlanes;         //Liczba plan�w  
    uint16_t   biBitCount;       //Ilo�� bit�w na piksel  
    uint32_t   biCompression;    //Typ kompresji  
    uint32_t   biSizeImage;      //Rozmiar obrazu w bajtach  
    uint32_t   biXPelsPerMeter;  //Rozdzielczo�� pozioma (pikseli/m)
    uint32_t   biYPelsPerMeter;  //Rozdzielczo�� pionowa (pikseli/m)
    uint32_t   biClrUsed;        //Liczba u�ytych kolor�w
    uint32_t   biClrImportant;   //Liczba istotnych kolor�w  
} BITMAPINFOHEADER;  
  
 
#define BI_RGB       0             //Brak kompresji
#define BI_RLE8      1             //RLE 8 bit�w/piksel 
#define BI_RLE4      2             //RLE 4 bity na piksel 
#define BI_BITFIELDS 3             //Obraz RGB 

typedef struct                     // Tablica kolor�w  
{  
    uint8_t  rgbBlue;          // Niebieski
    uint8_t  rgbGreen;         // Zielony  
    uint8_t  rgbRed;           // Czerwony  
    uint8_t  rgbReserved;      // Zarezerwowane/alfa  
} RGBQUAD;  
  
typedef struct                       //Dane o bitmapie
{  
	BITMAPFILEHEADER bmiFileHdr;     //Nag��wek BMP
    BITMAPINFOHEADER bmiHeader;      //Nag��wek  
    RGBQUAD          bmiColors[256]; //Mapa kolor�w
} BITMAPINFO; 
	
void LCD_BMP(const uint8_t __memx *Image, uint16_t Xpos, uint8_t Ypos)
{
    const uint8_t __memx *ImageData;  //Wska�nik do danych binarnych o obrazie
	const BITMAPINFO __memx *ImageHDR=(const BITMAPINFO __memx*)Image;
	const uint8_t __memx *rgb=(const __memx uint8_t*)ImageHDR;
	rgb+=__builtin_offsetof(BITMAPINFO, bmiColors);
	const RGBQUAD __memx *rgbtable=(const RGBQUAD __memx *)rgb;

    uint16_t LineWidth;               //Szeroko�� bitmapy w bajtach (wielokrotno�� 4 bajt�w)
	uint16_t DisplayWidth;            //Szeroko�� bitmapy na ekranie (je�li wi�ksza ni� pozosta�a szeroko�� ekranu)
    uint8_t DisplayHeight;            //Wysoko�� bitmapy na ekranie

    void DecompressRGB()
	{
		for(uint8_t dy=0; dy < DisplayHeight; dy++)
		{
			const uint8_t __memx *tmpimgdata=ImageData;
			for(uint16_t dx=0; dx <= DisplayWidth; dx++)
			{
				uint8_t index=*tmpimgdata++;    //Bajt zawiera indeks do tabeli kolor�w
				ssd2119_SendDataByte(rgbtable[index].rgbRed);    //Wy�lij poszczeg�lne sk��dowe koloru
				ssd2119_SendDataByte(rgbtable[index].rgbGreen);
				ssd2119_SendDataByte(rgbtable[index].rgbBlue);
			}
			ImageData+=LineWidth;  //Kolejna linia obrazu
		}
	}
	
	void SkipLinesRLE8(uint8_t lines)  //Funkcja pomija wskazan� liczb� linii skompresowanej bitmapy
	{
		while(lines)
		{
			uint8_t rlecnt=*ImageData++;       //Ile powt�rze�
			if(rlecnt) ImageData++;            //Pomijamy powtarzany bajt
			else
			{
				uint8_t unique=*ImageData++;       //Kod escape
				if(unique >= 2) ImageData+=((unique + 1) & 0b11111110); //Liczba niepowtarzaj�cych si� bajt�w wyr�wnana do granicy s�owa
				else if(unique == 0) lines--;
			}
		}
	}
	
	void DecompressRLE8()
	{
		for(uint8_t dy=DisplayHeight; dy > 0; dy--)
		{
			uint16_t pixcnt=0;   //Licznik wy�wietlonych pikseli w linii
			uint8_t cnt;         //Licznik powt�rzonych bajt�w
			
			while(pixcnt <= DisplayWidth)
			{
				uint8_t rlecnt=*ImageData++;  //Ile powt�rze�
				if(rlecnt)
				{
					uint8_t pixel=*ImageData++;          //Powtarzany piksel
					if((pixcnt + rlecnt) > DisplayWidth) //Czy mie�cimy si� na LCD?
					 {
						 rlecnt=DisplayWidth-pixcnt+1;   //Je�li nie to ile pikseli mo�na wy�wietli�?
						 while(rlecnt--)
						 {
							ssd2119_SendDataByte(rgbtable[pixel].rgbRed);    //Wy�lij poszczeg�lne sk�adowe koloru
							ssd2119_SendDataByte(rgbtable[pixel].rgbGreen);
							ssd2119_SendDataByte(rgbtable[pixel].rgbBlue);
						 }
						 SkipLinesRLE8(1);         //Pomijamy piksele poza matryc�
						 pixcnt=-1;
					 } else
					 {
						pixcnt+=rlecnt;     //Dodajemy liczb� powt�rze� piksela
						while(rlecnt--)
						{
							ssd2119_SendDataByte(rgbtable[pixel].rgbRed);    //Wy�lij poszczeg�lne sk�adowe koloru
							ssd2119_SendDataByte(rgbtable[pixel].rgbGreen);
							ssd2119_SendDataByte(rgbtable[pixel].rgbBlue);
						}
					 }
				} else
				{    //Mamy ci�g niepowtarzaj�cych si� bajt�w
					uint8_t unique=*ImageData++;  //Ile bajt�w si� nie powtarza?
					switch(unique)
					{
						case 0 :  pixcnt=-1;  //Znak ko�ca bie��cej linii
						          LCD_SetPosition(Xpos, Ypos + dy);   //Pozycja bitmapy - przejd� do kolejnej linii
						          ssd2119_SendCmd(ssd2119_Write_to_GRAM);  //Kontynuuj zapis do GRAM
						          break;
						case 1 :  return;  //Koniec bitmapy
						          break;
						case 2 :  break;  //Funkcja przej�cia do wskazanej pozycji - nie implementujemy jej
						default:  
								  cnt=0;
						          while(cnt < unique)
						          {
							       pixcnt++; cnt++;
							       uint8_t pixel=*ImageData++; //Unikalny piksel
							       ssd2119_SendDataByte(rgbtable[pixel].rgbRed);    //Wy�lij poszczeg�lne sk�adowe koloru
							       ssd2119_SendDataByte(rgbtable[pixel].rgbGreen);
							       ssd2119_SendDataByte(rgbtable[pixel].rgbBlue);
								   if(pixcnt > DisplayWidth) 
								   {
									   unique=((unique + 1) & 0b11111110);
									   ImageData+=(unique-cnt); //Pomi� pozosta�e bajty
									   SkipLinesRLE8(1);
									   pixcnt=-1;
									   break;
								   }
						          }
						          if((unique & 1) && (pixcnt!=-1)) ImageData++;  //Na ko�cu mamy zawsze bajt o warto�ci 0
						          break;
					}
				}
			}
		}		
	}
	
	void SkipLinesRLE4(uint8_t lines)  //Funkcja pomija wskazan� liczb� linii skompresowanej bitmapy
	{
		while(lines)
		{
			uint8_t rlecnt=*ImageData++;       //Ile powt�rze�
			if(rlecnt) ImageData++;            //Pomijamy powtarzany bajt
			else
			{
				uint8_t unique=*ImageData++;       //Kod escape
				if(unique >= 2) ImageData+=(((unique+3)/2) & 0b11111110); //Liczba niepowtarzaj�cych si� bajt�w wyr�wnana do granicy s�owa
				else if(unique == 0) lines--;
			}
		}
	}
	
	void  DecompressRLE4()
	{
		for(uint8_t dy=DisplayHeight; dy > 0; dy--)
		{
			uint16_t pixcnt=0;   //Licznik wy�wietlonych pikseli w linii
			uint8_t cnt;         //Licznik powt�rzonych bajt�w
			
			while(pixcnt <= DisplayWidth)
			{
				uint8_t rlecnt=*ImageData++;  //Ile powt�rze�
				if(rlecnt)
				{
					uint8_t pixel=*ImageData++;          //Powtarzany piksel
					if((pixcnt + rlecnt) > DisplayWidth) //Czy mie�cimy si� na LCD?
					{
						rlecnt=DisplayWidth-pixcnt+1;   //Je�li nie to ile pikseli mo�na wy�wietli�?
						while(rlecnt--)
						{
							pixel=__builtin_avr_swap(pixel);                  //Zmie� kolejno�� tetrad
							uint8_t nibble=pixel & 0x0f;                      //M�odsza tetrada okre�la indeks do tablicy kolor�w
							ssd2119_SendDataByte(rgbtable[nibble].rgbRed);    //Wy�lij poszczeg�lne sk�adowe koloru
							ssd2119_SendDataByte(rgbtable[nibble].rgbGreen);
							ssd2119_SendDataByte(rgbtable[nibble].rgbBlue);
						}
						SkipLinesRLE4(1);         //Pomijamy piksele poza matryc�
						pixcnt=-1;
					} else
					{
						pixcnt+=rlecnt;     //Dodajemy liczb� powt�rze� piksela
						while(rlecnt--)
						{
							pixel=__builtin_avr_swap(pixel);                  //Zmie� kolejno�� tetrad
							uint8_t nibble=pixel & 0x0f;                      //M�odsza tetrada okre�la indeks do tablicy kolor�w
							ssd2119_SendDataByte(rgbtable[nibble].rgbRed);    //Wy�lij poszczeg�lne sk�adowe koloru
							ssd2119_SendDataByte(rgbtable[nibble].rgbGreen);
							ssd2119_SendDataByte(rgbtable[nibble].rgbBlue);
						}
					}
				} else
				{    //Mamy ci�g niepowtarzaj�cych si� bajt�w
					uint8_t unique=*ImageData++;  //Ile bajt�w si� nie powtarza?
					switch(unique)
					{
						case 0 :  pixcnt=-1;  //Znak ko�ca bie��cej linii
						          LCD_SetPosition(Xpos, Ypos + dy);   //Pozycja bitmapy - przejd� do kolejnej linii
						          ssd2119_SendCmd(ssd2119_Write_to_GRAM);  //Kontynuuj zapis do GRAM
						          break;
						case 1 :  return;  //Koniec bitmapy
						          break;
						case 2 :  break;  //Funkcja przej�cia do wskazanej pozycji - nie implementujemy jej
						default:  cnt=0;
						          while(cnt < unique)
						          {
							         uint8_t pixel=pixel;                   //Unikamy ostrze�enia o potencjalnie niezainicjalizowanej zmiennej
									 if((cnt & 1) == 0) pixel=*ImageData++; //Za�aduj dane o dw�ch pikselach
									 pixel=__builtin_avr_swap(pixel);                 //Zmie� kolejno�� tetrad
									 uint8_t nibble=pixel & 0x0f;                     //M�odsza tetrada okre�la indeks do tablicy kolor�w
							         ssd2119_SendDataByte(rgbtable[nibble].rgbRed);   //Wy�lij poszczeg�lne sk�adowe koloru
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
						          if((((unique & 0b11) == 0b01) || ((unique & 0b11) == 0b10)) && (pixcnt!=-1)) ImageData++;  //Na ko�cu mamy zawsze bajt o warto�ci 0
						          break;
					}
				}
			}
		}
	}
	 
    ImageData=Image;    //Wska�nik do danych bitmapy
	ImageData+=ImageHDR->bmiFileHdr.bfOffBits;     //Wylicz adres danych na podstawie przemieszczenia w nag��wku
	LineWidth=ImageHDR->bmiHeader.biWidth;
	LineWidth=(LineWidth + 3) & (~0x03);           //Zaokr�glij w g�r� szeroko�� w bajtach, tak aby by�a podzielna przez 4

	DisplayWidth=ImageHDR->bmiHeader.biWidth;
	if((LCD_GetMaxX()-Xpos) < ImageHDR->bmiHeader.biWidth)    //Bitmapa wystaje poza ekran
		DisplayWidth=LCD_GetMaxX()-Xpos-1;

	DisplayHeight=ImageHDR->bmiHeader.biHeight;
	if((LCD_GetMaxY()-Ypos) < ImageHDR->bmiHeader.biHeight)    //Bitmapa wystaje poza ekran
		DisplayHeight=LCD_GetMaxY()-Ypos-1;
	
	LCD_SetWindow(Xpos, Ypos, Xpos + DisplayWidth, Ypos + DisplayHeight);
	LCD_SetPosition(Xpos, Ypos + DisplayHeight);   //Pozycja bitmapy
	
	ssd2119_SendCmdWithData(ssd2119_Entry_Mode, (ssd2119_EntryMode_Reg){.DFM=0b10, .DenMode=1, .TY=0b01, .ID=0b01, .AM=0}.word); //Zapisujemy od do�u - tak zapisywane s� pliki BMP
	ssd2119_SendCmd(ssd2119_Write_to_GRAM);        //Zapis danych o bitmapie pod wskazan� pozycj�

    switch (ImageHDR->bmiHeader.biCompression)
	{
		case BI_RGB:       ImageData+=LineWidth*(ImageHDR->bmiHeader.biHeight - DisplayHeight); //Poniewa� bitmapa zapisana jest od do�u musimy pomin�� odpowiedni� liczb� "dolnych" linii
		                   DecompressRGB(); break;    //zdekompresuj nieskompresowan� map� bitow�
		case BI_RLE8:      SkipLinesRLE8(ImageHDR->bmiHeader.biHeight - DisplayHeight);           //Pomi� podan� liczb� linii w pliku BMP
		                   DecompressRLE8(); break;   //zdekompresuj bitmap� skompresowan� RLE8
		case BI_RLE4:      SkipLinesRLE4(ImageHDR->bmiHeader.biHeight - DisplayHeight);           //Pomi� podan� liczb� linii w pliku BMP
		                   DecompressRLE4(); break;   //zdekompresuj bitmap� skompresowan� RLE4
		default:           break;            //Nieznany typ kompresji wi�c nic nie robimy
	};
	
	_delay_loop_1(2);   //Zaczekaj na opr�nienie bufora nadajnika USART (2*3 takty + co�tam z poprzedzaj�cego kodu)
	ssd2119_CS(1);
	ssd2119_SendCmdWithData(ssd2119_Entry_Mode, (ssd2119_EntryMode_Reg){.DFM=0b10, .DenMode=1, .TY=0b01, .ID=0b11, .AM=0}.word); //Przywr�� domy�lne parametry zapisu do GRAM
}
