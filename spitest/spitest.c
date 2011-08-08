/** @file spitest.c
*
* @brief Set up a bridge between serial port and SPI port to test radio
*
* @author Alvaro Prieto
*/
#include <string.h>
#include "common.h"

#include "intrinsics.h"
#include "leds.h"
#include "uart.h"
#include "oscillator.h"
#include "cc2500.h"
#include "radio.h"

uint8_t string[50];
uint8_t p_rx_buffer[10];
uint8_t registers[20];
uint8_t message[15];
uint8_t reg;
uint8_t radio_status;
extern cc2500_settings_t cc2500_settings;

void send_packet( uint8_t*, uint8_t );
uint8_t process_rx( uint8_t*, uint8_t );
void radio_debug( uint8_t* buffer );

void debug_status()
{
    radio_status = strobe(SNOP);
    hex_to_string( message, &radio_status, 1);
    radio_debug( message );
    
    switch ( ((radio_status & 0x70) >> 4) )
    {
      case 0x00:
        radio_debug(" IDLE\r\n");
        break;
      case 0x01:
        radio_debug(" RX\r\n");
        break;
      case 0x02:
        radio_debug(" TX\r\n");
        break;
      case 0x03:
        radio_debug(" FSTXON\r\n");
        break;
      case 0x04:
        radio_debug(" CALIBRATE\r\n");
        break;
      case 0x05:
        radio_debug(" SETTLING\r\n");
        break;
      case 0x06:
        radio_debug(" RXFIFO_OVERFLOW\r\n");
        break;
      case 0x07:
        radio_debug(" TXFIFO_UNDERFLOW\r\n");
        break;
      default:
        break;
    }

}

int main( void )
{
  
  uint8_t tx_power = 0xFB;   
  
  message[0]=5;
  message[1]='a';
  message[2]='a';
  message[3]='a';
  message[4]='a';
  message[5]='a';
  
  /* Init watchdog timer to off */
  WDTCTL = WDTPW|WDTHOLD;
  
  // Setup oscillator for 12MHz operation
  //BCSCTL1 = CALBC1_12MHZ;
  //DCOCTL = CALDCO_12MHZ; 
  
  setup_oscillator();
  
  // Wait for changes to take effect
  __delay_cycles(0x40000);

  // Initialize radio and enable receive callback function
  setup_radio( process_rx );
  
  radio_debug( "\r\n\n"); 
  radio_debug( "Setup UART\r\n");  
  setup_uart();
  
  radio_debug( "Setup SPI\r\n");
  setup_spi();
 
  P1SEL &= ~BIT0;
  
  radio_debug( "Initialize cc2500\r\n");
  initialize_radio();       // Reset radio
  
  __delay_cycles(100);      // Let radio settle (Won't configure unless you do?)
  
  //radio_debug( "Write RF Settings");
  
 
  write_rf_settings();      // Initialize settings                            
  write_burst_register(PATABLE, &tx_power, 1 ); // Set TX power
 
  //strobe( SRX );            // Set radio to RX mode
      
  //
  // Configure pushbutton input
  //  
 /* P1REN = BIT2;             // Pull-up resistor enable
  P1OUT = BIT2;             // Pull-up resistor (instead of pull-down)
  P1IES = BIT2;             // Edge select (high-to-low)
  P1IFG &= ~BIT2;           // Clear any pending interrupts
  P1IE = BIT2;              // Enable interrupts*/
  
  setup_leds();
  
  strobe(SRX);



  radio_debug( "Main Loop\r\n" );
             
  for (;;) 
  {        
    __bis_SR_register( /*LPM3_bits +*/ GIE );   // 
    
    
    debug_status();
    
    
    radio_debug( "RX_BYTES: " );
    radio_status = read_status( RXBYTES ) ;
    hex_to_string( message, &radio_status, 1);
    radio_debug( message );
    radio_debug("\r\n");
    
    radio_debug( "TX_BYTES: " );
    radio_status = read_status( TXBYTES ) ;
    hex_to_string( message, &radio_status, 1);
    radio_debug( message );
    radio_debug("\r\n");
    
    //led2_toggle();
      
    __delay_cycles(40000);     // 
    __delay_cycles(40000);     // 
    __delay_cycles(40000);     // 
    __delay_cycles(40000);     // 
    __delay_cycles(40000);     // 
    __delay_cycles(40000);     // 
    __delay_cycles(40000);     // 
    __delay_cycles(40000);     // 
    __delay_cycles(40000);     // 
    __delay_cycles(40000);     // 
    
    radio_debug( "Sending packet...\r\n" );
    send_packet( message, 6 );
    radio_debug( "Packet sent\r\n" );
    
    radio_debug( "RX_BYTES: " );
    radio_status = read_status( RXBYTES ) ;
    hex_to_string( message, &radio_status, 1);
    radio_debug( message );
    radio_debug("\r\n");
    
    radio_debug( "TX_BYTES: " );
    radio_status = read_status( TXBYTES ) ;
    hex_to_string( message, &radio_status, 1);
    radio_debug( message );
    radio_debug("\r\n");
    
  } 
  
  return 0;
}

