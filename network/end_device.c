/** @file end_device.c
*
* @brief Network Discovery End Device
*
* @author Alvaro Prieto
*/
#include "common.h"

#include "intrinsics.h"
#include "leds.h"
#include "uart.h"
#include "cc2500.h"
#include "radio_cc2500.h"

#include <string.h>

#ifndef DEVICE_ADDRESS
#define DEVICE_ADDRESS 0xFF
#endif

/*******************************************************************************
 * @fn     void rx_callback( void )
 * @brief  rx callback function, called when packet has been received
 * ****************************************************************************/
static uint8_t rx_callback( uint8_t* p_buffer, uint8_t size )
{  

  return 0;
}

int main( void )
{   
  
  /* Init watchdog timer to off */
  WDTCTL = WDTPW|WDTHOLD;
     
  // Setup oscillator for 12MHz operation
  BCSCTL1 = CALBC1_12MHZ;
  DCOCTL = CALDCO_12MHZ; 
  
  // Divide SMCLK by 8
  BCSCTL2 = DIVS_3;  
  
  // Wait for changes to take effect
  __delay_cycles(4000);
      
  setup_uart();
  
  setup_spi();
  
  setup_timer_a(MODE_CONTINUOUS);
  
  setup_cc2500( rx_callback );
  
  cc2500_set_address( DEVICE_ADDRESS );
       
  setup_leds();
           
  for (;;) 
  {        
    __bis_SR_register( LPM3_bits + GIE );   // Sleep

  } 
  
  return 0;
}
