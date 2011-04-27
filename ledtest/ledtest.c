/** @file ledtest.c
*
* @brief Simple LED blinker
*
* @author Alvaro Prieto
*/
#include "common.h"
#include "intrinsics.h"
#include "leds.h"

int main( void )
{
  // Turn off watchdog timer
  WDTCTL = WDTPW|WDTHOLD;

  // Initialize LEDs
  setup_leds();

  for(;;) {
    // Toggle all three leds
    led1_toggle();
    led2_toggle();
    led3_toggle();

    /* Delay for a while before blinking */
    _delay_cycles(0x4fff);

  }  
}