void radio_debug( uint8_t* buffer )
{
  uint16_t buffer_size;
  uint8_t radio_tx_buffer[PACKET_LEN];
  
  // Wait a bit for radio to be ready
  __delay_cycles(0x4000);
  
  buffer_size = strlen(buffer);
  if ( buffer_size > PACKET_LEN - 1 )
  {
    buffer_size = PACKET_LEN - 1;
  }
  
  radio_tx_buffer[0] = (uint8_t) buffer_size;
  memcpy( &radio_tx_buffer[1], buffer, buffer_size );
  
  radio_tx( radio_tx_buffer, buffer_size + 1 );
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
  
  // terminate string
  buffer_out[counter++] = 0;
  return counter;
}



/*******************************************************************************
 * @fn     void send_packet( uint8_t* p_buffer, uint8_t size )
 * @brief  Send packet over the radio using CC2500
 * ****************************************************************************/
void send_packet( uint8_t* p_buffer, uint8_t size )
{
  led1_toggle(); 
  write_burst_register( FIFO, p_buffer, size );
  
  //radio_debug("Wrote data to buffer\r\n");
  //debug_status();

  radio_debug( "TX_BYTES: " );
    radio_status = read_status( TXBYTES ) ;
    hex_to_string( message, &radio_status, 1);
    radio_debug( message );
    radio_debug("\r\n");
    
  strobe( STX );              // Change to TX mode, start data transfer
  
  //radio_debug("Strobed TX\r\n");
  //debug_status();
  
  while ( !( P1IN & BIT0 ) ); // Wait until GDO0 goes high (sync word txed)
  
  // Transmitting

  while ( P1IN & BIT0 );      // Wait until GDO0 goes low (end of packet)
  
  radio_debug( "TX_BYTES: " );
    radio_status = read_status( TXBYTES ) ;
    hex_to_string( message, &radio_status, 1);
    radio_debug( message );
    radio_debug("\r\n");
  
  strobe( SRX );
  
  P1IFG &= ~BIT0;             // Clear flag
  
}


/*******************************************************************************
 * @fn     uint8_t process_rx( uint8_t* buffer, uint8_t size )
 * @brief  callback function called when new message is received
 * ****************************************************************************/
uint8_t process_rx( uint8_t* buffer, uint8_t size )
{
   
  // Erase buffer just for fun
  memset( buffer, 0x00, size );

  return 1;
}


/*******************************************************************************
 * @fn     uint8_t receive_packet( uint8_t* p_buffer, uint8_t* size )
 * @brief  Receive packet from the radio using CC2500
 * ****************************************************************************/
uint8_t receive_packet( uint8_t* p_buffer, uint8_t* size )
{
  uint8_t status[2];
  uint8_t packet_length;
  
  // Make sure there are bytes to be read in the FIFO buffer
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
  uint8_t length = 10;
  led2_on();
  
  // Check
  if ( P1IFG & BIT0 )
  {
      if( receive_packet( p_rx_buffer, &length ) )
      {
        // Display received packet
        //uart_write("Rx: ", 4);
        //uart_write( p_rx_buffer, length );
        //uart_write("\r\n", 2);
        
        radio_debug( "message received");
        
      }
           
  }
  P1IFG &= ~BIT0;           // Clear flag 

  strobe( SRX ); // enter receive mode again

  led2_off();
}



