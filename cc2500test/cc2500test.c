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

#ifndef DEVICE_ADDRESS
#define DEVICE_ADDRESS 0xFF
#endif

uint8_t string[100];
uint8_t p_tx_buffer[10];
uint8_t registers[20];
uint8_t message[64];
uint8_t reg;
extern cc2500_settings_t cc2500_settings;

volatile uint8_t send_reply;

/*******************************************************************************
 * @fn     void rx_callback( void )
 * @brief  rx callback function, called when packet has been received
 * ****************************************************************************/
static uint8_t rx_callback( uint8_t* p_buffer, uint8_t size )
{
  led2_toggle();
    
  // DEBUG, print received message in HEX
  //debug("rx:  ");
  //hex_to_string( string, p_buffer, size+2);
  //debug("0x");
  //debug( string );
  //debug("\r\n");
  
  hex_to_string( string, &p_buffer[size], 1);
  debug("RSSI: 0x");
  debug( string );
  debug("\r\n");
  
  // DEBUG, print received message
  p_buffer[size] = 0;
  debug(p_buffer);
  debug("\r\n");
  
  // Message received flag
  send_reply = 1;

  return 0;
}

int main( void )
{   
  
  /* Init watchdog timer to off */
  WDTCTL = WDTPW|WDTHOLD;
   
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
  
  cc2500_set_address( DEVICE_ADDRESS );
     
  //
  // Configure pushbutton input
  //  
  P1REN = BIT2;             // Pull-up resistor enable
  P1OUT = BIT2;             // Pull-up resistor (instead of pull-down)
  P1IES = BIT2;             // Edge select (high-to-low)
  P1IFG &= ~BIT2;           // Clear any pending interrupts
  P1IE = BIT2;              // Enable interrupts
  
  setup_leds();
           
  for (;;) 
  {        
    __bis_SR_register( LPM3_bits + GIE );   // Sleep

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
      cc2500_tx_packet( "Button", 6, 5 );       
      
      __delay_cycles(0x40000);     // Debounce
                 
  }
  
  P1IFG &= ~BIT2;           // Clear flag

}

