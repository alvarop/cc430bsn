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

#ifndef DEVICE_ADDRESS
#define DEVICE_ADDRESS 0xFF
#endif

static uint8_t blink_led1 (void);
static uint8_t rx_callback( uint8_t*, uint8_t );

static uint8_t p_radio_tx_buffer[RADIO_BUFFER_SIZE];

static volatile uint8_t counter = LED_BLINK_CYCLES;
static volatile uint8_t ack_required = 0;
static volatile uint8_t ack_time = 0;

// Array to store all RSSIs from other devices
static volatile uint8_t rssi_table[MAX_DEVICES+1];

void debug_status();

int main( void )
{   
  
  /* Init watchdog timer to off */
  WDTCTL = WDTPW|WDTHOLD;
     
  // Setup oscillator for 12MHz operation
  BCSCTL1 = CALBC1_12MHZ;
  DCOCTL = CALDCO_12MHZ; 
  
  // Wait for changes to take effectif ( PACKET_SYNC == ( start_packet->type & TYPE_MASK ) )
  {
    // Reset timer_a
    clear_timer();
    counter = LED_BLINK_CYCLES;
    led1_on();
  }
  __delay_cycles(4000);
      
  setup_uart();
  
  setup_spi();
  
  setup_timer_a(MODE_CONTINUOUS);
  
  register_timer_callback( blink_led1, 1 );

  set_ccr( 1, 32768 );
  
  setup_cc2500( rx_callback );
  
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
 * @fn     uint8_t blink_led1( void )
 * @brief  timer_a callback function, blinks LED every LED_BLINK_CYCLES
 * ****************************************************************************/
static uint8_t blink_led1 (void)
{

  if( 0 == counter-- )
  {
    //led1_toggle();
    counter = LED_BLINK_CYCLES;        
  }
  
  if( ack_required && (counter == ack_time) )
  {
    uint8_t ack_packet[] = {3, 0, DEVICE_ADDRESS, PACKET_START | FLAG_ACK};
    cc2500_tx( ack_packet, 4 );
    ack_required = 0;
    led1_off();
  }
  
  return 0;
}

/*******************************************************************************
 * @fn     void rx_callback( void )
 * @brief  rx callback function, called when packet has been received
 * ****************************************************************************/
static uint8_t rx_callback( uint8_t* p_buffer, uint8_t size )
{    
  packet_header_t* p_rx_packet;
  
  p_rx_packet = (packet_header_t*)p_buffer;
  
  led2_toggle();
  
  // TODO Packet handling is temporary for testing purposes
  // Later implementation will use function pointers to avoid all the if-else
  // and have constant time to start processing each packet
  
  if ( PACKET_SYNC == ( p_rx_packet->type & TYPE_MASK ) )
  {
    // Reset timer_a
    clear_timer();
  }
  
  if ( PACKET_POLL == ( p_rx_packet->type & TYPE_MASK ) )
  {
    packet_header_t* p_tx_packet;  

    p_tx_packet = (packet_header_t*)p_radio_tx_buffer;
    
    // Setup ack packet
    p_tx_packet->destination = p_rx_packet->source;
    p_tx_packet->source = DEVICE_ADDRESS;
    p_tx_packet->type = PACKET_POLL | FLAG_ACK;
    
    // Send RSSI table back to AP
    memcpy(&p_radio_tx_buffer[sizeof(packet_header_t)], (uint8_t*)rssi_table, 
                                                          sizeof(rssi_table));
    // Delay before sending reply
    // Worked with a delay of 55, but using 100 to be safe in case the preceding
    // code becomes more efficient.
    __delay_cycles(100);
       
    cc2500_tx_packet(&p_radio_tx_buffer[1], (2 + sizeof(rssi_table) ),
                                                p_tx_packet->destination );
       
    led1_on();
  }
  
  // Not masking the packet type so that PACKET_START ACK packets don't clear
  // other device timers.
  if ( PACKET_START == ( p_rx_packet->type ) )
  {
    // Reset timer_a
    clear_timer();
    counter = LED_BLINK_CYCLES;
    //led1_on();
    ack_required = 1;
    ack_time = LED_BLINK_CYCLES - (2 * DEVICE_ADDRESS);
    
  }
  
  // Receive other EDs PACKET_START reply and save the RSSI in the table
  if ( (PACKET_START | FLAG_ACK) == p_rx_packet->type )
  { 
    // Make sure source is within bounds
    if ( ( p_rx_packet->source <= ( MAX_DEVICES - 1 ) )  
                             && ( p_rx_packet->source > 0) )
    {
      // Store RSSI in table
      rssi_table[p_rx_packet->source] = p_buffer[size];    
    }
  }
  
  return 0;
}

