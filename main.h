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

#define MASTER  1
#define CH_NO 0 // channel number

