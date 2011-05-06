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

uint8_t blink_led1 (void)
{
  led1_toggle();
  increment_ccr( 1, 10900 );
  
  return 1;
}

int main( void )
{
  // Turn off watchdog timer
  WDTCTL = WDTPW|WDTHOLD;

  // Initialize LEDs
  setup_leds();
  setup_timer_a(MODE_CONTINUOUS);
  
  register_timer_callback( blink_led1, 1 );

  set_ccr( 1, 10900);
  
  
  eint();

  for(;;) {
    // Toggle all three leds
    //led1_toggle();
    __bis_SR_register(LPM3_bits);
    /* Delay for a while before blinking */
    __no_operation();
    led2_toggle();

  }  
}


