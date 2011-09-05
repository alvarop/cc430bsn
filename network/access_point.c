/** @file access_point.c
*
* @brief Network Discovery Access Point
*   
* The main idea for this application is to discover all end devices (EDs) on the
* network and characterize all the links between them. Once all links have been
* characterized, new routing methods can be used.
* 
* The access point (AP) will periodically send a sync packet at full power.
* After receiving the sync packet, each ED will wait an ammount of time
* proportional to its network address (to avoid collisions) and reply with an
* acknowledge packet. The acknowledge packet will also contain a table of RSSI
* values from all other devices in the network.
*
* The acknowledge packet will be a broadcast message so that all other EDs can 
* 'listen in' and record the RSSI.
*
* The AP will then send the collected data to the host computer for processing.
* (Eventually, all processing will be done on the AP itself, but for now, it 
* will offline.)
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

#undef DEVICE_ADDRESS
#define DEVICE_ADDRESS (MAX_DEVICES + 1)

#if DEBUG_ON
#define debug(x) uart_write( x, strlen(x) )
#define dbg(x) uart_put_char(x)
// DEBUG for printing statements
uint8_t string[100];
#warning Debugging functions enabled. Regular output disabled.
#else
#define debug(x) // Disabled debug(x)
#define dbg(x) // Disabled dbg(x)
#endif

static uint8_t sync_message (void);
static uint8_t scheduler (void);
static uint8_t rx_callback( uint8_t*, uint8_t );
static uint8_t serial_callback( uint8_t );

#define FIRST_COMMAND '0'

static void command_null(void);
static void (* const serial_commands[])(void) = { command_null, command_null,
                                                  command_null, command_null };

// These are the different states the device can be in
#define STATE_WAIT              (0x00)
#define STATE_PROCESS_PACKET    (0x01)
#define STATE_PROCESS_COMMAND   (0x02)
#define STATE_SEND_DATA         (0x03)
#define STATE_BUILD_SYNC        (0x04)

// Total number of states, used for bounds checking before calling functions
#define TOTAL_STATES            (0x05)

static uint8_t state_wait(void);
static uint8_t state_process_packet(void);
static uint8_t state_command(void);
static uint8_t state_send_data(void);
static uint8_t state_build_sync(void);

// Functions called for each processor state
static uint8_t (* const states[])(void) = { state_wait, state_process_packet,
                                            state_command, state_send_data,
                                            state_build_sync };

static volatile uint8_t counter = TIMER_CYCLES;

// Local radio buffers
static uint8_t p_radio_tx_buffer[RADIO_BUFFER_SIZE];
static uint8_t p_radio_rx_buffer[RADIO_BUFFER_SIZE];

static uint8_t p_serial_rx_buffer[RADIO_BUFFER_SIZE];

// Table with all the RSSIs from all devices. This is sent back to the host.
static volatile uint8_t rssi_table[MAX_DEVICES+1][MAX_DEVICES+1];

// Table storing all device transmit powers
static volatile uint8_t power_table[MAX_DEVICES];

// Table storing all device routes
static volatile uint8_t routing_table[MAX_DEVICES];

// Table listing all devices that responded in the current cycle
static volatile uint8_t device_table[MAX_DEVICES+1];

// Variable to hold the current state of the processor
static volatile uint8_t current_state = STATE_WAIT;

static volatile uint8_t last_packet_size;
static volatile uint8_t last_serial_command;

// Flag to notify if we receive a packet while another one is being processed
// I have not yet implemented a solution for this problem
static volatile uint8_t processing_packet = 0;

packet_header_t* p_tx_packet = (packet_header_t*)p_radio_tx_buffer; 

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
  setup_uart_callback( serial_callback );  
  setup_spi();

  // Initialize routing and power tables
  memset( (uint8_t*)power_table, 0xFF, sizeof(power_table) );
  memset( (uint8_t*)routing_table, DEVICE_ADDRESS, sizeof(routing_table) );

  // Setup sync packet
  p_tx_packet->destination = 0x00;        // Broadcast
  p_tx_packet->source = DEVICE_ADDRESS;
  p_tx_packet->type = PACKET_SYNC;

  setup_timer_a(MODE_CONTINUOUS);
  
  register_timer_callback( scheduler, 1 );
  register_timer_callback( sync_message, 0 );

  set_ccr( 1, 32768 );  
  set_ccr( 0, 0 );
  
  setup_cc2500( rx_callback );  
  cc2500_set_address( DEVICE_ADDRESS );
  cc2500_disable_addressing();
       
  setup_leds();
  
  // Initialize all devices to 'disconnected'
  memset( (void*)device_table, 0x00, sizeof(device_table) );  
            
  // Enable interrupts
  eint();
  
  __no_operation();
  
  dbg('~');
  
  for (;;) 
  {
    dbg('c');
    // Check that we are in a valid state
    if ( current_state >= TOTAL_STATES )
    {
      // If not, default to waiting
      current_state = STATE_WAIT;
    }
    
    // Call the current state function and put the processor to sleep if needed
    if( 0 == states[current_state]() )
    {
      dbg('<');
      __bis_SR_register( LPM1_bits + GIE );   // Sleep
    }
    
  } 
  
  return 0;
}

/*******************************************************************************
 * @fn     uint8_t scheduler( void )
 * @brief  timer_a callback function, takes care of time-related functions
 * ****************************************************************************/
