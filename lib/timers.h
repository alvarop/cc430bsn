/** @file timers.h
*
* @brief Timer functions
*
* @author Alvaro Prieto
*/
#ifndef _TIMERS_H
#define _TIMERS_H

#include "common.h"

#define TOTAL_CCRS 5 // Number of capture compare registers

void setup_timer_a( void );
void register_timer_callback( void (*)(void), uint8_t  );
void set_ccr( uint8_t, uint16_t );
void clear_ccr( uint8_t );
void increment_ccr( uint8_t, uint16_t );
#endif /* _TIMERS_H */\

