/** @file uart.c
*
* @brief UART functions
*
* @author Alvaro Prieto
*/
#include "uart.h"

/*******************************************************************************
 * @fn     void setup_uart( void )
 * @brief  configure uart for 115200BAUD on ports 1.6 and 1.7
 * ****************************************************************************/
void setup_uart( void )
{
  //Set up UART TX RX Pins for CC430
  PMAPPWD = 0x02D52;                        // Get write-access to port mapping regs 
  P1MAP5 = PM_UCA0RXD;                      // Map UCA0RXD output to P1.5
  P1MAP6 = PM_UCA0TXD;                      // Map UCA0TXD output to P1.6
  PMAPPWD = 0;                              // Lock port mapping registers
 
  P1DIR |= BIT6;                            // Set P2.7 as TX output
  P1SEL |= BIT5 + BIT6;                     // Select P2.6 & P2.7 to UART function 

  UCA0CTL1 |= UCSWRST;                      // **Put state machine in reset**
  UCA0CTL1 |= UCSSEL_2;                     // CLK = SMCLK
  UCA0BR0 = 104;                            // 12MHz/115200=104.167 (see User's Guide)
  UCA0BR1 = 0x00;                           //
  UCA0MCTL = UCBRS_1+UCBRF_0;               // Modulation UCBRSx=3, UCBRFx=0
  UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
  UCA0IE |= UCRXIE;                         // Enable USCI_A0 RX interrupt
}

/*******************************************************************************
 * @fn     setup_spi( )
 * @brief  Initialize SPI port and settings
 * ****************************************************************************/
void setup_spi()
{
  PMAPPWD = 0x02D52;                        // Get write-access to port mapping regs 
  P1MAP2 = PM_UCB0SOMI;                     // Map UCB0SOMI output to P1.2
  P1MAP3 = PM_UCB0SIMO;                     // Map UCB0SIMO output to P1.6
  P1MAP4 = PM_UCB0CLK;                      // Map UCB0CLK output to P1.6
  PMAPPWD = 0;                              // Lock port mapping registers
  
  P1OUT |= BIT7;  // CSn output
  P1DIR |= BIT7;  // Disable CSn

  UCB0CTL1 = UCSWRST;                   // Put state machine in reset state
  UCB0CTL0 = UCMST + UCCKPH + UCSYNC + UCMSB; // Master,3-pin,MSB first, sync
  UCB0CTL1 = UCSSEL_2;                  // Select SMCLK as source
  UCB0BR0 = 2;                         // 12MHz/2 = 6MHz clock
  UCB0BR1 = 0;  

  P1SEL |= BIT2 + BIT3 + BIT4; // Select UART/SPI function
  P1DIR |= BIT3 + BIT4; // Set P1.3, P1.4 as output
  
  UCB0CTL1 &= ~UCSWRST;                 // Start state machine
      
  //
  // Configure GDO0 input
  //
  P1IES |= BIT0;            // Edge select (high-to-low)
  P1IFG &= ~BIT0;           // Clear any pending interrupts
  P1IE |= BIT0;             // Enable interrupts

}

/*******************************************************************************
 * @fn     uart_put_char( uint8_t character )
 * @brief  transmit single character
 * ****************************************************************************/
void uart_put_char( uint8_t character )
{
  // Enable TX interrupt
  //UCA0IE |= UCRXIE;
  while (!(UCA0IFG&UCTXIFG));	// USCI_A0 TX buffer ready?
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
  
  P1OUT |= BIT7;
  
  // Enable TX interrupts on SPI
  UCB0IE |= UCTXIE;
  
  while (!(UCB0IFG & UCTXIFG ));	// USCI_A0 TX buffer ready?
  UCB0TXBUF = character;
}

/*******************************************************************************
 * @fn     void uart_isr( void )
 * @brief  UART ISR
 * ****************************************************************************/
wakeup interrupt ( USCI_A0_VECTOR ) uart_isr(void) // CHANGE
{
  //PJOUT ^= 0x2;

  switch(UCA0IV)
  {
    case 0:	// Vector 0 - no interrupt
    {
      break;
    }
    case 2:	// Vector 2 - RXIFG
    {

      //while (!(UCA0IFG&UCTXIFG));	// USCI_A0 TX buffer ready?
      //UCA0TXBUF = UCA0RXBUF;		// TX -> RXed character
      break;
    }
    case 4:	// Vector 4 - TXIFG
    {
      
      
      break;
    }

    default: break;
  }
}

/*******************************************************************************
 * @fn     void spi_isr( void )
 * @brief  SPI ISR
 * ****************************************************************************/
wakeup interrupt ( USCI_B0_VECTOR ) spi_isr(void) // CHANGE
{
  uint8_t rx_char;

  switch(UCB0IV)
  {
    case 0:	// Vector 0 - no interrupt
    {
      break;
    }
    case 2:	// Vector 2 - RXIFG
    {

      rx_char = UCB0RXBUF; 
      break;
    }
    case 4:	// Vector 4 - TXIFG
    {
      // Check if this is a burst transfer, otherwise, pull CSn high
      P1OUT &= ~BIT7;
      
      // Disable TX interrupts on SPI
      UCB0IE &= ~UCTXIE;
      
      break;
    }

    default: break;
  }
}