static uint8_t scheduler (void)
{
  dbg('s');
  // Count down TIMER_CYCLES between sync messages
  if( 0 == counter-- )
  {
    counter = TIMER_CYCLES;
  }
  
  // Send RSSI table to host
  if( counter == TIME_TX_RSSI )
  {
    current_state = STATE_SEND_DATA;
    return 1;
  }
  dbg('S');
  return 0;
}

/*******************************************************************************
 * @fn     uint8_t send_sync_message( void )
 * @brief  Send sync message to keep ED clocks aligned
 * ****************************************************************************/
static uint8_t sync_message (void)
{
  dbg('.');
  if (TIMER_CYCLES == counter)
  {
    dbg('|');
    cc2500_tx_packet( &p_radio_tx_buffer[1], 
      ( sizeof(packet_header_t) + sizeof(routing_table) + sizeof(power_table) ), 
      p_tx_packet->destination );
      
      dbg('M');
    
    //led1_toggle();
  }
  else if ( 1 == counter )
  {
    current_state = STATE_BUILD_SYNC;
    dbg('m');
    return 1;
  }
  
    //debug("Sync sent\r\n");
  return 0;
}

/*******************************************************************************
 * @fn     uint8_t state_wait(void)
 * @brief  Function for state STATE_WAIT. Puts processor to sleep.
 * ****************************************************************************/
static uint8_t state_wait(void)
{
  // Do nothing
  dbg('w');
  // Return 0 to put the processor to sleep
  return 0;
}

/*******************************************************************************
 * @fn     static uint8_t state_send_date(void)
 * @brief  Send data through serial port
 * ****************************************************************************/
static uint8_t state_send_data(void)
{
  dbg('d');
#if DEBUG_ON == 0
  uart_write_escaped((uint8_t*)rssi_table, sizeof(rssi_table));
#endif
    
  //debug("Dump Table\r\n");
  
    
  // clean rssi table
  memset( (uint8_t*)rssi_table, MIN_RSSI, sizeof(rssi_table) );
  dbg('D');
  // Return 0 to put the processor to sleep
  return 0;
}

/*******************************************************************************
 * @fn     uint8_t state_process_packet(void)
 * @brief  Function for state STATE_WAIT. Puts processor to sleep.
 * ****************************************************************************/
static uint8_t state_process_packet(void)
{
  packet_header_t* rx_packet;
  dbg('p');
  //led2_on();
  
  dint();
  processing_packet = 1;
  eint();
             
  rx_packet = (packet_header_t*)p_radio_rx_buffer;

  if ( (PACKET_SYNC | FLAG_ACK) == rx_packet->type )
  { 
    
    // Make sure source and origin are within bounds
    if ( ( ( rx_packet->source <= ( MAX_DEVICES ) )  
                                && ( rx_packet->source > 0 ) )
        && ( ( rx_packet->origin <= ( MAX_DEVICES ) ) 
                                && ( rx_packet->origin > 0 ) ) )
                             
    {
      // Store RSSI in table
      rssi_table[AP_INDEX][rx_packet->source] = 
                                          p_radio_rx_buffer[last_packet_size];
                                      
      // Copy RSSI table from packet to master table
      memcpy( (uint8_t*)rssi_table[rx_packet->origin], 
              &p_radio_rx_buffer[sizeof(packet_header_t)], (MAX_DEVICES + 1) );
      
      // Since the device replied to the poll, we can assume it is 'active'
      device_table[rx_packet->origin] |= DEVICE_ACTIVE;
            
    }
    else
    {
      // device out of bounds, ignore
    }

  }

  // Return to waiting state
  current_state = STATE_WAIT;

  // Clear the buffer after use
  memset( p_radio_rx_buffer, 0x00, sizeof(last_packet_size) );       

  dint();
  processing_packet = 0;
  eint();

  led2_off();
  dbg('P');
  // Return 0 to put the processor to sleep
  return 0;
}
       
