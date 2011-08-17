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

#define DEVICE_ADDRESS (MAX_DEVICES + 1)

#define print(x) uart_write( x, strlen(x) )

static uint8_t sync_message (void);
static uint8_t scheduler (void);
static uint8_t rx_callback( uint8_t*, uint8_t );
static uint8_t run_serial_command( uint8_t );

#define FIRST_COMMAND '0'

static void command_null(void);
static void (* const serial_commands[])(void) = { command_null, command_null,
                                                  command_null, command_null };

// These are the different states the device can be in
#define STATE_WAIT              (0x00)
#define STATE_PROCESS_PACKET    (0x01)
#define STATE_PROCESS_COMMAND   (0x02)

// Total number of states, used for bounds checking before calling functions
#define TOTAL_STATES            (0x03)

static uint8_t state_wait(void);
static uint8_t state_process_packet(void);
static uint8_t state_command(void);

// Functions called for each processor state
static uint8_t (* const states[])(void) = { state_wait, state_process_packet,
                                                  state_command };

static volatile uint8_t counter = TIMER_CYCLES;

// Local radio buffers
static uint8_t p_radio_tx_buffer[RADIO_BUFFER_SIZE];
static uint8_t p_radio_rx_buffer[RADIO_BUFFER_SIZE];

// Table with all the RSSIs from all devices. This is sent back to the host.
static volatile uint8_t rssi_table[MAX_DEVICES+1][MAX_DEVICES+1];

// Table listing all devices that responded in the current cycle
static volatile uint8_t device_table[MAX_DEVICES+1];

// Variable to hold the current state of the processor
static volatile uint8_t current_state = STATE_WAIT;

static volatile uint8_t last_packet_size;
static volatile uint8_t last_serial_command;

// Flag to notify if we receive a packet while another one is being processed
// I have not yet implemented a solution for this problem
static volatile uint8_t processing_packet = 0;

// Instead of re-building the sync packet every time, just save it in memory
static const uint8_t sync_packet[] = {0x03, 0x00, DEVICE_ADDRESS, PACKET_SYNC};

// DEBUG for printing statements
uint8_t string[100];

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
  setup_uart_callback( run_serial_command );  
  setup_spi();
  setup_timer_a(MODE_CONTINUOUS);
  
  register_timer_callback( scheduler, 1 );
  register_timer_callback( sync_message, 0 );

  set_ccr( 1, 32768 );  
  set_ccr( 0, 0 );
  
  setup_cc2500( rx_callback );  
  cc2500_set_address( DEVICE_ADDRESS );
       
  setup_leds();
  
  // Initialize all devices to 'disconnected'
  memset( (void*)device_table, 0x00, sizeof(device_table) );  
            
  // Enable interrupts
  eint();
  
  __no_operation();
  
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
    
  } 
  
  return 0;
}

/*******************************************************************************
 * @fn     uint8_t scheduler( void )
 * @brief  timer_a callback function, takes care of time-related functions
 * ****************************************************************************/
static uint8_t scheduler (void)
{
  // Count down TIMER_CYCLES between sync messages
  if( 0 == counter-- )
  {
    counter = TIMER_CYCLES;
  }
  
  // Send RSSI table to host
  if( counter == TIME_TX_RSSI )
  {
    //print("tx");
    uart_write_escaped((uint8_t*)rssi_table, sizeof(rssi_table));
    
    // clean rssi table
    memset( (uint8_t*)rssi_table, MIN_RSSI, sizeof(rssi_table) );
  }
  
  return 0;
}

/*******************************************************************************
 * @fn     uint8_t send_sync_message( void )
 * @brief  Send sync message to keep ED clocks aligned
 * ****************************************************************************/
static uint8_t sync_message (void)
{
  if (TIMER_CYCLES == counter)
  {
    // Transmit sync message and reset flag
    cc2500_tx( (uint8_t *)sync_packet, 4 );
    
    led1_toggle();
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
  packet_header_t* rx_packet;
  
  led2_on();
  
  dint();
  processing_packet = 1;
  eint();
             
  rx_packet = (packet_header_t*)p_radio_rx_buffer;

  if ( (PACKET_SYNC | FLAG_ACK) == rx_packet->type )
  { 
    
    // Make sure source is within bounds
    if ( ( rx_packet->source <= ( MAX_DEVICES ) )  
                             && ( rx_packet->source > 0) )
    {
      // Store RSSI in table
      rssi_table[AP_INDEX][rx_packet->source] = 
                                          p_radio_rx_buffer[last_packet_size];
                                      
      // Copy RSSI table from packet to master table
      memcpy( (uint8_t*)rssi_table[rx_packet->source], 
              &p_radio_rx_buffer[sizeof(packet_header_t)], (MAX_DEVICES + 1) );
      
      // Since the device replied to the poll, we can assume it is 'active'
      device_table[rx_packet->source] |= DEVICE_ACTIVE;
            
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

  // Return 0 to put the processor to sleep
  return 0;
}
       
/*******************************************************************************
 * @fn     uint8_t state_command(void)
 * @brief  Function for STATE_COMMAND. Runs function for associated command
 * ****************************************************************************/
static uint8_t state_command(void)
{
  // Run function associated with last command
  serial_commands[last_serial_command - FIRST_COMMAND]();
  
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
  if( processing_packet == 1 )
  {
    //print("err pp\r\n");
    // This means that another packet is still being processed
  }

  // Copy received message to local rx buffer. ( size+1 accounts for RSSI byte )
  // NOTE: Could just point to the radio library buffer instead, but would need 
  // to worry about overwriting it while it's still being processed
  memcpy( p_radio_rx_buffer, p_buffer, ( size + 1 ) );
  
  // Since the packet size is not in the received packet, we need to save it for
  // later use
  last_packet_size = size;
  
  // Change the current state so that the packet will be processed after the ISR
  current_state = STATE_PROCESS_PACKET;
  
  // Return 1 to wake-up processor after ISR
  return 1;
}

/*******************************************************************************
 * @fn     void run_serial_command( uint8_t command )
 * @brief  Run serial commands...
 * ****************************************************************************/
static uint8_t run_serial_command( uint8_t command )
{
  
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
  
}

/*******************************************************************************
 * @fn     void command_null(void)
 * @brief  pointer for commands that have not been implemented
 * ****************************************************************************/
static void command_null(void)
{
  // Do nothing
}

