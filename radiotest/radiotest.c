/******************************************************************************
* CC430 RF Code Example - TX and RX (fixed packet length =< FIFO size)
*
* Simple RF Link to Toggle Receiver's LED by pressing Transmitter's Button    
* Warning: This RF code example is setup to operate at either 868 or 915 MHz, 
* which might be out of allowable range of operation in certain countries.
* The frequency of operation is selectable as an active build configuration
* in the project menu. 
* 
* Please refer to the appropriate legal sources before performing tests with 
* this code example. 
* 
* This code example can be loaded to 2 CC430 devices. Each device will transmit 
* a small packet, less than the FIFO size, upon a button pressed. Each device will also toggle its LED 
* upon receiving the packet. 
* 
* The RF packet engine settings specify fixed-length-mode with CRC check 
* enabled. The RX packet also appends 2 status bytes regarding CRC check, RSSI 
* and LQI info. For specific register settings please refer to the comments for 
* each register in RfRegSettings.c, the CC430x613x User's Guide, and/or 
* SmartRF Studio.
* 
* M. Morales/D. Dang
* Texas Instruments Inc.
* June 2010
* Built with IAR v4.21 and CCS v4.1
******************************************************************************/

#include "radiotest.h"
#include <signal.h>
#include "leds.h"
#include "oscillator.h"

#define  PACKET_LEN         (0x05)			// PACKET_LEN <= 61
#define  RSSI_IDX           (PACKET_LEN)    // Index of appended RSSI 
#define  CRC_LQI_IDX        (PACKET_LEN+1)  // Index of appended LQI, checksum
#define  CRC_OK             (BIT7)          // CRC_OK bit 
#define  PATABLE_VAL        (0x51)          // 0 dBm output 

extern RF_SETTINGS rfSettings;

uint8_t packetReceived;
uint8_t packetTransmit; 

uint8_t RxBuffer[PACKET_LEN+2];
uint8_t RxBufferLength = 0;
const uint8_t TxBuffer[PACKET_LEN]= {0xAA, 0xBB, 0xCC, 0xDD, 0xEE};
uint8_t buttonPressed = 0;
unsigned int i = 0; 

uint8_t transmitting = 0; 
uint8_t receiving = 0; 


int main( void )
{  
  // Stop watchdog timer to prevent time out reset 
  WDTCTL = WDTPW + WDTHOLD; 
  
  setup_oscillator();
  
  // Initialize LEDs
  setup_leds();

  // Increase PMMCOREV level to 2 for proper radio operation
  SetVCore(2);                            
  
  ResetRadioCore();     
  InitRadio();
  InitButtonLeds();
  
  ReceiveOn(); 
  receiving = 1; 
    
  while (1)
  { 
    __bis_SR_register( LPM3_bits + GIE );   
    __no_operation(); 
    
    if (buttonPressed)                      // Process a button press->transmit
    {
      led1_on();                            // Pulse LED during Transmit
      buttonPressed = 0; 
      P1IFG = 0; 
      
      ReceiveOff();
      receiving = 0; 
      Transmit( (uint8_t*)TxBuffer, sizeof TxBuffer);         
      transmitting = 1;
       
      P1IE |= BIT0;                         // Re-enable button press  
    }
    else if(!transmitting)
    {
      ReceiveOn();      
      receiving = 1; 
    }
  }
  
  return 0;
}

void InitButtonLeds(void)
{
  // Set up the button as interruptible 
  P1DIR &= ~BIT0;
  P1REN |= BIT0;
  P1IES &= BIT0;
  P1IFG = 0;
  P1OUT |= BIT0;
  P1IE  |= BIT0; 

}

void InitRadio(void)
{
  // Set the High-Power Mode Request Enable bit so LPM3 can be entered
  // with active radio enabled 
  PMMCTL0_H = 0xA5;
  PMMCTL0_L |= PMMHPMRE; // CHANGE from PMMHPMRE_L
  PMMCTL0_H = 0x00; 
  
  WriteRfSettings(&rfSettings);
  
  WriteSinglePATable(PATABLE_VAL);
}

