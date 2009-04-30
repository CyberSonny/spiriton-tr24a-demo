/******************************************************************************
  Simple demo code for SPIRIT-ON ISM Band Transceiver TR24A (IC EM198810)
  with UART debugging information
  v0.01 (a)
  (c) 2009 Alexander Yerezeyev
  http://alyer.frihost.net
  e-mail: wapbox@bk.ru
  ICQ: 305206239
  
Target MCU: Atmega168 with internal RC @ 8MHz
All timings bellow are calculated for 8MHz CLK frequency!

You can define Master or Slave functionality by comment/uncomment 
  "#define MASTER  1"string in main.h

When configured as Master MCU starts sending PING packets each 1000ms
and waiting response from Slave. The communication dialog is printed via UART.

When configured as Slave MCU enters in Power-down mode and wakes up at each
received packet then retranslates it back to Master

*******************************************************************************/

#include "main.h"
#include "compiler.h"
#include "EM198810.h"
#include "UART.h"

#include "flashstr.h"

void SPI_MasterInit(void)
{  
  /* Set MOSI and SCK, SS output, all others input */
  DDR_SPI = (1<<DD_MOSI)|(1<<DD_SCK)|(1<<DD_SS)|(1<<RESET_n);
  DDR_LED = (1<<LED_TX)|(1<<LED_RX)|(1<<LED_INFO);
  sbi(PORT_PKT_FLG, PKT_flag); // turn-on pullup
  sbi(PORT_PKT_FLG, FIFO_flag); // turn-on pullup
  sbi (PORT_SPI, SS); // set SS high
  cbi (PORT_SPI, RESET_n); // Reset RF module
  /* Enable SPI, Master, set clock rate */
  SPCR = (1<<SPE)|(1<<MSTR)|(1<<CPHA);
  SPSR = (1<<SPI2X); // Max speed
}

void print_dump(unsigned char* p, unsigned int len)
{
  unsigned char i=0;
  while (i<len)
  {
     printf_P(_HexByteNum, *p);
     i++;
     p++;
     putchar (0x3B);
  }  
  printf_P(_Razer);
  printf_P(_Razer);
}


u16 corr_cnt;
u16 tout_cnt;

void main (void)
{
  corr_cnt=0;
  tout_cnt=0;
  u8 msglen=0;
#ifdef MASTER  

  
  uart_init();
  printf_P(_Razer);
  printf_P(_Hello);
  printf_P(_Razer);
#endif  
  
  SPI_MasterInit();
  Init_RF();
 // sbi(PORT_LED, LED_INFO);
//  _delay_ms(500);
//  cbi(PORT_LED, LED_INFO); 
#ifdef MASTER
   sbi(PORT_LED, LED_INFO);
  _delay_ms(500);
  cbi(PORT_LED, LED_INFO);   
  
TX_begin:  
  SetModeTx();

  memcpy_P(&FIFObuf[0],TXmsg,strlen_P(TXmsg) ); // copy message to FIFObuf
  // do PING
    printf_P(_Ping); 
  FIFO_write(strlen(FIFObuf), &FIFObuf[0]); // write SRAM FIFObuf to EM198810 and transmitt   
 // _delay_ms(100);
  if (PacketReceived()==1) //wait for PONG or time-out
  { // PONG has come
    msglen=FIFO_read(&FIFObuf[0]); //read FIFO buffer from EM198810 to SRAM FIFObuf.
    put_message(&FIFObuf[0]);
    if ((strcmp_P(FIFObuf, TXmsg ))!=0) 
    {
      //sbi (PORT_LED, LED_INFO);
      corr_cnt++;
      printf_P (_Corrupted, corr_cnt);
    }  
    else
    {
      printf_P (_Pong);
    }
  } 
  else
  {
    // the ball was loose (time out)
    tout_cnt++;
    printf_P (_Loose, tout_cnt);
  }
  
  _delay_ms(1000);
goto TX_begin;
#endif  

#ifndef MASTER
// SLAVE routines  
  sbi(PORT_LED, LED_INFO);
  _delay_ms(500);
  cbi(PORT_LED, LED_INFO);  
  /*
  sbi (EICRA, ISC00); // any logic level change will generate IRQ
  sbi (EICRA, ISC01); // any logic level change will generate IRQ
  EIFR= (1<<INTF0);   // clear old requests  
  sbi (EIMSK, INT0);  // enable INT0 aka PKT_flag IRQ
  */
  sbi (PCMSK2, PCINT18);  // turn on PCINT18 (PD2)
  PCIFR=(1<<PCIF2);   // clear old request
  sbi(PCICR, PCIE2);  // enable pin change interrupt
  SMCR=(1<<SE)|(1<<SM1); // select power-down mode
  
  SetModeRx(); // switch to RX  
  __enable_interrupt();  
RX_loop:
    __sleep();    // go to SLEEP until packet recevied
    goto RX_loop;
#endif  
}
