/** @file leds.h
*
* @brief LED functions
*
* @author Alvaro Prieto
*/
#ifndef _LEDS_H
#define _LEDS_H

#include "common.h"

void setup_leds( void );

void __inline__ led1_on( );
void __inline__ led1_off( );
void __inline__ led1_toggle( );

void __inline__ led2_on( );
void __inline__ led2_off( );
void __inline__ led2_toggle( );

void __inline__ led3_on( );
void __inline__ led3_off( );
void __inline__ led3_toggle( );

#endif /* _LEDS_H */\

