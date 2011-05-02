/** @file uart.h
*
* @brief intrinsic functions for compatibility with some code
*
* @author Alvaro Prieto
*/
#ifndef _INTRINSICS_H
#define _INTRINSICS_H

#include "common.h"

void __delay_cycles(unsigned long cycles);
void  __set_interrupt_state(unsigned short state);
unsigned short __even_in_range(unsigned short value, unsigned short bound);
unsigned short __get_interrupt_state(void);

#endif /* _INTRINSICS_H */\