/*******************************************************************************
 * @fn     uint8_t state_command(void)
 * @brief  Function for STATE_COMMAND. Runs function for associated command
 * ****************************************************************************/
static uint8_t state_command(void)
{
  dbg('c');
  // Run function associated with last command
  serial_commands[last_serial_command - FIRST_COMMAND]();
  
  // Return to waiting state
  current_state = STATE_WAIT;
  dbg('C');
  // Return 0 to put the processor to sleep
  return 0;
}

/*******************************************************************************
 * @fn     uint8_t state_build_sync(void)
 * @brief  Function for state STATE_BUILD_SYNC. Prepares sync packet.
 * ****************************************************************************/
static uint8_t state_build_sync(void)
{
  dbg('b');
  // Build sync message
  memcpy( &p_radio_tx_buffer[sizeof(packet_header_t)], 
           (uint8_t*)routing_table, sizeof(routing_table) );
                    
  memcpy( &p_radio_tx_buffer[sizeof(packet_header_t) + sizeof(routing_table)], 
           (uint8_t*)power_table, sizeof(power_table) );
  
  // Return to waiting state
  current_state = STATE_WAIT;
  
  // Return 0 to put the processor to sleep
  return 0;
}

/*******************************************************************************
 * @fn     void rx_callback( void )
 * @brief  rx callback function, called when packet has been received
 * ****************************************************************************/
static uint8_t rx_callback( uint8_t* p_buffer, uint8_t size )
{ 
  dbg('r');
  if( processing_packet == 1 )
  {
    //print("err pp\r\n");
    // This means that another packet is still being processed
  }
  
  led2_on();

  // Copy received message to local rx buffer. ( size+1 accounts for RSSI byte )
  // NOTE: Could just point to the radio library buffer instead, but would need 
  // to worry about overwriting it while it's still being processed
  memcpy( p_radio_rx_buffer, p_buffer, ( size + 1 ) );
  
  // Since the packet size is not in the received packet, we need to save it for
  // later use
  last_packet_size = size;
  
  // Change the current state so that the packet will be processed after the ISR
  current_state = STATE_PROCESS_PACKET;
  
  
  dbg('R');
  // Return 1 to wake-up processor after ISR
  return 1;
}

/*******************************************************************************
 * @fn     void serial_callback( uint8_t command )
 * @brief  Receive serial packets, decode them, and then process them
 * ****************************************************************************/
static uint8_t serial_callback( uint8_t rx_byte )
{
  static uint8_t receiving_packet;
  static uint8_t escape_next_character;
  static uint8_t buffer_index;
  
  if( receiving_packet )
  {
    if( escape_next_character )
    {
      escape_next_character = 0;
      p_serial_rx_buffer[buffer_index++] = rx_byte ^ 0x20;
    }
    else if ( ESCAPE_BYTE == rx_byte )
    {
      escape_next_character = 1;    
    }
    else if ( SYNC_BYTE == rx_byte )
    {      
      
      if( buffer_index == ( sizeof(routing_table) + sizeof(power_table) ) )
      {
        memcpy( (uint8_t*)routing_table, &p_serial_rx_buffer[0], 
                                                      sizeof(routing_table) );
        
        memcpy( (uint8_t*)power_table, 
                                    &p_serial_rx_buffer[sizeof(routing_table)], 
                                                        sizeof(power_table) );
        
        
      }
      else
      {
        led1_toggle();
      }
      
      receiving_packet = 0;
      buffer_index = 0;
    }
    else
    {
      p_serial_rx_buffer[buffer_index++] = rx_byte;
    }
  }
  else if ( SYNC_BYTE == rx_byte)
  {
    receiving_packet = 1;
  }
  else
  {
    //led1_toggle();
    buffer_index = 0;
  }

  // Make sure the buffer doesn't overflow
  if( buffer_index == RADIO_BUFFER_SIZE )
  {
    buffer_index = 0;
    receiving_packet = 0;
  }
  dbg('?');
return 0;
#if 0  
  // Check to be sure the command is in range
  if( ( command >= FIRST_COMMAND ) && 
      ( command < ( FIRST_COMMAND + sizeof(serial_commands)/2 ) ) )
  {
    // Change the current state so that the command will be run after the ISR
    current_state = STATE_PROCESS_COMMAND;
    
    // Save the command
    last_serial_command = command;
    
    // Return 1 to wake-up processor after ISR
    return 1;
  }
  else
  {
    // Invalid command    
    // No need to wake up processor with invalid commands
    return 0;
  }
#endif
  
}

/*******************************************************************************
 * @fn     void command_null(void)
 * @brief  pointer for commands that have not been implemented
 * ****************************************************************************/
static void command_null(void)
{
  dbg('n');
  // Do nothing
}

