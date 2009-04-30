#include "iom168.h"

#include <intrinsics.h>
#include <pgmspace.h>
#include <string.h>

#define u8 unsigned char
#define u16 unsigned int
#define nop() __no_operation();
#define sbi(reg,bit)  (reg |= (1<<bit))   //<! set bit in port
#define cbi(reg,bit)  (reg &= ~(1<<bit))  //<! clear bit in port
#define sbr(reg,bit)  (reg |= (1<<bit))   //<! set bit in port
#define cbr(reg,bit)  (reg &= ~(1<<bit))  //<! clear bit in port

#define XTALL			8.0

#define	_delay_us(us)	__delay_cycles (XTALL * us);
#define _delay_ms(ms)	_delay_us (1000 * ms) 