wakeup interrupt (PORT1_VECTOR) p1_isr (void)
{
  // CHANGED from __even_in_range(P1IV, 16)
  switch( P1IV ) 
  {
    case  0: break;
    case  2:                                // P1.0 IFG
    {
      P1IE = 0;                             // Debounce by disabling buttons
      buttonPressed = 1;
      __bic_SR_register_on_exit(LPM3_bits); // Exit active 
      break;
    }
    case  4: break;                         // P1.1 IFG
    case  6: break;                         // P1.2 IFG
    case  8: break;                         // P1.3 IFG
    case 10: break;                         // P1.4 IFG
    case 12: break;                         // P1.5 IFG
    case 14: break;                         // P1.6 IFG
    case 16: break;                         // P1.7 IFG
         
      break;
  }
}

void Transmit(uint8_t *buffer, uint8_t length)
{
  
  RF1AIES |= BIT9;                          
  RF1AIFG &= ~BIT9;                         // Clear pending interrupts
  RF1AIE |= BIT9;                           // Enable TX end-of-packet interrupt
  
  WriteBurstReg(RF_TXFIFOWR, buffer, length);     
  
  Strobe( RF_STX );                         // Strobe STX   
}

void ReceiveOn(void)
{  
  RF1AIES |= BIT9;                          // Falling edge of RFIFG9
  RF1AIFG &= ~BIT9;                         // Clear a pending interrupt
  RF1AIE  |= BIT9;                          // Enable the interrupt 
  
  // Radio is in IDLE following a TX, so strobe SRX to enter Receive Mode
  Strobe( RF_SRX );                      
}

void ReceiveOff(void)
{
  RF1AIE &= ~BIT9;                          // Disable RX interrupts
  RF1AIFG &= ~BIT9;                         // Clear pending IFG

  // It is possible that ReceiveOff is called while radio is receiving a packet.
  // Therefore, it is necessary to flush the RX FIFO after issuing IDLE strobe 
  // such that the RXFIFO is empty prior to receiving a packet.
  Strobe( RF_SIDLE );
  Strobe( RF_SFRX  );                       
}

wakeup interrupt (CC1101_VECTOR) radio_isr (void)
{
  uint16_t vector_flag;
  
  //
  // NOTE: For some reason, the switch statement with argument RF1AIV does not
  //        work. Adding the temporary variable 'vector_flag' fixes the problem
  //
  vector_flag = RF1AIV;
  
  // CHANGED from __even_in_range(RF1AIV,32)
  switch(vector_flag)        // Prioritizing Radio Core Interrupt 
  {
    case RF1AIV_NONE:   break;    // No RF core interrupt pending                                            
    case RF1AIV_RFIFG0: break;    // RFIFG0 
    case RF1AIV_RFIFG1: break;    // RFIFG1
    case RF1AIV_RFIFG2: break;    // RFIFG2
    case RF1AIV_RFIFG3: break;    // RFIFG3
    case RF1AIV_RFIFG4: break;    // RFIFG4
    case RF1AIV_RFIFG5: break;    // RFIFG5
    case RF1AIV_RFIFG6: break;    // RFIFG6          
    case RF1AIV_RFIFG7: break;    // RFIFG7
    case RF1AIV_RFIFG8: break;    // RFIFG8
    case RF1AIV_RFIFG9:           // RFIFG9
    {
      
      if(receiving)			    // RX end of packet
      {
        // Read the length byte from the FIFO       
        RxBufferLength = ReadSingleReg( RXBYTES );               
        ReadBurstReg(RF_RXFIFORD, RxBuffer, RxBufferLength); 
        
        // Stop here to see contents of RxBuffer
        __no_operation(); 		   
        
        // Check the CRC results
        if(RxBuffer[CRC_LQI_IDX] & CRC_OK)  
          led2_toggle();                    // Toggle LED1      
      }
      else if(transmitting)		    // TX end of packet
      {
        RF1AIE &= ~BIT9;                    // Disable TX end-of-packet interrupt
        led1_off();                         // Turn off LED after Transmit               
        transmitting = 0; 
      }
      else while(1); 			    // trap 
      break;
    }
    case RF1AIV_RFIFG10: break;                         // RFIFG10
    case RF1AIV_RFIFG11: break;                         // RFIFG11
    case RF1AIV_RFIFG12: break;                         // RFIFG12
    case RF1AIV_RFIFG13: break;                         // RFIFG13
    case RF1AIV_RFIFG14: break;                         // RFIFG14
    case RF1AIV_RFIFG15: break;                         // RFIFG15
    default: break;
  }  
  __bic_SR_register_on_exit(LPM3_bits);     
}

