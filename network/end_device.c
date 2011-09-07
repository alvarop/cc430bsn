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

// These are the different states the device can be in
#define STATE_WAIT              (0x00)
#define STATE_PROCESS_PACKET    (0x01)

// Total number of states, used for bounds checking before calling functions
#define TOTAL_STATES            (0x02)

static uint8_t state_wait(void);
static uint8_t state_process_packet(void);

// Variable to hold the current state of the processor
static volatile uint8_t current_state = STATE_WAIT;

// Functions called for each processor state
static uint8_t (* const states[])(void) = { state_wait, state_process_packet };

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

static volatile uint8_t last_packet_size;
static volatile uint8_t p_rx_buffer[RADIO_BUFFER_SIZE];

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
  
    // Check that we are in a valid state
    if ( current_state >= TOTAL_STATES )
    {
      // If not, default to waiting
      current_state = STATE_WAIT;
    }
    
    // Call the current state function and put the processor to sleep if needed
    if( 0 == states[current_state]() )
    {
      __bis_SR_register( LPM1_bits + GIE );   // Sleep
    }
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
 * @fn     uint8_t state_wait(void)
 * @brief  Function for state STATE_WAIT. Puts processor to sleep.
 * ****************************************************************************/
static uint8_t state_wait(void)
{
  // Do nothing
  // Return 0 to put the processor to sleep
  return 0;
}

/*******************************************************************************
 * @fn     uint8_t state_process_packet(void)
 * @brief  Function for state STATE_WAIT. Puts processor to sleep.
 * ****************************************************************************/
static uint8_t state_process_packet(void)
{
  
  packet_header_t* p_rx_packet;
  
  
  p_rx_packet = (packet_header_t*)p_rx_buffer;
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
    ack_time = TIMER_CYCLES - ( DEVICE_ADDRESS * TIME_SCALE_FACTOR );
    
    // Store routing table
    memcpy(  (uint8_t*)routing_table, 
      (uint8_t *)&p_rx_buffer[sizeof(packet_header_t)], sizeof(routing_table) );
    
    // Store power table
    memcpy(  (uint8_t*)power_table, 
      (uint8_t *)&p_rx_buffer[sizeof(packet_header_t) + sizeof(routing_table)], 
            sizeof(power_table) );
    
    // Save current table so we can send it later
    memcpy( (uint8_t*)final_rssi_table, (uint8_t*)rssi_table, 
                                                  sizeof(final_rssi_table) );

    // Update transmit power from table
    cc2500_set_power( power_table[DEVICE_ADDRESS - 1] );
    
    // clean rssi table
    memset( (uint8_t*)rssi_table, MIN_RSSI, sizeof(rssi_table) );
       
    // Store AP RSSI in table
    rssi_table[AP_INDEX] = p_rx_buffer[last_packet_size]; 
  }     
  // Receive other EDs PACKET_SYNC reply and save the RSSI in the table
  else if ( (PACKET_SYNC | FLAG_ACK) == p_rx_packet->type )
  { 

    // Make sure source is within bounds
    if ( ( p_rx_packet->source <= ( MAX_DEVICES ) )  
                             && ( p_rx_packet->source > 0) )
    {
      // Store RSSI in table
      rssi_table[p_rx_packet->source] = p_rx_buffer[last_packet_size];
      
      // If this packet was addressed to this device, relay it
      if( DEVICE_ADDRESS == p_rx_packet->destination )
      {
        led2_on();
        
        // Send the packet to the appropriate destination
        p_rx_packet->destination = routing_table[DEVICE_ADDRESS - 1];
        
        // TODO add delay here?
        __delay_cycles(100);
        
        // Relay message
        cc2500_tx_packet( (uint8_t *)&p_rx_buffer[1], last_packet_size, 
                                                    p_rx_packet->destination );
        
        led2_off();
        
      }
    }
  }

  
  // Return 0 to put the processor to sleep
  return 0;
}

/*******************************************************************************
 * @fn     void rx_callback( void )
 * @brief  rx callback function, called when packet has been received
 * ****************************************************************************/
static uint8_t rx_callback( uint8_t* p_buffer, uint8_t size )
{    
  
  // Copy received message to local rx buffer. ( size+1 accounts for RSSI byte )
  // NOTE: Could just point to the radio library buffer instead, but would need 
  // to worry about overwriting it while it's still being processed
  memcpy( p_rx_buffer, p_buffer, ( size + 1 ) );
  
  last_packet_size = size;
  
  // Change the current state so that the packet will be processed after the ISR
  current_state = STATE_PROCESS_PACKET;
  
  return 1;
}

