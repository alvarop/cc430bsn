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

uint8_t string[] = "UART Test Program!\r\nEcho mode...\r\n";
uint8_t p_rx_buffer[10];
extern cc2500_settings_t cc2500_settings;

int main( void )
{
  
  uint8_t tx_power = 0xFB;
  
  uint8_t p_buffer[10];
  uint8_t reg;
  
  /* Init watchdog timer to off */
  WDTCTL = WDTPW|WDTHOLD;
  
  // Setup oscillator for 12MHz operation
  BCSCTL1 = CALBC1_12MHZ;
  DCOCTL = CALDCO_12MHZ; 
  
  // Wait for changes to take effect
  __delay_cycles(0x40000);
  
  setup_leds();
  
  setup_uart();
  setup_spi();
  
  //  
  // Configure I/O ports
  //
  
  //
  // Configure pushbutton input
  //  
  P1REN = BIT2;             // Pull-up resistor enable
  P1OUT = BIT2;             // Pull-up resistor (instead of pull-down)
  P1IES = BIT2;             // Edge select (high-to-low)
  P1IFG &= ~BIT2;           // Clear any pending interrupts
  P1IE = BIT2;              // Enable interrupts
  
  //
  // Configure GDO0 input
  //
  P2SEL = 0;
  P2IES |= BIT6;            // Edge select (high-to-low)
  P2IFG &= ~BIT6;           // Clear any pending interrupts
  P2IE |= BIT6;             // Enable interrupts
  
  
  
  
  //uart_write( string, sizeof(string) );
  
  initialize_radio();       // Reset radio
   __delay_cycles(0x40000);
  write_rf_settings();                           // Initialize settings
  
  write_burst_register(PATABLE, &tx_power, 1 );  // Set TX power
  
  strobe( SRX );
    
  
  __bis_SR_register(GIE);		// enable general interrupts
  
  for (;;) {

    __delay_cycles(0x40000); __delay_cycles(0x40000); __delay_cycles(0x40000);
    //__bis_SR_register(LPM3_bits + GIE);
  }  /* while */
}

void send_packet( uint8_t* p_buffer, uint8_t size )
{
  led1_on(); 
  write_burst_register( FIFO, p_buffer, size );

  strobe( STX );
  
  while ( !( P2IN & BIT6 ) ); // Wait until GDO0 goes high

  // Transmitting
  
  while ( P2IN & BIT6 );
  
  P2IFG &= ~BIT6;           // Clear flag
  led1_off();
}

uint8_t receive_packet( uint8_t* p_buffer, uint8_t* size )
{
  uint8_t status[2];
  uint8_t packet_length;
  
  if ( (read_status( RXBYTES ) & NUM_RXBYTES ) )
  {
    read_burst_register( FIFO, &packet_length, 1 );
    
    if ( packet_length <= *size )
    {
      read_burst_register( FIFO, p_buffer, packet_length );
      *size = packet_length;
      read_burst_register( FIFO, status, 2 );
      
      return ( status[LQI_POS] & CRC_OK );
    }
    else
    {
      *size = packet_length;
      strobe( SFRX );
      
      return 0;
    }
    
  }
  
  return 0;
  
}

/*******************************************************************************
 * @fn     void port1_isr( void )
 * @brief  UART ISR
 * ****************************************************************************/
wakeup interrupt ( PORT1_VECTOR ) port1_isr(void) // CHANGE
{

  if ( P1IFG & BIT2 )
  {
      //led1_toggle();
      P1IFG &= ~BIT2;           // Clear flag
      
      send_packet( string, 5 );
      
      __delay_cycles(8000);     // Debounce
      
  }
}

/*******************************************************************************
 * @fn     void port2_isr( void )
 * @brief  UART ISR
 * ****************************************************************************/
wakeup interrupt ( PORT2_VECTOR ) port2_isr(void) // CHANGE
{
  uint8_t length = 10;
  if ( P2IFG & BIT6 )
  {
      if( receive_packet( p_rx_buffer, &length ) )
      {
        led2_toggle();
      }
      P2IFG &= ~BIT6;           // Clear flag
      //uart_write("hmmm\r\n", 6 );
  }
}



