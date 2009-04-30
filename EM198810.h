/******************************************************************************
  Driver for SPIRIT-ON ISM Band Transceiver TR24A (IC EM198810)
  with UART debugging information
  v0.01 (a)
  (c) 2009 Alexander Yerezeyev
  http://alyer.frihost.net
  e-mail: wapbox@bk.ru
  ICQ: 305206239
  
Target MCU: Atmega168 with internal RC @ 8MHz
All timings bellow is calculated for 8MHz CLK frequency!
*******************************************************************************/

#define DDR_SPI DDRB
#define PORT_SPI PORTB
#define PORT_LED PORTD
#define DDR_LED DDRD
#define PIN_SPI  PINB
#define PIN_PKT_FLG  PIND
#define PORT_PKT_FLG  PORTD

#define DD_MOSI PB3
#define DD_SCK PB5
#define DD_SS PB2

#define SS PB2
#define RESET_n PB1
#define PKT_flag  PD2
#define FIFO_flag PD3
#define LED_TX PD5
#define LED_RX PD6
#define LED_INFO PD7

extern u8 FIFObuf[64];

/******************************************
/ Write EM198810 register with 16-bit data
*******************************************/
extern void SpiRegTx (u8 Addr, u16 Data);

/******************************************
/ Read 16-bit data from EM198810 register
*******************************************/
extern u16 SpiRegRx (u8 Addr);

/********************************************
 Switch transmitter into RX mode
********************************************/
extern void SetModeRx(void);

/********************************************
 Switch transmitter into TX mode
********************************************/
extern void SetModeTx(void);

/********************************************
 Switch transmitter into TX mode with pre-defined channel
********************************************/
extern void SetModeTxNo(u8 Channel);

/*************************************************
 Write FIFO buffer and send it to the air
*************************************************/
extern void FIFO_write(u8 tx_leng, u8* tx_data);

/*************************************************
 Read FIFO buffer
*************************************************/
extern u8 FIFO_read(u8* rx_buf);

/*****************************************
// wait for received packet 
// return -1: when timeout occured
// return 1: when packed received
*****************************************/
extern signed char PacketReceived (void);

/************************************************
  RF Initialisation routine
************************************************/
extern void Init_RF(void);