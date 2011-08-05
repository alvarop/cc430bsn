/** @file radio_cc2500.c
*
* @brief CC2500 radio functions
*
* @author Alvaro Prieto
*/
#include "radio_cc2500.h"
#include "cc2500.h"
#include "uart.h"
#include "intrinsics.h"
#include <signal.h>

static uint8_t dummy_callback( uint8_t*, uint8_t );
inline void tx_done( void );
inline void rx_enable();
inline void rx_disable();
uint8_t receive_packet( uint8_t*, uint8_t* );

// Receive buffer
static uint8_t p_rx_buffer[CC2500_BUFFER_SIZE];

extern cc2500_settings_t cc2500_settings;

// Holds pointers to all callback functions for CCR registers (and overflow)
static uint8_t (*rx_callback)( uint8_t*, uint8_t ) = dummy_callback;

/*******************************************************************************
 * @fn     void setup_radio( uint8_t (*callback)(void) )
 * @brief  Initialize radio and register Rx Callback function
 * ****************************************************************************/
void setup_cc2500( uint8_t (*callback)(uint8_t*, uint8_t) )
{
  uint8_t tx_power = 0xFB; // Maximum power

  // Set-up rx_callback function
  rx_callback = callback;
  
 initialize_radio();       // Reset radio
  
  __delay_cycles(100);      // Let radio settle (Won't configure unless you do?)

  write_rf_settings();      // Initialize settings                            
  write_burst_register(PATABLE, &tx_power, 1 ); // Set TX power
 
  strobe( SRX );            // Set radio to RX mode

}

/*******************************************************************************
 * @fn     void radio_tx( uint8_t* buffer, uint8_t size )
 * @brief  Send message through radio
 * ****************************************************************************/
void cc2500_tx( uint8_t* p_buffer, uint8_t size )
{
  write_burst_register( FIFO, p_buffer, size );
  
  strobe( STX );              // Change to TX mode, start data transfer

  while ( !( GDO0_PxIN & GDO0_PIN ) ); // Wait until GDO0 goes high (sync word txed)
  
  // Transmitting

  while ( GDO0_PxIN & GDO0_PIN );      // Wait until GDO0 goes low (end of packet)
  
  strobe( SRX );
  
  GDO0_PxIFG &= ~GDO0_PIN;             // Clear flag  
}

/*******************************************************************************
 * @fn     void tx_done( )
 * @brief  Called at the end of transmission
 * ****************************************************************************/
inline void tx_done( )
{
  
}


/*******************************************************************************
 * @fn     rx_enable( )
 * @brief  Enable Rx
 * ****************************************************************************/
inline void rx_enable()
{

}

/*******************************************************************************
 * @fn     rx_disable( )
 * @brief  Disable Rx
 * ****************************************************************************/
inline void rx_disable()
{

}

/*******************************************************************************
 * @fn     void dummy_callback( void )
 * @brief  empty function works as default callback
 * ****************************************************************************/
static uint8_t dummy_callback( uint8_t* buffer, uint8_t size )
{
  __no_operation();

  return 0;
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
 * @fn     void port2_isr( void )
 * @brief  UART ISR
 * ****************************************************************************/
wakeup interrupt ( PORT2_VECTOR ) port2_isr(void) // CHANGE
{
  uint8_t length = CC2500_BUFFER_SIZE; 
  
  // Check
  if ( GDO0_PxIFG & GDO0_PIN )
  {
      if( receive_packet( p_rx_buffer, &length ) )
      {
        rx_callback(p_rx_buffer, length );
      }
           
  }
  GDO0_PxIFG &= ~GDO0_PIN;           // Clear flag 

  strobe( SRX ); // enter receive mode again
}
