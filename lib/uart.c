/** @file uart.c
*
* @brief UART functions
*
* @author Alvaro Prieto
*/
#include "uart.h"

static uint8_t tx_buffer;

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
 * @brief  transmit single character
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

      while (!(UCA0IFG&UCTXIFG));	// USCI_A0 TX buffer ready?
      UCA0TXBUF = UCA0RXBUF;		// TX -> RXed character
      break;
    }
    case 4:	// Vector 4 - TXIFG
    {
      
      
      break;
    }

    default: break;
  }
}
