/** @file serialtest.c
*
* @brief Set up serial port at 115200 and echo characters
*
* @author Alvaro Prieto
*/
#include <cc430.h>
#include <stdint.h>


#include "oscillator.h"
#include "leds.h"
#include "uart.h"

/* Brute Force delay loop */
void delay(unsigned int d)
{
    for (; d>0; d--) {
        nop();
        nop();
    }
}

int main( void )
{
  
  /* Init watchdog timer to off */
  WDTCTL = WDTPW|WDTHOLD;
  
  setup_oscillator();

  setup_leds();
  
  setup_uart();
  
  __bis_SR_register(GIE);		// enable general interrupts

  /* Loop until the universe breaks down */
  for (;;) {
    /* Toggle P1.0 ouput pin */
    led1_toggle();

    /* Delay for a while before blinking */
    delay(0x4fff);
    delay(0x4fff);
    delay(0x4fff);
    delay(0x4fff);
    delay(0x4fff);
    delay(0x4fff);
    delay(0x4fff);
    delay(0x4fff);
    delay(0x4fff);
    delay(0x4fff);
    delay(0x4fff);
  }  /* while */
}





