/** @file access_point.c
*
* @brief Network Discovery Access Point
*   
* The main idea for this application is to discover all end devices (EDs) on the
* network and characterize all the links between them. Once all links have been
* characterized, new routing methods can be used.
* 
* Initially, the access point (AP) will begin by sending a beacon at full power.
* After receiving the start beacon, each ED will wait an ammount of time
* proportional to its network address (to avoid collisions) and reply with an
* acknowledge packet.
*
* The acknowledge packet will be a broadcast message so that all other EDs can 
* 'listen in' and record the RSSI.
*
* Once all EDs have replied, the AP will poll each ED for its received power
* table. This table will include a device address and received RSSI for all
* devices it was able to receive messages from.
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

#define FIRST_COMMAND '0'

#define print(x) uart_write( x, strlen(x) )

static uint8_t blink_led1 (void);
static uint8_t rx_callback( uint8_t*, uint8_t );
static uint8_t run_serial_command( uint8_t );

static void command_0(void);
static void command_1(void);
static void command_null(void);

static void (* const serial_commands[])(void) = { command_0, command_1,
                                                  command_null, command_null };

static uint8_t p_radio_tx_buffer[RADIO_BUFFER_SIZE];

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
  
  register_timer_callback( blink_led1, 1 );

  set_ccr( 1, 0 );
  
  setup_cc2500( rx_callback );
  
  cc2500_set_address( DEVICE_ADDRESS );
       
  setup_leds();  
           
  for (;;) 
  {        
    
    print("\r\n\nEnter command from ");
    uart_put_char( FIRST_COMMAND );
    print(" to ");
    uart_put_char( FIRST_COMMAND + sizeof(serial_commands)/2 - 1 );
    print(": ");
    
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

  return 0;
}

/*******************************************************************************
 * @fn     void run_serial_command( uint8_t command )
 * @brief  Run serial commands...
 * ****************************************************************************/
static uint8_t run_serial_command( uint8_t command )
{
  print("\r\n");
  
  // Check to be sure the command is in range
  if( ( command >= FIRST_COMMAND) && 
      ( command < ( FIRST_COMMAND + sizeof(serial_commands)/2 ) ) )
  {
    serial_commands[command - FIRST_COMMAND]();
  }
  else
  {
    print("Invalid command!\r\n");
  }
  
  return 1;
}

/*******************************************************************************
 * @fn     void run_serial_command( uint8_t command )
 * @brief  Send START packet to initialize network discovery
 * ****************************************************************************/
static void command_0(void)
{
  packet_header_t* start_packet;
  
  start_packet = (packet_header_t*)p_radio_tx_buffer;
  
  // Initialize START packet
  start_packet->source = DEVICE_ADDRESS;
  start_packet->destination = BROADCAST_ADDRESS;
  start_packet->type = PACKET_START;
  
  // Send START packet
  cc2500_tx_packet( &p_radio_tx_buffer[1], 2, start_packet->destination );
    
  print("Starting network discovery...\r\n");
      
  // Wakeup processor after interrupt  
}

/*******************************************************************************
 * @fn      void command_1(void)
 * @brief   Send sync message
 * ****************************************************************************/
static void command_1(void)
{
  packet_header_t* start_packet;
  
  start_packet = (packet_header_t*)p_radio_tx_buffer;
  
  // Initialize START packet
  start_packet->destination = BROADCAST_ADDRESS;
  start_packet->source = DEVICE_ADDRESS;
  start_packet->type = PACKET_SYNC;
  
  // Send START packet
  // cc2500_tx_packet already adds the destination field
  cc2500_tx_packet( &p_radio_tx_buffer[1], 2, start_packet->destination );
  
  clear_timer();
  led1_on();
  
  print("Sending sync message\r\n");
}

/*******************************************************************************
 * @fn     void command_null(void)
 * @brief  pointer for commands that have not been implemented
 * ****************************************************************************/
static void command_null(void)
{
  // Start network discovery
  print("This command has not been implemented yet\r\n");
}

