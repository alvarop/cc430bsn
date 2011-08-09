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
uint8_t receive_packet( uint8_t*, uint8_t* );

// Receive buffer
static uint8_t p_rx_buffer[CC2500_BUFFER_SIZE];

extern cc2500_settings_t cc2500_settings;

// Holds pointers to all callback functions for CCR registers (and overflow)
static uint8_t (*rx_callback)( uint8_t*, uint8_t ) = dummy_callback;

void debug_marcstate()
{
    uint8_t radio_status = read_status( MARCSTATE );
    
    debug(">");
    
    switch ( radio_status & 0x0f )
    {
      case 0x00:
        debug("SLEEP");
        break;
      case 0x01:
        debug("IDLE");
        break;
      case 0x02:
        debug("XOFF");
        break;
      case 0x03:
        debug("VCOON_MC");
        break;
      case 0x04:
        debug("REGON_MC");
        break;
      case 0x05:
        debug("MANCAL");
        break;
      case 0x06:
        debug("VCOON");
        break;
      case 0x07:
        debug("REGON");
        break;
      case 0x08:
        debug("STARTCAL");
        break;
      case 0x09:
        debug("BWBOOST");
        break;
      case 0x0A:
        debug("FS_LOCK");
        break;
      case 0x0B:
        debug("IFADCON");
        break;
      case 0x0C:
        debug("ENDCAL");
        break;
      case 0x0D:
        debug("RX");
        break;
      case 0x0E:
        debug("RX_END");
        break;
      case 0x0F:
        debug("RX_RST");
        break;
      case 0x10:
        debug("TXRX_SWITCH");
        break;
      case 0x11:
        debug("RXFIFO_OVERFLOW");
        break;
      case 0x12:
        debug("FSTXON");
        break;
      case 0x13:
        debug("TX");
        break;
      case 0x14:
        debug("TX_END");
        break;
      case 0x15:
        debug("RXTX_SWITCH");
        break;
      case 0x16:
        debug("TXFIFO_UNDERFLOW");
        break;
    }
  
    debug("\r\n");

}

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
  GDO0_PxIE &= ~GDO0_PIN;           // Disable interrupt
  
  write_burst_register( FIFO, p_buffer, size );
  
  strobe( STX );              // Change to TX mode, start data transfer
  
  while ( !( GDO0_PxIN & GDO0_PIN ) );  // Wait until GDO0 goes high (sync word txed)
  debug_marcstate();
  // Transmitting

  while ( GDO0_PxIN & GDO0_PIN );       // Wait until GDO0 goes low (end of packet)
  debug_marcstate();
  // Only needed if radio is configured to return to IDLE after transmission
  // Check register MCSM1.TXOFF_MODE
  //strobe( SRX ); 
  
  GDO0_PxIFG &= ~GDO0_PIN;              // Clear flag
  GDO0_PxIE |= GDO0_PIN;                // Enable interrupt  
}

/*******************************************************************************
 * @fn     cc2500_set_address( uint8_t );
 * @brief  Set device address
 * ****************************************************************************/
void cc2500_set_address( uint8_t address )
{
  cc2500_settings.addr = address;
  write_register( &cc2500_settings.addr );
}

/*******************************************************************************
 * @fn     cc2500_set_channel( uint8_t );
 * @brief  Set device channel
 * ****************************************************************************/
void cc2500_set_channel( uint8_t channel )
{
  cc2500_settings.channr = channel;
  write_register( &cc2500_settings.channr );
}

/*******************************************************************************
 * @fn     cc2500_set_power( uint8_t );
 * @brief  Set device transmit power
 * ****************************************************************************/
void cc2500_set_power( uint8_t power )
{
  //TODO use linear lookup table instead of raw values
  write_burst_register(PATABLE, &power, 1 ); // Set TX power
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
      
      // Flush RX FIFO
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
        uart_write("CRC OK - ", 9);
        rx_callback( p_rx_buffer, length );
        
      }
      else
      {
        uart_write("CRC NOK\r\n", 9);
      }
           
  }
  GDO0_PxIFG &= ~GDO0_PIN;           // Clear flag 
  
  // Only needed if radio is configured to return to IDLE after transmission
  // Check register MCSM1.TXOFF_MODE
  //strobe( SRX ); // enter receive mode again
}
