/** @file ledtest.c
*
* @brief Simple LED blinker
*
* @author Alvaro Prieto
*/
#include "common.h"
#include "intrinsics.h"
#include "leds.h"
#include "timers.h"
#include <signal.h>

void blink_led (void)
{
  led1_toggle();
  increment_ccr( 1, 10900 );
}

int main( void )
{
  // Turn off watchdog timer
  WDTCTL = WDTPW|WDTHOLD;

  // Initialize LEDs
  setup_leds();
  setup_timer_a();
  
  register_timer_callback( blink_led, 1 );

  set_ccr( 1, 10900);
  
  clear_ccr( 2 );
  
  eint();

  for(;;) {
    // Toggle all three leds
    //led1_toggle();

    /* Delay for a while before blinking */
    __delay_cycles(0x4fff);

  }  
}


