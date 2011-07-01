/** @file timers.h
*
* @brief Timer functions
*
* @author Alvaro Prieto
*/
#ifndef _TIMERS_H
#define _TIMERS_H

#include "common.h"

#if defined(__CC430F6137__)
#define TOTAL_CCRS 5 // Number of capture compare registers
#elif defined(__MSP430F2274__)
#define TOTAL_CCRS 3 // Number of capture compare registers
#endif

#define MODE_OFF MC_0
#define MODE_UP MC_1
#define MODE_CONTINUOUS MC_2
#define MODE_UPDOWN MC_3

void setup_timer_a( uint8_t );
void register_timer_callback( uint8_t (*)(void), uint8_t);
void set_ccr( uint8_t, uint16_t );
void clear_ccr( uint8_t );
void increment_ccr( uint8_t, uint16_t );
inline void clear_timer();
#endif /* _TIMERS_H */\

