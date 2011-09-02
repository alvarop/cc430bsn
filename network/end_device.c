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

#if DEVICE_ADDRESS > MAX_DEVICES
#warning Device address out of bounds.
#endif

static uint8_t scheduler (void);
static uint8_t rx_callback( uint8_t*, uint8_t );

static uint8_t p_radio_tx_buffer[RADIO_BUFFER_SIZE];

static volatile uint8_t counter = TIMER_CYCLES;
static volatile uint8_t ack_required = 0;
static volatile uint8_t ack_time = 0;

// Array to store all RSSIs from other devices
static volatile uint8_t rssi_table[MAX_DEVICES+1];
static volatile uint8_t final_rssi_table[MAX_DEVICES+1];

// Table storing all device transmit powers
static volatile uint8_t power_table[MAX_DEVICES];

// Table storing all device routes
static volatile uint8_t routing_table[MAX_DEVICES];

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
  register_timer_callback( scheduler, 1 );
  set_ccr( 1, 32768 );
  
  setup_cc2500( rx_callback );
  cc2500_set_address( DEVICE_ADDRESS );
  cc2500_disable_addressing();
       
  setup_leds();
           
  for (;;) 
  {        
    __bis_SR_register( LPM1_bits + GIE );   // Sleep
    __no_operation();    
  } 
  
  return 0;
}

/*******************************************************************************
 * @fn     uint8_t scheduler( void )
 * @brief  timer_a callback function, takes care of time-related functions
 * ****************************************************************************/
static uint8_t scheduler (void)
{

  if( 0 == counter-- )
  {
    //led1_toggle();
    counter = TIMER_CYCLES;        
  }
  
  if( ack_required && (counter == ack_time) )
  {
    packet_header_t* p_tx_packet;  

    led1_on();

    p_tx_packet = (packet_header_t*)p_radio_tx_buffer;
    
    // Setup ack packet
    p_tx_packet->destination = routing_table[DEVICE_ADDRESS - 1];
    p_tx_packet->source = DEVICE_ADDRESS;
    p_tx_packet->origin = DEVICE_ADDRESS;
    p_tx_packet->type = PACKET_SYNC | FLAG_ACK;
    
    // Send RSSI table back to AP
    memcpy( &p_radio_tx_buffer[sizeof(packet_header_t)], 
                      (uint8_t*)final_rssi_table, sizeof(final_rssi_table) );
       
    cc2500_tx_packet( &p_radio_tx_buffer[1], 
        ( sizeof(packet_header_t) + sizeof(final_rssi_table) ),
                                    p_tx_packet->destination );
                                                
    led1_off();   
    
    ack_required = 0;
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
  
  //led2_toggle();
  
  // TODO Packet handling is temporary for testing purposes
  // Later implementation will use function pointers to avoid all the if-else
  // and have constant time to start processing each packet
  
  if ( PACKET_SYNC == p_rx_packet->type )
  {
    // Reset timer_a
    clear_timer();
    
    counter = TIMER_CYCLES;
    
    ack_required = 1;
    ack_time = TIMER_CYCLES - (DEVICE_ADDRESS);
    
    // Store routing table
    memcpy(  (uint8_t*)routing_table, 
            &p_buffer[sizeof(packet_header_t)], sizeof(routing_table) );
    
    // Store power table
    memcpy(  (uint8_t*)power_table, 
            &p_buffer[sizeof(packet_header_t) + sizeof(routing_table)], 
            sizeof(power_table) );
    
    // Save current table so we can send it later
    memcpy( (uint8_t*)final_rssi_table, (uint8_t*)rssi_table, 
                                                  sizeof(final_rssi_table) );

    // Update transmit power from table
    cc2500_set_power( power_table[DEVICE_ADDRESS - 1] );
    
    // clean rssi table
    memset( (uint8_t*)rssi_table, MIN_RSSI, sizeof(rssi_table) );
       
    // Store AP RSSI in table
    rssi_table[AP_INDEX] = p_buffer[size]; 
  }     
  // Receive other EDs PACKET_SYNC reply and save the RSSI in the table
  else if ( (PACKET_SYNC | FLAG_ACK) == p_rx_packet->type )
  { 
    // Make sure source is within bounds
    if ( ( p_rx_packet->source <= ( MAX_DEVICES ) )  
                             && ( p_rx_packet->source > 0) )
    {
      // Store RSSI in table
      rssi_table[p_rx_packet->source] = p_buffer[size];
      
      // If this packet was addressed to this device, relay it
      if( DEVICE_ADDRESS == p_rx_packet->destination )
      {
        // Send the packet to the appropriate destination
        p_rx_packet->destination = routing_table[DEVICE_ADDRESS - 1];
        
        // TODO add delay here?
        __delay_cycles(100);
        
        // Relay message
        cc2500_tx_packet( &p_buffer[1], size, p_rx_packet->destination );
        
      }
    }
  }
  
  return 0;
}

