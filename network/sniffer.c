/** @file sniffer.c
*
* @brief Packet sniffer. Prints out all received packets (with some filtering)
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

#ifndef DEVICE_ADDRESS
#define DEVICE_ADDRESS 0xFF
#endif

static uint8_t rx_callback( uint8_t*, uint8_t );

static uint8_t tx_buffer[RADIO_BUFFER_SIZE];

extern cc2500_settings_t cc2500_settings;

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
  
  setup_cc2500( rx_callback );
  
  // Disable address checking
  cc2500_settings.pktctrl1 &= ~0x2;
  write_register( &cc2500_settings.pktctrl1 );
  
  cc2500_set_address( DEVICE_ADDRESS );
       
  setup_leds();

  for (;;) 
  {        
    __bis_SR_register( LPM1_bits + GIE );   // Sleep
    __no_operation();
  } 
  
  return 0;
}

/*******************************************************************************
 * @fn     void rx_callback( void )
 * @brief  rx callback function, called when packet has been received
 * ****************************************************************************/
static uint8_t rx_callback( uint8_t* p_buffer, uint8_t size )
{
  
  packet_header_t* packet;
          
  packet = (packet_header_t*)p_buffer;    

  tx_buffer[0] = size;
  memcpy(&tx_buffer[1], p_buffer, size );

  uart_write_escaped( tx_buffer, size+1 );
  
  led2_toggle();
  
  return 0;
}

