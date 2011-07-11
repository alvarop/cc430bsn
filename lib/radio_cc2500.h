/** @file radio_cc2500.h
*
* @brief cc2500 radio functions
*
* @author Alvaro Prieto
*     derived from SLAA465 examples
*/
#ifndef _RADIO_CC2500_H
#define _RADIO_CC2500_H

#include "common.h"
#include "RF1A.h"
#include "hal_pmm.h"

#define CC2500_BUFFER_SIZE 255

void setup_radio( uint8_t (*)(uint8_t*, uint8_t) );
void radio_tx( uint8_t*, uint8_t );


#endif /* _RADIO_CC2500_H */\

