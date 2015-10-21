Target MCU is ATMEL ATmega168 @ 8 MHz (internal RC clock signal).

When configured as Master MCU starts sending PING packets each 1000ms
and waiting response from Slave. The communication dialog is printed via UART TX pin.

When configured as Slave MCU enters in Power-down mode and wakes up at each
received packet then retranslates it back to Master