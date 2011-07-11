/** @file radio_cc2500.c
*
* @brief CC2500 radio functions
*
* @author Alvaro Prieto
*/
#include "radio_cc2500.h"
#include "cc2500.h"
#include <signal.h>

static uint8_t dummy_callback( uint8_t*, uint8_t );
inline void tx_done( void );
inline void rx_enable();
inline void rx_disable();

// Receive buffer
static uint8_t rx_buffer[RX_BUFFER_SIZE];

// Radio mode holds whether or not radio is transmitting or receiving
volatile uint8_t radio_mode = RADIO_RX;

extern cc2500_settings_t cc2500_settings;

// Holds pointers to all callback functions for CCR registers (and overflow)
static uint8_t (*rx_callback)( uint8_t*, uint8_t ) = dummy_callback;

/*******************************************************************************
 * @fn     void setup_radio( uint8_t (*callback)(void) )
 * @brief  Initialize radio and register Rx Callback function
 * ****************************************************************************/
void setup_radio( uint8_t (*callback)(uint8_t*, uint8_t) )
{
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
void radio_tx( uint8_t* buffer, uint8_t size )
{

  
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

