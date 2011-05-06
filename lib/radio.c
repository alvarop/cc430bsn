/** @file radio.c
*
* @brief Radio functions
*
* @author Alvaro Prieto
*/
#include "radio.h"
#include <signal.h>

static uint8_t dummy_callback( uint8_t*, uint8_t );
inline void tx_done( void );
inline void rx_enable();
inline void rx_disable();

// Receive buffer
static uint8_t rx_buffer[RX_BUFFER_SIZE];

// Radio mode holds whether or not radio is transmitting or receiving
volatile uint8_t radio_mode = RADIO_RX;

extern RF_SETTINGS rfSettings;

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
  
  // Increase PMMCOREV level to 2 for proper radio operation
  SetVCore(2);
  
  ResetRadioCore();
  
  // Set the High-Power Mode Request Enable bit so LPM3 can be entered
  // with active radio enabled
  PMMCTL0_H = 0xA5;
  PMMCTL0_L |= PMMHPMRE; // CHANGE from PMMHPMRE_L
  PMMCTL0_H = 0x00;
  
  WriteRfSettings(&rfSettings);
  
  WriteSinglePATable(PATABLE_VAL);

  rx_enable();
}

/*******************************************************************************
 * @fn     void radio_tx( uint8_t* buffer, uint8_t size )
 * @brief  Send message through radio
 * ****************************************************************************/
void radio_tx( uint8_t* buffer, uint8_t size )
{
  rx_disable();
  radio_mode = RADIO_TX;
    
  RF1AIES |= BIT9;
  RF1AIFG &= ~BIT9; // Clear pending interrupts
  RF1AIE |= BIT9; // Enable TX end-of-packet interrupt
  
  WriteBurstReg(RF_TXFIFOWR, buffer, size);
  
  Strobe( RF_STX ); // Strobe STX
  
}

/*******************************************************************************
 * @fn     void tx_done( )
 * @brief  Called at the end of transmission
 * ****************************************************************************/
inline void tx_done( )
{
  rx_enable();
}


/*******************************************************************************
 * @fn     rx_enable( )
 * @brief  Enable Rx
 * ****************************************************************************/
inline void rx_enable()
{
  radio_mode = RADIO_RX;

  RF1AIES |= BIT9; // Falling edge of RFIFG9
  RF1AIFG &= ~BIT9; // Clear a pending interrupt
  RF1AIE |= BIT9; // Enable the interrupt
  
  // Radio is in IDLE following a TX, so strobe SRX to enter Receive Mode
  Strobe( RF_SRX );
}

/*******************************************************************************
 * @fn     rx_disable( )
 * @brief  Disable Rx
 * ****************************************************************************/
inline void rx_disable()
{
  RF1AIE &= ~BIT9; // Disable RX interrupts
  RF1AIFG &= ~BIT9; // Clear pending IFG  // Increase PMMCOREV level to 2 for proper radio operation
  SetVCore(2);

  // It is possible that ReceiveOff is called while radio is receiving a packet.
  // Therefore, it is necessary to flush the RX FIFO after issuing IDLE strobe
  // such that the RXFIFO is empty prior to receiving a packet.
  Strobe( RF_SIDLE );
  Strobe( RF_SFRX );
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
 * @fn     void radio_isr( void )
 * @brief  
 * ****************************************************************************/
wakeup interrupt (CC1101_VECTOR) radio_isr (void)
{
  uint16_t vector_flag;
  uint8_t rx_message_size;
  //
  // NOTE: For some reason, the switch statement with argument RF1AIV does not
  // work. Adding the temporary variable 'vector_flag' fixes the problem
  //
  vector_flag = RF1AIV;
  
  // CHANGED from __even_in_range(RF1AIV,32)
  switch(vector_flag) // Prioritizing Radio Core Interrupt
  {
    case RF1AIV_NONE: break; // No RF core interrupt pending
    case RF1AIV_RFIFG0: break; // RFIFG0
    case RF1AIV_RFIFG1: break; // RFIFG1
    case RF1AIV_RFIFG2: break; // RFIFG2
    case RF1AIV_RFIFG3: break; // RFIFG3
    case RF1AIV_RFIFG4: break; // RFIFG4
    case RF1AIV_RFIFG5: break; // RFIFG5
    case RF1AIV_RFIFG6: break; // RFIFG6
    case RF1AIV_RFIFG7: break; // RFIFG7
    case RF1AIV_RFIFG8: break; // RFIFG8
    case RF1AIV_RFIFG9: // RFIFG9
    {
      
      if(radio_mode == RADIO_RX) 
      {
        // Read the length byte from the FIFO
        rx_message_size = ReadSingleReg( RXBYTES );
        ReadBurstReg(RF_RXFIFORD, rx_buffer, rx_message_size);
        
        // Stop here to see contents of RxBuffer
        __no_operation();
        
        // Check the CRC results
        if(rx_buffer[rx_message_size + CRC_LQI_IDX_OFFSET] & CRC_OK)
        {
          if ( rx_callback(rx_buffer, rx_message_size) )
          {
            // If callback function returns 1, wake up after interrupt
            // Otherwise, stay in whatever mode it is in.
            __bic_SR_register_on_exit(LPM3_bits);
          }
                    
        }
        
        // Not sure why this is needed, but it fixes a problem of not
        // receiving messages after the first one comes in
        rx_enable();
        
      }
      else if(radio_mode == RADIO_TX)
      {
        RF1AIE &= ~BIT9; // Disable TX end-of-packet interrupt        
        
        // Shouldn't be sleeping if it just transmitted, but in case it is
        // wake up after transmission
        __bic_SR_register_on_exit(LPM3_bits);
        
        // Clean up if needed
        tx_done();
      }
      else while(1); // trap
      break;
    }
    case RF1AIV_RFIFG10: break; // RFIFG10
    case RF1AIV_RFIFG11: break; // RFIFG11
    case RF1AIV_RFIFG12: break; // RFIFG12
    case RF1AIV_RFIFG13: break; // RFIFG13
    case RF1AIV_RFIFG14: break; // RFIFG14
    case RF1AIV_RFIFG15: break; // RFIFG15
    default: break;
  }
  
}

