/** @file radiotest.c
*
* @brief  Simple test of radio functions.
*         A timer interrupt sends a message every ~2 seconds and prints out any
*         received message information.
*
* @author Alvaro Prieto
*       derived from work by M. Morales/D. Dang of Texas Instruments
*/
#include "common.h"
#include <signal.h>
#include <string.h>
#include "leds.h"
#include "oscillator.h"
#include "uart.h"
#include "timers.h"
#include "radio.h"

uint8_t tx_buffer[PACKET_LEN+1];
uint8_t buttonPressed = 0;

uint8_t print_buffer[200];

#define TOTAL_SAMPLES 50

typedef struct
{
  uint8_t length;
  uint8_t source;
  uint8_t type;
  uint8_t flags;
} packet_header_t;

typedef struct
{
  uint8_t samples[TOTAL_SAMPLES];
} packet_data_t;

typedef struct
{
  uint8_t rssi;
  uint8_t lqi_crcok;
} packet_footer_t;

uint8_t hex_to_string( uint8_t*, uint8_t*, uint8_t );
uint8_t fake_button_press();
uint8_t process_rx( uint8_t*, uint8_t );

int main( void )
{
  uint8_t buffer_index;
  packet_header_t* header;
  packet_data_t* data;

  // Stop watchdog timer to prevent time out reset
  WDTCTL = WDTPW + WDTHOLD;
  
  header = (packet_header_t*)tx_buffer;
  data = (packet_data_t*)(tx_buffer + sizeof(packet_header_t));
  
  // Initialize Tx Buffer
  header->length = PACKET_LEN;
  header->source = DEVICE_ADDRESS;
  header->type = 0xAA;
  header->flags = 0x55;
  
  // Fill in dummy values for the buffer
  for( buffer_index=0; buffer_index < TOTAL_SAMPLES; buffer_index++ )
  {
    data->samples[buffer_index] = buffer_index;
  }  
  
  // Make sure processor is running at 12MHz
  setup_oscillator();
  
  // Initialize UART for communications at 115200baud
  setup_uart();
   
  // Initialize LEDs
  setup_leds();
  
  // Initialize timer
  setup_timer_a();
  
  // Send fake button press every ~2 seconds
  register_timer_callback( fake_button_press, TOTAL_CCRS );

  // Initialize radio and enable receive callback function
  setup_radio( process_rx );
  
  // Enable interrupts, otherwise nothing will work
  eint();
   
  while (1)
  {
    // Enter sleep mode
    __bis_SR_register( LPM3_bits + GIE );
    __no_operation();
    
    if (buttonPressed) // Process a button press->transmit
    {
      led1_toggle(); // Pulse LED during Transmit
      buttonPressed = 0;
      
      radio_tx( tx_buffer, sizeof(tx_buffer) );

      uart_write( "\r\nTx\r\n", 6 );

    }    
  }
  
  return 0;
}

/*******************************************************************************
 * @fn     uint8_t hex_to_string( uint8_t* buffer_out, uint8_t* buffer_in, 
 *                                          uint8_t buffer_in_size  )
 * @brief  Used to convert hex values to [hex]string format
 * ****************************************************************************/
uint8_t hex_to_string( uint8_t* buffer_out, uint8_t* buffer_in, 
                                    uint8_t buffer_in_size  )
{
  static const uint8_t hex_char[16] = { '0', '1', '2', '3', '4', '5', '6', '7', 
                                      '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
  uint8_t counter = 0;
  
  while(counter < buffer_in_size*2 )
  {
    buffer_out[counter] = hex_char[((buffer_in[(counter>>1)]>>4) & 0xF)];
    counter++;
    buffer_out[counter] = hex_char[(buffer_in[(counter>>1)] & 0xF)];
    counter++;
  }
  
  return counter;
}

/*******************************************************************************
 * @fn     uint8_t fake_button_press()
 * @brief  Instead of using buttons, this function is called from a timer isr
 * ****************************************************************************/
uint8_t fake_button_press()
{
  buttonPressed = 1;
  
  return 1;
}

/*******************************************************************************
 * @fn     uint8_t process_rx( uint8_t* buffer, uint8_t size )
 * @brief  callback function called when new message is received
 * ****************************************************************************/
uint8_t process_rx( uint8_t* buffer, uint8_t size )
{
  packet_header_t* header;
  header = (packet_header_t*)buffer;
  
  packet_footer_t* footer;
  // Add one to account for the byte with the packet length
  footer = (packet_footer_t*)(buffer + header->length + 1 );
  
  // Print incoming packet information for debugging
  uart_write( "Size: ", 6 );
  hex_to_string( print_buffer, &header->length, 1 );
  uart_write( print_buffer, 2 );
  uart_write( "\r\n", 2 );
  
  uart_write( "From: ", 6 );
  hex_to_string( print_buffer, &header->source, 1 );
  uart_write( print_buffer, 2 );
  uart_write( "\r\n", 2 );
  
  uart_write( "Type: ", 6 );
  hex_to_string( print_buffer, &header->type, 1 );
  uart_write( print_buffer, 2 );
  uart_write( "\r\n", 2 );
  
  uart_write( "Flags: ", 7 );
  hex_to_string( print_buffer, &header->flags, 1 );
  uart_write( print_buffer, 2 );
  uart_write( "\r\n", 2 );
  
  uart_write( "Rssi: ", 6 );
  hex_to_string( print_buffer, &footer->rssi, 1 );
  uart_write( print_buffer, 2 );
  uart_write( "\r\n", 2 );
 
  // Print whole packet in hex
  //hex_to_string( print_buffer, buffer, size );  
  //uart_write( print_buffer, (size)*2 );
  //uart_write( "\r\n", 2 );
  
  // Erase buffer just for fun
  memset( buffer, 0x00, size );
  
  led3_toggle();
  return 1;
}

