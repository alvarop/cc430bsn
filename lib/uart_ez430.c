/** @file uart_ez430.c
*
* @brief UART functions for msp430x2274 (ez430-
*
* @author Alvaro Prieto
*/
#include "uart.h"
#include "leds.h"

/*******************************************************************************
 * @fn     void setup_uart( void )
 * @brief  configure uart for 115200BAUD on ports 1.6 and 1.7
 * ****************************************************************************/
void setup_uart( void )
{
  //Set up UART TX RX Pins for CC430
  //PMAPPWD = 0x02D52;                // Get write-access to port mapping regs 
  //P3MAP4 = PM_UCA0RXD;              // Map UCA0RXD output to P1.5
  //P3MAP5 = PM_UCA0TXD;              // Map UCA0TXD output to P1.6
  //PMAPPWD = 0;                      // Lock port mapping registers

  //
  // NOTE on port selection. p3.0 is the a slave enable output for SPI
  // but instead of using the built in functions to use it, it will be 
  // controlled as a regular I/O pin
  //  
  P3DIR |= BIT4; // Set P3.4 as TX output
  P3SEL |= BIT4 + BIT5; // Select UART/SPI function


  // 
  // Configure Serial Port
  // 

  UCA0CTL1 |= UCSWRST;                // **Put state machine in reset**
  UCA0CTL1 |= UCSSEL_2;               // CLK = SMCLK
  UCA0BR0 = 104;                      // 12MHz/115200=104.167 (see User's Guide)
  UCA0BR1 = 0x00;                     //
  UCA0MCTL = UCBRS_1+UCBRF_0;         // Modulation UCBRSx=3, UCBRFx=0
  UCA0CTL1 &= ~UCSWRST;               // **Initialize USCI state machine**

  

  // Enable Interrupts
  IE2 |= UCA0RXIE;          // Enable USCI_A0/B0 RX interrupt

}

void setup_spi()
{  
  P3OUT |= BIT0;  // CSn output
  P3DIR |= BIT0;  // Disable CSn  

  UCB0CTL1 = UCSWRST;                   // Put state machine in reset state
  UCB0CTL0 = UCMST + UCCKPH + UCSYNC + UCMSB; // Master,3-pin,MSB first, sync
  UCB0CTL1 = UCSSEL_2;                  // Select SMCLK as source
  UCB0BR0 = 2;                         // 12MHz/2 = 6MHz clock
  UCB0BR1 = 0;  

  P3SEL |= BIT3 + BIT2 + BIT1; // Select UART/SPI function
  P3DIR |= BIT3 + BIT1; // Set P3.3, P3.1 as output
  
  UCB0CTL1 &= ~UCSWRST;                 // Start state machine

}

/*******************************************************************************
 * @fn     uart_put_char( uint8_t character )
 * @brief  transmit single character
 * ****************************************************************************/
void uart_put_char( uint8_t character )
{
  // Enable TX interrupt
  //UCA0IE |= UCRXIE;
  
  while (!(IFG2&UCA0TXIFG));	// USCI_A0 TX buffer ready?
  UCA0TXBUF = character;
}

/*******************************************************************************
 * @fn     uart_write( uint8_t character )
 * @brief  transmit whole buffer
 * ****************************************************************************/
void uart_write( uint8_t* buffer, uint16_t length )
{
  uint16_t buffer_index;
  
  for( buffer_index = 0; buffer_index < length; buffer_index++ )
  {
    uart_put_char( buffer[buffer_index] );
  }
}

/*******************************************************************************
 * @fn     uart_write_escaped( uint8_t character )
 * @brief  transmit whole buffer while escaping characters
 * ****************************************************************************/
void uart_write_escaped( uint8_t* buffer, uint16_t length )
{
  uint16_t buffer_index;
    
  uart_put_char( 0x7e );
  for( buffer_index = 0; buffer_index < length; buffer_index++ )
  {
    if( (buffer[buffer_index] == 0x7e) | (buffer[buffer_index] == 0x7d) )
    {
      uart_put_char( 0x7d ); // Escape byte
      uart_put_char( buffer[buffer_index] ^ 0x20 );
    }
    else
    {
      uart_put_char( buffer[buffer_index] );
    }
  }
  uart_put_char( 0x7e );
}

/*******************************************************************************
 * @fn     spi_put_char( uint8_t character )
 * @brief  transmit single character through spi
 * ****************************************************************************/
void spi_put_char( uint8_t character )
{

  // Enable CSn
  
  P3OUT |= BIT0;
  
  // Enable TX interrupts on SPI
  IE2 |= UCB0TXIE;
  
  while (!(IFG2&UCB0TXIFG));	// USCI_A0 TX buffer ready?
  UCB0TXBUF = character;
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
 * @fn     void uart_rx_isr( void )
 * @brief  UART ISR
 * ****************************************************************************/
wakeup interrupt ( USCIAB0RX_VECTOR ) uart_rx_isr(void) // CHANGE
{
  uint8_t rx_char;
  
  // Process incoming byte from USART
  if( IFG2 & UCA0RXIFG )
  { 
      // Send character to SPI   
      spi_put_char( UCA0RXBUF );
  }
  // Process incoming byte from SPI
  else if ( IFG2 & UCB0RXIFG )
  {
      //led2_on();
      // Send character to Serial
      rx_char = UCB0RXBUF;     
  }
}

/*******************************************************************************
 * @fn     void uart_tx_isr( void )
 * @brief  UART ISR
 * ****************************************************************************/
wakeup interrupt ( USCIAB0TX_VECTOR ) uart_tx_isr(void) // CHANGE
{
  // Done transmitting character through SPI
  if ( IFG2 & UCB0TXIFG )
  {
      // Check if this is a burst transfer, otherwise, pull CSn high
      P3OUT &= ~BIT0;
      
      // Disable TX interrupts on SPI
      IE2 &= ~UCB0TXIE;
      
      
  }
}

