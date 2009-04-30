#include "compiler.h"

#define LINE_LENGTH 80          /* Change if you need */

#define In_DELETE 0x7F          /* ASCII <DEL> */
#define In_EOL '\r'             /* ASCII <CR> */
#define In_SKIP '\3'            /* ASCII control-C */
#define In_EOF '\x1A'           /* ASCII control-Z */

#define Out_DELETE "\x8 \x8"    /* VT100 backspace and clear */
#define Out_SKIP "^C\n"         /* ^C and new line */
#define Out_EOF "^Z"            /* ^Z and return EOF */

/****************************************************************************
**
** Putchar function needed for ICC
**
** Parameters: send_char, character to send
**
** Returns: Sent character
**
****************************************************************************/

u8 putchar(unsigned char send_char)
{
while (!(UCSR0A & 0x20))
; /* wait xmit ready */
UDR0 = (unsigned char) send_char;
return(send_char);
}

u8 _low_level_get(void)
{
/* Wait for data to be received */
while ( !(UCSR0A & (1<<RXC0)) )
;
/* Get and return received data from buffer */
return UDR0;
}

/*******************************
 put message from SRAM to UART
*******************************/
void put_message(char *s)
{
  while (*s)
    putchar(*s++);
}


/****************************************************************************
**
** Initializes UART0
**
** Parameters: None
**
** Returns: None
**
****************************************************************************/
void uart_init(void)
{
/* UART0 initialisation */

UCSR0B = 0x00; /* disable while setting baud rate */
UCSR0A = 0x00;
UCSR0C = 0x06; // 8 bit data
UBRR0L = 12; // set baud rate lo (38400 @ 8 MHz)
UBRR0H = 0x00; /* set baud rate hi */
UCSR0B = 0x18; //RX and TX are enabled
}



