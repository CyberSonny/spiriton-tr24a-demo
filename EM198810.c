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
#include "compiler.h"
#include "main.h"
#include "EM198810.h"
#include "UART.h"
#include "flashstr.h"

// Configuration arrays stored in FLASH memory
// Upper registers
__flash u8 rf_fram_table[][3]={
  /*
//	{0x30,0x98,0x00},
	{0x30,0x98,0x40}, 
        {0x31,0xff,0x8f},
	{0x32,0x80,0x28},
        {0x33,0x80,0x56},
	{0x34,0x4E,0xF6},
        {0x35,0xF6,0xF5},
	{0x36,0x18,0x5C},
        {0x37,0xD6,0x51},
	{0x38,0x44,0x44},
        {0x39,0xE0,0x00}
  */
        {0x30,0x98,0x00},{0x31,0xff,0x8f},{0x32,0xA0,0x28},{0x33,0xA0,0x56},	
	{0x34,0x4E,0xF6},{0x35,0xF6,0xF5},{0x36,0x18,0x5C},{0x37,0xD6,0x51},
	{0x38,0x44,0x04},{0x39,0xE0,0x00}
	}; 
// Lower registers	
__flash u8 rf_regi_table[][3]={
/*        
        {0x09,0x00,0x01}, {0x00,0X01,0X40}, 
        {0x02,0X1F,0X01}, {0x04,0xBC,0xF0},
	{0x05,0x00,0xA1}, {0x07,0x12,0x4C}, {0x08,0x80,0x00}, {0x0c,0X80,0X00},                                  
        {0x0e,0x16,0x9b}, {0x0f,0x90,0xAD}, {0x10,0xb0,0x00}, {0x13,0xa1,0x14},
        {0x14,0x81,0x91}, {0x16,0x00,0x02}, 
        {0x18,0xB1,0x40}, {0x19,0xA8,0x0F},
        {0x1A,0x3F,0x04}, {0x1C,0x58,0x00},
*/

        {0x09,0x00,0x01}, {0x00,0X01,0X4D}, // {0x00,0X35,0X4D}, 
        {0x02,0X1F,0X01}, {0x04,0xBC,0xF0},
  
        
  //      {0x09,0x21,0x81},{0x00,0X35,0X4D}, 
  //     {0x02,0X1F,0X01},{0x04,0xB8,0xD8},
        
	{0x05,0x00,0xA1},{0x07,0x12,0x4C}, {0x08,0x80,0x08},{0x0a,0X00,0X04},                    
        {0x0b,0X40,0X41},{0x0c,0X7E,0X00},{0x0d,0X00,0X00},{0x0e,0x16,0x9d},                   
	{0x0f,0x80,0x2f},{0x10,0xb0,0xf8},{0x12,0xe0,0x00},{0x13,0xa1,0x14},                            
	{0x14,0x81,0x91},{0x15,0x69,0x62},{0x16,0x00,0x02},{0x17,0x00,0x02},
	{0x18,0xb1,0x40},{0x19,0x78,0x0f},{0x1a,0x3f,0x04},{0x1c,0x58,0x00}
	}; 

// FIFO buffer place in SRAM memory
u8 FIFObuf[64]="";

/*******************************
/ Send 8-bit Data via SPI
*******************************/
#pragma inline = forced
void SpiTx (u8 Data)
{
  SPDR= Data;
  while(!(SPSR & (1<<SPIF))); // wait for transmission  
}

/*******************************
/ Receive 8-bit Data via SPI
*******************************/
#pragma inline = forced
u8 SpiRx (void)
{
  SPDR = 0xFF; // Dummy byte
  while(!(SPSR & (1<<SPIF))); // wait for transmission  
  return SPDR;
}

/******************************************
/ Write EM198810 register with 16-bit data
*******************************************/
void SpiRegTx (u8 Addr, u16 Data)
{
  cbi (PORT_SPI, SS);             // set SS LOW
  nop();                          //Relationship between SPI_SS & SPI_CLK 125 ns @ 8MHz
  
  SpiTx(Addr&0x7F);               // write ADDR and write command (bit 7 - "0")  
  nop();
  SpiTx((u8) (Data >> 8));        // write 15-8 bits of data to spdr
  nop();                          //125 ns @ 8MHz    
  SpiTx((u8) (0x00FF&Data));      // write 7-0 bits of data to spdr
  nop();                          //125 ns @ 8MHz
  if (Addr<0x20) _delay_us(3);    // When MCU writes register 0x00~0x1f, at least 3us wait time is required for
                                  // framer to program register(0x00~0x1f) used by internal BuleRF interface Dbus.
  sbi (PORT_SPI, SS);             // set SS HIGH  
  nop(); //125 ns @ 8MHz
  nop(); //125 ns @ 8MHz
}

