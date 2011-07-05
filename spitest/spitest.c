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

extern cc2500_settings_t cc2500_settings;

int main( void )
{
  uint8_t string[] = "UART Test Program!\r\nEcho mode...\r\n";
  uint8_t status_byte;
  uint8_t tx_power = 0xFB;
  /* Init watchdog timer to off */
  WDTCTL = WDTPW|WDTHOLD;
  
  BCSCTL1 = CALBC1_12MHZ;
  DCOCTL = CALDCO_12MHZ; 
  
  __delay_cycles(0x40000);
  
  setup_leds();
  
  setup_uart();
  setup_spi();
  
  __bis_SR_register(GIE);		// enable general interrupts
  
  uart_write( string, sizeof(string) );
  
  initialize_radio();       // Reset radio
   
  write_rf_settings();                           // Initialize settings
  
  write_burst_register(PATABLE, &tx_power, 1 );  // Set TX power
  
  __delay_cycles(0x40000);
  
  strobe( SRX );
  
  for (;;) {
    
    //led1_toggle();
    // NOP Strobe
    //spi_put_char( SNOP );
    //hex_to_string( string, &status_byte, 1 ); uart_write( string, 2 );

    __delay_cycles(0x40000); __delay_cycles(0x40000); __delay_cycles(0x40000);
    
  }  /* while */
}





