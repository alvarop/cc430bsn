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

uint8_t string[10];
uint8_t p_rx_buffer[10];
uint8_t reg;
extern cc2500_settings_t cc2500_settings;

int main( void )
{
  
  uint8_t tx_power = 0xFB;
  
  uint8_t p_buffer[10];
  
  
  /* Init watchdog timer to off */
  WDTCTL = WDTPW|WDTHOLD;
  
  // Setup oscillator for 12MHz operation
  BCSCTL1 = CALBC1_12MHZ;
  DCOCTL = CALDCO_12MHZ; 
  
  // Wait for changes to take effect
  __delay_cycles(0x40000);
      
  setup_uart();
  
  uart_write( "Starting...\r\n\n", 14 );
  
  setup_spi();
  
  P2SEL = 0;
  initialize_radio();       // Reset radio

  reg = strobe(SNOP); hex_to_string( string, &reg, 1);
  uart_write( string, 2);uart_write("|1\r\n",4);
  
  __delay_cycles(100);      // Let radio settle (Won't configure unless you do?)
  write_rf_settings();      // Initialize settings                            
  write_burst_register(PATABLE, &tx_power, 1 ); // Set TX power
  
  reg = strobe(SNOP); hex_to_string( string, &reg, 1);
  uart_write( string, 2);uart_write("|2\r\n",4);
  
  strobe( SRX );            // Set radio to RX mode
  
  reg = strobe(SNOP); hex_to_string( string, &reg, 1);
  uart_write( string, 2);uart_write("|3\r\n",4);
    
  //
  // Configure pushbutton input
  //  
  P1REN = BIT2;             // Pull-up resistor enable
  P1OUT = BIT2;             // Pull-up resistor (instead of pull-down)
  P1IES = BIT2;             // Edge select (high-to-low)
  P1IFG &= ~BIT2;           // Clear any pending interrupts
  P1IE = BIT2;              // Enable interrupts
  
  setup_leds();
  
  //
  // Configure GDO0 input
  //

  P2IES |= BIT6;            // Edge select (high-to-low)
  P2IFG &= ~BIT6;           // Clear any pending interrupts
  P2IE |= BIT6;             // Enable interrupts
         
        
 
  for (;;) 
  {        
    __bis_SR_register(/*LPM3_bits +*/ GIE);
    reg = strobe(SNOP); hex_to_string( string, &reg, 1);
    uart_write( string, 2);uart_write("|m\r\n",4);
    if ( !( reg & 0x70 ) )
    {
      uart_write("..",2);
      strobe(SRX);
    }
    __delay_cycles(50000);
    __delay_cycles(50000);
    __delay_cycles(50000);
    __delay_cycles(50000);
    __delay_cycles(50000);
  } 
  
  return 0;
}

void send_packet( uint8_t* p_buffer, uint8_t size )
{
  
  reg = strobe(SNOP); hex_to_string( string, &reg, 1);
  uart_write( string, 2);uart_write("|s\r\n",4);
   
  write_burst_register( FIFO, p_buffer, size );
  
  reg = strobe(SNOP); hex_to_string( string, &reg, 1);
  uart_write( string, 2);uart_write("|b\r\n",4);
  
  strobe( STX );              // Change to TX mode, start data transfer

  while ( !( P2IN & BIT6 ) ); // Wait until GDO0 goes high (sync word txed)
  
  // Transmitting

  while ( P2IN & BIT6 );      // Wait until GDO0 goes low (end of packet)

  
  reg = strobe(SNOP); hex_to_string( string, &reg, 1);
  uart_write( string, 2);uart_write("|t\r\n",4);
  
  strobe( SRX );
  
  P2IFG &= ~BIT6;             // Clear flag
  
}

uint8_t receive_packet( uint8_t* p_buffer, uint8_t* size )
{
  uint8_t status[2];
  uint8_t packet_length;
  
  
  
  reg = strobe(SNOP); hex_to_string( string, &reg, 1);
  uart_write( string, 2);uart_write("|R\r\n",4);
  
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
  
  reg = strobe(SNOP); hex_to_string( string, &reg, 1);
  uart_write( string, 2);uart_write("|r\r\n",4);
  
  strobe( SRX ); // enter receive mode again
  
  return 0;
  
}

/*******************************************************************************
 * @fn     void port1_isr( void )
 * @brief  UART ISR
 * ****************************************************************************/
wakeup interrupt ( PORT1_VECTOR ) port1_isr(void) // CHANGE
{
  led1_on();
  if ( P1IFG & BIT2 )
  {                 
      send_packet( "12345", 5 );
      
      __delay_cycles(40000);     // Debounce
                 
  }
  
  P1IFG &= ~BIT2;           // Clear flag
  led1_off();
}

/*******************************************************************************
 * @fn     void port2_isr( void )
 * @brief  UART ISR
 * ****************************************************************************/
wakeup interrupt ( PORT2_VECTOR ) port2_isr(void) // CHANGE
{
  uint8_t length = 10;
  led2_on();
  
  if ( P2IFG & BIT6 )
  {
      if( receive_packet( p_rx_buffer, &length ) )
      {
        uart_write("Rx: ", 4);
        uart_write( p_rx_buffer, length );
        uart_write("\r\n", 2);
      }
           
  }
  P2IFG &= ~BIT6;           // Clear flag 
  
  led2_off();
}



