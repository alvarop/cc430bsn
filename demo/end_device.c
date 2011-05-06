/** @file access_point.c
*
* @brief  TODO
*
* @author Alvaro Prieto
*       
*/
#include <signal.h>
#include <string.h>
#include "leds.h"
#include "oscillator.h"
#include "uart.h"
#include "timers.h"
#include "radio.h"
#include "settings.h"

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

uint8_t start_sample();
uint8_t process_rx( uint8_t*, uint8_t );
uint8_t send_samples();


uint8_t sample_buffer[ADC_MAX_SAMPLES * 2];
uint8_t buffer_index = 0;
uint8_t current_buffer = 0;

int main( void )
{
  
  packet_header_t* header;

  // Stop watchdog timer to prevent time out reset
  WDTCTL = WDTPW + WDTHOLD;
  
  header = (packet_header_t*)tx_buffer;
  
  // Initialize Tx Buffer
  header->length = sizeof(packet_header_t) + sizeof(packet_data_t) - 1;
  header->source = DEVICE_ADDRESS;
  header->type = 0xAA; // Samples
  header->flags = 0x00;
  
  // Make sure processor is running at 12MHz
  setup_oscillator();
  
  // Initialize UART for communications at 115200baud
  //setup_uart();
   
  // Initialize LEDs
  setup_leds();
  
  // Initialize timer
  set_ccr( 0, TIMER_LIMIT );
  setup_timer_a(MODE_UP);
  
  // Send sync message
  register_timer_callback( start_sample, 1 );
  set_ccr( 1, SAMPLE_RATE );
  
  register_timer_callback( send_samples, 2 );
  set_ccr( 2, (( REST_TIME/2) + MINOR_CYCLE * (DEVICE_ADDRESS - 1) ) );
    
  // Initialize radio and enable receive callback function
  setup_radio( process_rx );
  
  // Enable interrupts, otherwise nothing will work
  eint();
   
  while (1)
  {
    // Enter sleep mode
    __bis_SR_register( LPM3_bits + GIE );
    __no_operation();
    //led2_toggle();
  }
  
  return 0;
}

/*******************************************************************************
 * @fn     uint8_t start_sample()
 * @brief  TODO
 * ****************************************************************************/
uint8_t start_sample()
{ 
  //TODO Actually use timer functions later
  // Direct access to timer registers for faster development
  led1_toggle();
  
  TA0CCR1 += SAMPLE_RATE;
  if (TA0CCR1 > TIMER_LIMIT)
  {
    TA0CCR1 -= TIMER_LIMIT;
  }
  
  // This will be in ADC ISR, just testing for now
  sample_buffer[buffer_index] = buffer_index;        
  buffer_index++;
  
   if ( (ADC_MAX_SAMPLES) == buffer_index )
  {      
      current_buffer = 0;
      
  }
  else if( (2*ADC_MAX_SAMPLES) == buffer_index )
  {
      buffer_index = 0;
      current_buffer = 1;       
  }
  
  return 0;
}

/*******************************************************************************
 * @fn     uint8_t process_rx( uint8_t* buffer, uint8_t size )
 * @brief  callback function called when new message is received
 * ****************************************************************************/
uint8_t process_rx( uint8_t* buffer, uint8_t size )
{
  packet_header_t* header;
  header = (packet_header_t*)buffer;
  
  if( header->type == 0x66 )
  {
    // TODO: save current timer value here
    clear_timer();
    TA0CCR1 = SAMPLE_RATE;
    led1_off();
  }
  
  packet_footer_t* footer;
  // Add one to account for the byte with the packet length
  footer = (packet_footer_t*)(buffer + header->length + 1 );
    
  // Erase buffer just for fun
  memset( buffer, 0x00, size );
  
  led3_toggle();
  
  return 0;
}

/*******************************************************************************
 * @fn     uint8_t send_samples()
 * @brief  TODO
 * ****************************************************************************/
uint8_t send_samples()
{ 
  packet_data_t* data;
  
  led2_toggle();
  
  if( TA0CCR2 > MAJOR_CYCLE_LOOP )
  {
    TA0CCR2 = ( REST_TIME/2 ) + MINOR_CYCLE * (DEVICE_ADDRESS - 1);
  }
  else
  {
    TA0CCR2 += ( MAJOR_CYCLE );
  }
  
  
  data = (packet_data_t*)(tx_buffer + sizeof(packet_header_t));
  
  memcpy( data->samples, &sample_buffer[ current_buffer * ADC_MAX_SAMPLES ], 
  ( sizeof(sample_buffer) / 2 ) );
  
  radio_tx( tx_buffer, sizeof(packet_header_t) + sizeof(packet_data_t) );
  
  return 0;
}