/******************************************
/ Read 16-bit data from EM198810 register
*******************************************/
u16 SpiRegRx (u8 Addr)
{
  u8 dataH,dataL;
  cbi (PORT_SPI, SS); // set SS LOW
  nop();              //Relationship between SPI_SS & SPI_CLK 125 ns @ 8MHz
  SpiTx(Addr|0x80);   // set ADDR and write command (bit 7 - "1")  
  if ((Addr&0x7F)<0x20) _delay_us(3); 
  dataH=SpiRx();      // read 15..8 bits of register
  nop();              //125 ns @ 8MHz
  dataL=SpiRx();      // read 7..0 bits of register
  nop();              //125 ns @ 8MHz
  sbi (PORT_SPI, SS); // set SS HIGH  
  nop();              //125 ns @ 8MHz
  nop();              //125 ns @ 8MHz
  return (u16) ((dataH<<8)|(dataL));
}

/****************************************
/ Reset transmitter FIFO buffer
****************************************/
#pragma inline = forced
void RstFIFOTx(void)
{
  SpiRegTx(0x52,0x8000); //RESET TX FIFO
}

/****************************************
// Reset receiver FIFO buffer
****************************************/
#pragma inline = forced
void RstFIFORx(void)
{
  SpiRegTx(0x52,0x0080); //RESET RX FIFO
}

/********************************************
 Switch transmitter into RX mode
********************************************/
void SetModeRx(void)
{ 
  RstFIFORx();                   //RESET RX (clear RX FIFO point)
  _delay_ms(5); 
  SpiRegTx(0x07,(1<<7)|(CH_NO)); //Enable RX @ 2402+CH_NO
  _delay_ms(5); 
}

/********************************************
 Switch transmitter into TX mode
********************************************/
void SetModeTx(void)
{ 
  SpiRegTx(0x07,(1<<8)|(CH_NO));  //Set DBUS_TX_EN (Enable transmission)
}

/********************************************
 Switch transmitter into TX mode with pre-defined channel
********************************************/
void SetModeTxNo(u8 Channel)
{ 
  Channel&=0x7F;
  SpiRegTx(0x07,(1<<8)|(Channel));  //Set DBUS_TX_EN (Enable transmission)
}

/*************************************************
 Write FIFO buffer and send it to the air
*************************************************/
void FIFO_write(u8 tx_leng, u8* tx_data)
{ 
  int i;
  sbi(PORT_LED, LED_TX);
  RstFIFOTx();                    //For transmit, it is required to clear FIFO write point
                                  //before application writes in data via access reg82[15].
  _delay_us(1);
  cbi (PORT_SPI, SS);             // set SS LOW
  nop();                          //Relationship between SPI_SS & SPI_CLK 125 ns @ 8MHz
  SpiTx(0x50);                    // ADDR = reg80
  // 500 ns wait
  nop();
  nop();
  nop();
  nop();
  if (tx_leng>63) tx_leng=63;     // Temporary limit message length
  SpiTx(tx_leng);                 // write 8 bits of data buffer length
  nop();                          //125 ns @ 8MHz
  // WRITE FIFO BUFFER
  for(i=0;i<tx_leng;i++) 
  {
    SpiTx(*tx_data++);
  }
  nop();                          //125 ns @ 8MHz
  sbi (PORT_SPI, SS);             // set SS HIGH  
  nop();                          //125 ns @ 8MHz
  nop();                          //125 ns @ 8MHz
  nop();                          //125 ns @ 8MHz
  nop();                          //125 ns @ 8MHz      
  SetModeTx();                    //Set DBUS_TX_EN (Enable transmission)  
  while (!(PIN_PKT_FLG&(1<<PKT_flag)));// wait PKT_flag high (TX end)
  cbi(PORT_LED, LED_TX);
  RstFIFOTx();              //RESET TX 
  SpiRegTx(0x07,0x0000);               // clear DBUS_RX_EN & DBUS_TX_EN
  _delay_ms(5);
}

