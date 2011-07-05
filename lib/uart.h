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

void setup_uart( void );

void uart_put_char( uint8_t );

void uart_write( uint8_t*, uint16_t );

void uart_write_escaped( uint8_t*, uint16_t );

void spi_put_char( uint8_t );

#endif /* _UART_H */\

