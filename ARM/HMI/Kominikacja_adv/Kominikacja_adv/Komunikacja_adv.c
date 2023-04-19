/*
 * Komunikacja_adv.c
 *
 * Created: 2017-04-15 10:52:41
 * Author : tmf
 */ 

#include "sam.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "Clk/SetClk.h"
#include "Delay/delay.h"
#include "usart.h"
#include "bufusart.h"

void USART_init()
{
	USART_bufs_init();

	REG_PM_APBCMASK|=PM_APBCMASK_SERCOM1;  //W³¹cz zegar dla SERCOM1
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(GCLK_CLKCTRL_ID_SERCOM1_CORE_Val) | //Generic Clock 0
	GCLK_CLKCTRL_GEN_GCLK0 |                            // jest Ÿród³em zegara
	GCLK_CLKCTRL_CLKEN;
	
	while(GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY); //Zaczekaj na zynchronizacjê
	
	LCD_USART.CTRLA.bit.ENABLE=0;      //Wy³¹cz SERCOM1
	while(LCD_USART.SYNCBUSY.bit.ENABLE);	

	LCD_PORT.WRCONFIG.reg=PORT_WRCONFIG_HWSEL | PORT_WRCONFIG_WRPINCFG | PORT_WRCONFIG_WRPMUX | PORT_WRCONFIG_PMUX(2) | PORT_WRCONFIG_PMUXEN | 0b0110; //Wybierz funkcjê SERCOM1 dla PA16-19
	LCD_PORT.DIR.reg=LCD_TX;  //Pin Tx jest wyjœciem

	uint32_t baudTimes8 = (SystemCoreClock * 8) / (16 * 9600);
	LCD_USART.BAUD.FRAC.FP   = (baudTimes8 % 8);
	LCD_USART.BAUD.FRAC.BAUD = (baudTimes8 / 8);

	LCD_USART.CTRLB.reg=SERCOM_USART_CTRLB_RXEN | SERCOM_USART_CTRLB_TXEN;  //8-bitów, bez stopu i parzystoœci
	LCD_USART.CTRLA.reg=SERCOM_USART_CTRLA_ENABLE | SERCOM_USART_CTRLA_RXPO(1) | SERCOM_USART_CTRLA_TXPO(1) | SERCOM_USART_CTRLA_MODE(SERCOM_USART_CTRLA_MODE_USART_INT_CLK_Val) | SERCOM_USART_CTRLA_SAMPR(1); //PAD1-Rx, PAD2 -Tx, generator frakcyjny
	while(LCD_USART.SYNCBUSY.reg & SERCOM_SPI_SYNCBUSY_MASK);  //Zaczekaj na synchronizacjê zapisu rejestrów
	NVIC_EnableIRQ(SERCOM1_IRQn);  //Odblokuj przerwania SERCOM1
}

void HMI_WaitForTxAndResponse()
{
	while(cb_ElementsNo(&recBuf) < 4);      //Zaczekaj na odpowiedŸ z modu³u - otrzymanie 4 bajtów
}

uint8_t HMI_GetResponse()
{
	uint8_t res=-1;  //Brak poprawnej odpowiedzi
	
	if(cb_ElementsNo(&recBuf) >= 4)
	{
		uint8_t tmpres=cb_Read(&recBuf);
		uint8_t ff=cb_Read(&recBuf) & cb_Read(&recBuf) & cb_Read(&recBuf);
		if(ff==0xFF) res=tmpres;  //Jeœli format odpowiedzi jest poprawny to j¹ zwróæ
	}
	return res;
}

uint8_t HMI_GetNumber(uint32_t *liczba)
{
	uint8_t res=-1;  //Brak poprawnej odpowiedzi
	
	if(cb_ElementsNo(&recBuf) >= 4)
	{
		res=cb_Read(&recBuf);   //Pobierz typ danych
		if(res == 0x71)         //Komunikat zawiera liczbê
		{
			while(cb_ElementsNo(&recBuf) < 7);  //Komunikat musi zawieraæ kolejnych 7 bajtów
			*liczba=cb_Read(&recBuf) + (cb_Read(&recBuf) << 8) + ((uint32_t)cb_Read(&recBuf) << 8) + ((uint32_t)cb_Read(&recBuf) << 8);
		}
		uint8_t ff=cb_Read(&recBuf) & cb_Read(&recBuf) & cb_Read(&recBuf);
		if(ff != 0xFF) res=-1;  //Jeœli format odpowiedzi jest poprawny to j¹ zwróæ
	}
	return res;
}

uint8_t HMI_GetString(char *str)
{
	uint8_t res=-1;  //Brak poprawnej odpowiedzi
	
	if(cb_ElementsNo(&recBuf) >= 4)
	{
		res=cb_Read(&recBuf);  //Pobierz typ danych
		if(res == 0x70)        //Komunikat zawiera ³añcuch
		{
			uint16_t indeks=0;
			uint8_t znak, sufiks=0;
			do{
				while(cb_IsEmpty(&recBuf));  //SprawdŸ czy jest kolejny znak, jeœli nie to zaczekaj
				znak=cb_Read(&recBuf);
				if(znak != 0xFF)
				{
					str[indeks++]=znak;
					sufiks=0;
				} else sufiks++;
			} while(sufiks<3);
			str[indeks]=0;   //Utwórz poprawny ³añcuch jêzyka C
			res=0x01;
		} else
		{
			uint8_t ff=cb_Read(&recBuf) & cb_Read(&recBuf) & cb_Read(&recBuf);
			if(ff != 0xFF) res=-1;  //Jeœli format odpowiedzi nie jest poprawny to zwróæ -1
		}
	}
	return res;
}

volatile static uint8_t resp;
volatile static uint32_t wynik;
volatile static uint8_t tekst[30];

int main(void)
{
    Set48MHzClk();
    delay_init();

	USART_init();
	
	USART_send_buf("bkcmd=3\xff\xff\xff");   //W³¹cz potwierdzenia wykonania przes³anych poleceñ
	HMI_WaitForTxAndResponse();
	resp=HMI_GetResponse();
	
	USART_send_buf("j0.val=100\xff\xff\xff");
	HMI_WaitForTxAndResponse();
	resp=HMI_GetResponse();

	USART_send_buf("j1.val=50\xff\xff\xff");
	HMI_WaitForTxAndResponse();
	resp=HMI_GetResponse();

	USART_send_buf("value.val=80\xff\xff\xff");
	HMI_WaitForTxAndResponse();
	resp=HMI_GetResponse();
	
	USART_send_buf("get value.val\xff\xff\xff");
	HMI_WaitForTxAndResponse();
	resp=HMI_GetNumber(&wynik);
	
	//USART_send_buf("get t1.val\xff\xff\xff");  //Nieprawid³owe polecenie - powinno zwróciæ 0x1A
	USART_send_buf("get t1.txt\xff\xff\xff");
	HMI_WaitForTxAndResponse();
	resp=HMI_GetString(&tekst);

	while (1); 
/*	{
		USART_putchar(&SERCOM1->USART, 0xff);
		delay_ms(5);
		USART_putchar(&SERCOM1->USART, 0x00);
		delay_ms(5);
	}*/
}