/*************************************************
 Read FIFO buffer
*************************************************/
u8 FIFO_read(u8* rx_buf)
{
  u8 rx_len, i;
//  SetModeRx(); // switch to RX    
//  while (!(PIN_PKT_FLG&(1<<PKT_flag))); // need for polling mode of operation
  sbi(PORT_LED, LED_RX);    
  cbi (PORT_SPI, SS); // set SS LOW
  nop(); //Relationship between SPI_SS & SPI_CLK 125 ns @ 8MHz
  SpiTx (0x50|(1<<7)); // ADDR = reg80 for read
  // 500 ns wait
  nop();
  nop();
  nop();
  nop();
  rx_len = SpiRx();                 // read 8 bits of data received buffer length  
  // limit buffer length
  if (rx_len>63) rx_len=63;
  for (i=0;i<rx_len; i++)
  {
      *rx_buf++ = SpiRx();
  }  
  nop(); //125 ns @ 8MHz
  sbi (PORT_SPI, SS); // set SS HIGH  
  nop(); //125 ns @ 8MHz
  nop(); //125 ns @ 8MHz
  nop(); //125 ns @ 8MHz
  nop(); //125 ns @ 8MHz      
  RstFIFORx(); //RESET RX (clear RX FIFO point)  
  SpiRegTx(0x07,0x0000); // clear DBUS_RX_EN & DBUS_TX_EN
  _delay_ms(5); 
  cbi(PORT_LED, LED_RX);
  return rx_len;
}

/*****************************************
// wait for received packet 
// return -1: when timeout occured
// return 1: when packed received
*****************************************/
signed char PacketReceived (void)
{    
  u16 wait_timer=0;  
     SetModeRx(); // switch to RX    
    while (!(PIN_PKT_FLG&(1<<PKT_flag)))
      {
        if (wait_timer++==65535) return -1;
      }; // wait PKT_flag high (RX end)  
   return 1;     
}

/************************************************
  RF Initialisation routine
************************************************/
void Init_RF(void)
{ 
  u16 val;
  u8 i; 
  //u16 val;
  sbi (PORT_SPI, RESET_n);    
  _delay_ms(5);  // wait for crystal to stabilize
  //unsigned int rec_data; debug
  //INITIAL REG48~REG58   
  for (i=0;i<10;i++)
  {
    SpiRegTx((rf_fram_table[i][0]),(u16)(((rf_fram_table[i][1])<<8)|(rf_fram_table[i][2])));
  }
  _delay_ms(5); // wait for RFIC calibration 
  if ((SpiRegRx(0x40)>>8)!=0xc0) while (1);
  else  
  {
    //INITIAL REG0~REG28 
     for (i=0;i<20;i++)
     {
       SpiRegTx((rf_regi_table[i][0]),(u16)(((rf_regi_table[i][1])<<8)|(rf_regi_table[i][2])));
      }
    _delay_ms(5);
    SpiRegTx(0x07,0x0000); 
    }  
#ifdef MASTER  
  printf_P(_regi_tab);
  for (i=0;i<32;i++)
  {
    val = SpiRegRx(i);
    printf_P(_HexByteNum, i);
    putchar(':');
    printf_P(_HexByteNum, (u8) (val>>8));
    printf_P(_HexByteNum, (u8) (val&0x00FF));
    printf_P(_Razer);
  }        
   printf_P(_FRAM_tab);
    for (i=0;i<10;i++)
  {
    val = SpiRegRx(rf_fram_table[i][0]);
    printf_P(_HexByteNum, rf_fram_table[i][0]);
    putchar(':');
    printf_P(_HexByteNum, (u8) (val>>8));
    printf_P(_HexByteNum, (u8) (val&0x00FF));
    printf_P(_Razer);
  }
    printf_P(_Razer);
   val = SpiRegRx(0x40);
    //printf_P(_HexByteNum, 0x56);
    printf_P(_HexByteNum, (u8) (val>>8));
    printf_P(_HexByteNum, (u8) (val&0x00FF));  
#endif    
}

/***********************************************
  PKT_flag interrupt handler
  IRQ from Pin change
************************************************/
#ifndef MASTER
#pragma vector=PCINT2_vect
__interrupt void PCINT2_IRQ( void )
{
     u8 c, msglen;       
    for (c=0;c<=3;c++) if ((PIN_PKT_FLG&(1<<PKT_flag))==0) goto INT_exit; // return if it was low level IRQ    
    msglen=FIFO_read(&FIFObuf[0]); //read FIFO buffer from EM198810 to SRAM FIFObuf
    // turn ON information LED if received message was corrupted
   // if (!(strcmp_P(FIFObuf, TXmsg ))) sbi (PORT_LED, LED_INFO);
   // else cbi (PORT_LED, LED_INFO);
   // memcpy_P(&FIFObuf[0],TXmsg,strlen_P(TXmsg) ); // copy message to FIFObuf
    _delay_ms(50); // wait until MASTER is swithing to RX mode
    SetModeTx(); // switch SLAVE to TX       
    FIFO_write(strlen(FIFObuf), &FIFObuf[0]); // write SRAM FIFObuf to EM198810 and transmitt
    SetModeRx(); // switch to RX    
INT_exit:    
  PCIFR=(1<<PCIF2); 
}
#endif