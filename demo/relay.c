/** @file relay.c
*
* @brief  TODO
*
* @author Alvaro Prieto
*       
*/
#include "settings.h"
#include <signal.h>
#include <string.h>
#include "leds.h"
#include "oscillator.h"
#include "timers.h"
#include "radio.h"

typedef struct
{
  uint8_t length;
  uint8_t source;
  uint8_t type;
  uint8_t flags;
} packet_header_t;

typedef struct
{
  uint8_t samples[ADC_MAX_SAMPLES];
} packet_data_t;

typedef struct
{
  uint8_t rssi;
  uint8_t lqi_crcok;
} packet_footer_t;

uint8_t heartbeat();
uint8_t process_rx( uint8_t*, uint8_t );

uint8_t tx_buffer[256];
volatile uint8_t new_message = 0;

int main( void )
{
 
  // Stop watchdog timer to prevent time out reset
  WDTCTL = WDTPW + WDTHOLD;
   
  // Make sure processor is running at 12MHz
  setup_oscillator();
   
  // Initialize LEDs
  setup_leds();
   
  // Initialize timer
  set_ccr( 0, TIMER_LIMIT );
  setup_timer_a(MODE_UP);
  
  set_ccr( 2, 10 );
  register_timer_callback( heartbeat, 2 );
  
  // Initialize radio and enable receive callback function
  setup_radio( process_rx );
  
  // Full Power
  WriteSinglePATable(0xC0);
  
  // Enable interrupts, otherwise nothing will work
  eint();
   
  while (1)
  {
    // Enter sleep mode
    __bis_SR_register( LPM3_bits + GIE );
    __no_operation();
    
    if ( new_message )
    {
      volatile uint16_t delay;
      
      for( delay=0x9fff; delay > 0; delay-- )
      {
        __no_operation();
      }
      
      radio_tx( tx_buffer, sizeof(packet_header_t) + sizeof(packet_data_t) );
      led2_toggle();    
    }
    
    new_message = 0;
        
  }
  
  return 0;
}

/*******************************************************************************
 * @fn     uint8_t heartbeat()
 * @brief  TODO
 * ****************************************************************************/
uint8_t heartbeat()
{
  
  led1_toggle();
   
  return 1;
}

/*******************************************************************************
 * @fn     uint8_t process_rx( uint8_t* buffer, uint8_t size )
 * @brief  callback function called when new message is received
 * ****************************************************************************/
uint8_t process_rx( uint8_t* buffer, uint8_t size )
{
  packet_header_t* header;
  header = (packet_header_t*)(buffer);

  //packet_footer_t* footer;
  // Add one to account for the byte with the packet length
  //footer = (packet_footer_t*)(buffer + header->length + 1 );
  
  // Erase buffer just for fun
  //memset( buffer, 0x00, size );
  
  led3_toggle();
  if( header->type == 0xAA )
  {
    memcpy( tx_buffer, buffer, sizeof(packet_header_t) + sizeof(packet_data_t) );  
    new_message = 1;
  }
  
  
  return 1;
}

