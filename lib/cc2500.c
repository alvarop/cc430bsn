/** @file cc2500.c
*
* @brief Radio functions for CC2500
*
* @author Alvaro Prieto
*/
#include "cc2500.h"
#include "uart.h"
#include "leds.h"



// Phase transition time = 0 
// Base frequency = 2432.499634 
// Carrier frequency = 2445.246521 
// Channel number = 255 
// Carrier frequency = 2445.246521 
// Modulated = true 
// Modulation format = MSK 
// Manchester enable = false 
// Sync word qualifier mode = 30/32 sync word bits detected 
// Preamble count = 2 
// Channel spacing = 49.987793 
// Carrier frequency = 2445.246521 
// Data rate = 249.939 
// RX filter BW = 541.666667 
// Data format = Normal mode 
// CRC enable = true 
// Device address = 0 
// Address config = No address check 
// CRC autoflush = false 
// TX power = 0 dBm
cc2500_settings_t cc2500_settings = {
  0x29,  // IOCFG2              GDO2Output Pin Configuration 
  0x2E,  // IOCFG1              GDO1Output Pin Configuration 
  0x06,  // IOCFG0              GDO0Output Pin Configuration 
  0x07,  // FIFOTHR             RX FIFO and TX FIFO Thresholds
  0xD3,  // SYNC1               Sync Word, High Byte 
  0x91,  // SYNC0               Sync Word, Low Byte 
  0xFF,  // PKTLEN              Packet Length 
  0x04,  // PKTCTRL1            Packet Automation Control
  0x05,  // PKTCTRL0            Packet Automation Control
  0x00,  // ADDR                Device Address 
  0xFF,  // CHANNR              Channel Number 
  0x12,  // FSCTRL1             Frequency Synthesizer Control 
  0x00,  // FSCTRL0             Frequency Synthesizer Control 
  0x5D,  // FREQ2               Frequency Control Word, High Byte 
  0x8E,  // FREQ1               Frequency Control Word, Middle Byte 
  0xC4,  // FREQ0               Frequency Control Word, Low Byte 
  0x2D,  // MDMCFG4             Modem Configuration 
  0x3B,  // MDMCFG3             Modem Configuration 
  0xF3,  // MDMCFG2             Modem Configuration
  0x00,  // MDMCFG1             Modem Configuration
  0xF8,  // MDMCFG0             Modem Configuration 
  0x00,  // DEVIATN             Modem Deviation Setting 
  0x07,  // MCSM2               Main Radio Control State Machine Configuration 
  0x30,  // MCSM1               Main Radio Control State Machine Configuration
  0x18,  // MCSM0               Main Radio Control State Machine Configuration 
  0x1D,  // FOCCFG              Frequency Offset Compensation Configuration
  0x1C,  // BSCFG               Bit Synchronization Configuration
  0xC7,  // AGCCTRL2            AGC Control
  0x00,  // AGCCTRL1            AGC Control
  0xB0,  // AGCCTRL0            AGC Control
  0x87,  // WOREVT1             High Byte Event0 Timeout 
  0x6B,  // WOREVT0             Low Byte Event0 Timeout 
  0xF8,  // WORCTRL             Wake On Radio Control
  0xB6,  // FREND1              Front End RX Configuration 
  0x10,  // FREND0              Front End TX configuration 
  0xEA,  // FSCAL3              Frequency Synthesizer Calibration 
  0x0A,  // FSCAL2              Frequency Synthesizer Calibration 
  0x00,  // FSCAL1              Frequency Synthesizer Calibration 
  0x11,  // FSCAL0              Frequency Synthesizer Calibration 
  0x41,  // RCCTRL1             RC Oscillator Configuration 
  0x00,  // RCCTRL0             RC Oscillator Configuration 
  0x59,  // FSTEST              Frequency Synthesizer Calibration Control 
  0x7F,  // PTEST               Production Test 
  0x3F,  // AGCTEST             AGC Test 
  0x88,  // TEST2               Various Test Settings 
  0x31,  // TEST1               Various Test Settings 
  0x0B,  // TEST0               Various Test Settings 
};

uint8_t write_default_radio_register ( uint8_t* p_setting )
{
    
  IE2 &= ~UCB0RXIE;          // Disable USCI_B0 RX interrupt

  //
  // Compute register address using address of parameter in cc2500_settings_t
  // Subtract address of structure from address of parameter
  //
  
  //
  // Write byte with address and write bit
  //   
  spi_put_char( 
  ( (int8_t)(p_setting - (uint8_t*)&cc2500_settings) ) & ADDRESS_MASK );
  
  while( !( IFG2 & UCB0RXIFG ));
  IFG2 &= ~UCB0RXIFG;
  //
  // Write data
  //
  spi_put_char( *p_setting );
  while( !( IFG2 & UCB0RXIFG ));
  IFG2 &= ~UCB0RXIFG;

  IE2 |= UCB0RXIE;          // Enable USCI_B0 RX interrupt
  led1_toggle();
  return UCB0RXBUF;
}

