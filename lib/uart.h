/** @file uart.h
*
* @brief UART functions
*
* @author Alvaro Prieto
*/
#ifndef _UART_H
#define _UART_H

#include "common.h"
#include <signal.h>

#define GDO0_PxOUT      P2OUT
#define GDO0_PxIN       P2IN
#define GDO0_PxSEL      P2SEL
#define GDO0_PxDIR      P2DIR
#define GDO0_PxIE       P2IE
#define GDO0_PxIES      P2IES
#define GDO0_PxIFG      P2IFG
#define GDO0_PIN        BIT6

#define CSn_PxOUT       P3OUT
#define CSn_PxDIR       P3DIR
#define CSn_PIN         BIT0


void setup_uart( void );

void uart_put_char( uint8_t );

void uart_write( uint8_t*, uint16_t );

void uart_write_escaped( uint8_t*, uint16_t );

void setup_spi( void );

uint8_t hex_to_string( uint8_t* , uint8_t*, uint8_t );

#endif /* _UART_H */\

