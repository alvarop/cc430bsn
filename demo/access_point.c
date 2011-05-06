/** @file access_point.c
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
#include "uart.h"
#include "timers.h"
#include "radio.h"

uint8_t tx_buffer[PACKET_LEN+1];

uint8_t print_buffer[200];

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

uint8_t send_sync_message();
uint8_t process_rx( uint8_t*, uint8_t );

int main( void )
{
  uint8_t buffer_index;
  packet_header_t* header;

  // Stop watchdog timer to prevent time out reset
  WDTCTL = WDTPW + WDTHOLD;
  
  header = (packet_header_t*)tx_buffer;
  
  // Initialize Tx Buffer
  header->length = sizeof(packet_header_t) - 1;
  header->source = DEVICE_ADDRESS;
  header->type = 0x66; // Sync message
  header->flags = 0xAA;
  
  // Make sure processor is running at 12MHz
  setup_oscillator();
  
  // Initialize UART for communications at 115200baud
  setup_uart();
   
  // Initialize LEDs
  setup_leds();
  
  // Initialize timer
  set_ccr( 0, TIMER_LIMIT );
  setup_timer_a(MODE_UP);
  
  // Send sync message
  register_timer_callback( send_sync_message, 0 );

  // Initialize radio and enable receive callback function
  setup_radio( process_rx );
  
  // Enable interrupts, otherwise nothing will work
  eint();
   
  while (1)
  {
    // Enter sleep mode
    __bis_SR_register( LPM3_bits + GIE );
    __no_operation();
        
  }
  
  return 0;
}

/*******************************************************************************
 * @fn     uint8_t send_sync_message()
 * @brief  TODO
 * ****************************************************************************/
uint8_t send_sync_message()
{
  // Send sync message
  radio_tx( tx_buffer, sizeof(packet_header_t) );
  led2_toggle();
  
  return 1;
}

/*******************************************************************************
 * @fn     uint8_t process_rx( uint8_t* buffer, uint8_t size )
 * @brief  callback function called when new message is received
 * ****************************************************************************/
uint8_t process_rx( uint8_t* buffer, uint8_t size )
{
  packet_header_t* header;
  static uint8_t counter = 0;
  header = (packet_header_t*)(buffer);
  header->flags = counter++;
  //packet_footer_t* footer;
  // Add one to account for the byte with the packet length
  //footer = (packet_footer_t*)(buffer + header->length + 1 );

  //uart_write( , 1 );
  uart_write_escaped( buffer, header->length + 1 );  
  
  // Erase buffer just for fun
  memset( buffer, 0x00, size );
  
  led3_toggle();
  return 1;
}

