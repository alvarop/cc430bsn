/** @file serialtest.c
*
* @brief Set up serial port at 115200 and echo characters
*
* @author Alvaro Prieto
*/
#include "common.h"

#include "intrinsics.h"
#include "oscillator.h"
#include "leds.h"
#include "uart.h"

int main( void )
{
  uint8_t string[] = "UART Test Program!\r\nEcho mode...\r\n";
  /* Init watchdog timer to off */
  WDTCTL = WDTPW|WDTHOLD;
  
  setup_oscillator();

  setup_leds();
  
  setup_uart();
  
  __bis_SR_register(GIE);		// enable general interrupts
  
  uart_write( string, sizeof(string) );
  
  for (;;) {
    
    led1_toggle();

    
    _delay_cycles(0x40000);
    
  }  /* while */
}





