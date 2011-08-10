/** @file spitest.c
*
* @brief Set up a bridge between serial port and SPI port to test radio
*
* @author Alvaro Prieto
*/
#include "common.h"

#include "intrinsics.h"
#include "leds.h"
#include "uart.h"
#include "cc2500.h"
#include "radio_cc2500.h"

#include <string.h>

uint8_t string[100];
uint8_t p_tx_buffer[10];
uint8_t registers[20];
uint8_t message[64];
uint8_t reg;
extern cc2500_settings_t cc2500_settings;

volatile uint8_t send_reply;

void send_packet( uint8_t*, uint8_t );
void debug_status();
void debug_fifo();

/*******************************************************************************
 * @fn     void rx_callback( void )
 * @brief  rx callback function, called when packet has been received
 * ****************************************************************************/
static uint8_t rx_callback( uint8_t* p_buffer, uint8_t size )
{
  led2_toggle();
  
  // DEBUG, print received message in HEX
  debug("rx:  ");
  hex_to_string( string, p_buffer, size);
  debug("0x");
  debug( string );
  debug("\r\n");
  
  // Message received flag
  send_reply = 1;

  return 0;
}

int main( void )
{

  uint8_t counter;    
  
  /* Init watchdog timer to off */
  WDTCTL = WDTPW|WDTHOLD;
  
  // Initialize test message with ones
  for ( counter = 0; counter < 10; counter++ )
  {
    message[counter] = 1;
  }
  
  send_reply = 0;
    
  // Setup oscillator for 12MHz operation
  BCSCTL1 = CALBC1_12MHZ;
  DCOCTL = CALDCO_12MHZ; 
  
  // Wait for changes to take effect
  __delay_cycles(0x40000);
      
  setup_uart();
  
  debug("Starting...\r\n");
  
  setup_spi();
  
  setup_cc2500( rx_callback );
     
  //
  // Configure pushbutton input
  //  
  P1REN = BIT2;             // Pull-up resistor enable
  P1OUT = BIT2;             // Pull-up resistor (instead of pull-down)
  P1IES = BIT2;             // Edge select (high-to-low)
  P1IFG &= ~BIT2;           // Clear any pending interrupts
  P1IE = BIT2;              // Enable interrupts
  
  setup_leds();
  debug_status();            
  for (;;) 
  {        
    __bis_SR_register( /*LPM0_bits +*/ GIE );   // Sleep

    if ( send_reply )
    {
      send_reply = 0;
      
      // Don't reply while debugging
      //p_tx_buffer[0] = 0xac;
      //cc2500_tx_packet(p_tx_buffer,1, 0);
      led1_toggle();
    }
  } 
  
  return 0;
}

/*******************************************************************************
 * @fn     void port1_isr( void )
 * @brief  Pushbutton Interrupt (PORT 1)
 * ****************************************************************************/
wakeup interrupt ( PORT1_VECTOR ) port1_isr(void) // CHANGE
{
  //
  // If the button is pressed, toggle the LED and transmit message
  //
  if ( P1IFG & BIT2 )
  {        
      led1_toggle();         
      cc2500_tx_packet( message, 10, 0 );       
      
      __delay_cycles(0x40000);     // Debounce
                 
  }
  
  P1IFG &= ~BIT2;           // Clear flag

}

//
// Temporary debug function that displays current radio status
//
void debug_status()
{
    uint8_t radio_status = strobe(SNOP);
    uint8_t debug_string[3];
    hex_to_string( debug_string, &radio_status, 1);
    debug( debug_string );
    
    switch ( ((radio_status & 0x70) >> 4) )
    {
      case 0x00:
        debug(" IDLE\r\n");
        break;
      case 0x01:
        debug(" RX\r\n");
        break;
      case 0x02:
        debug(" TX\r\n");
        break;
      case 0x03:
        debug(" FSTXON\r\n");
        break;
      case 0x04:
        debug(" CALIBRATE\r\n");
        break;
      case 0x05:
        debug(" SETTLING\r\n");
        break;
      case 0x06:
        debug(" RXFIFO_OVERFLOW\r\n");
        break;
      case 0x07:
        debug(" TXFIFO_UNDERFLOW\r\n");
        break;
      default:
        break;
    }

}

//
// Temporary function to display RX and TX fifo buffer statuses
//
void debug_fifo()
{
  uint8_t radio_status;
  
  debug( "rx: " );
  radio_status = read_status( RXBYTES ) ;
  hex_to_string( string, &radio_status, 1);
  debug( string );
  debug(" - ");

  debug( "tx: " );
  radio_status = read_status( TXBYTES ) ;
  hex_to_string( string, &radio_status, 1);
  debug( string );
  debug("\r\n");
}

