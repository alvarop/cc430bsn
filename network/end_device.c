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
#include "timers.h"
#include "cc2500.h"
#include "radio_cc2500.h"
#include "network.h"

#include <string.h>

static uint8_t blink_led1 (void);
static uint8_t rx_callback( uint8_t*, uint8_t );

int main( void )
{   
  
  /* Init watchdog timer to off */
  WDTCTL = WDTPW|WDTHOLD;
     
  // Setup oscillator for 12MHz operation
  BCSCTL1 = CALBC1_12MHZ;
  DCOCTL = CALDCO_12MHZ; 
  
  // Wait for changes to take effect
  __delay_cycles(4000);
      
  setup_uart();
  
  setup_spi();
  
  setup_timer_a(MODE_CONTINUOUS);
  
  register_timer_callback( blink_led1, 1 );

  set_ccr( 1, 100 );
  
  setup_cc2500( rx_callback );
  
  cc2500_set_address( DEVICE_ADDRESS );
       
  setup_leds();
           
  for (;;) 
  {        
    __bis_SR_register( LPM1_bits + GIE );   // Sleep

  } 
  
  return 0;
}

/*******************************************************************************
 * @fn     uint8_t blink_led1( void )
 * @brief  timer_a callback function, blinks LED every LED_BLINK_CYCLES
 * ****************************************************************************/
static uint8_t blink_led1 (void)
{
  static uint8_t counter = LED_BLINK_CYCLES;
  if( 0 == counter-- )
  {
    led1_toggle();
    counter = LED_BLINK_CYCLES;
  }
  


  return 0;
}

/*******************************************************************************
 * @fn     void rx_callback( void )
 * @brief  rx callback function, called when packet has been received
 * ****************************************************************************/
static uint8_t rx_callback( uint8_t* p_buffer, uint8_t size )
{    
  packet_header_t* start_packet;  
  start_packet = (packet_header_t*)p_buffer;
  
  led2_toggle();
  
  if ( PACKET_SYNC == ( start_packet->type & TYPE_MASK ) )
  {
    // Reset timer_a
    clear_timer();
    led1_on();
  }
  
  return 0;
}

