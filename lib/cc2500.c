/** @file cc2500.c
*
* @brief Radio functions for CC2500
*
* @author Alvaro Prieto
*/
#include "cc2500.h"
#include "intrinsics.h"
#include "uart.h"


uint8_t buffer[10];

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
  0x30,  // IOCFG2              GDO2Output Pin Configuration 
  0x2E,  // IOCFG1              GDO1Output Pin Configuration 
  0x06,  // IOCFG0              GDO0Output Pin Configuration 
  0x07,  // FIFOTHR             RX FIFO and TX FIFO Thresholds
  0xD3,  // SYNC1               Sync Word, High Byte 
  0x91,  // SYNC0               Sync Word, Low Byte 
  0xFF,  // PKTLEN              Packet Length 
  0x04,  // PKTCTRL1            Packet Automation Control
  0x05,  // PKTCTRL0            Packet Automation Control
  0xcc,  // ADDR                Device Address 
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

//
// @fn write_register ( uint8_t* p_setting )
// @brief Write single register value to CC2500
//
void write_register ( uint8_t* p_setting )
{ 
  //
  // Compute register address using address of parameter in cc2500_settings_t
  // Subtract address of structure from address of parameter
  //
  
  //
  // Write byte with address and write bit
  //
         
  P1OUT &= ~BIT7;                 // CSn enable
    
  while ( !( UCB0IFG & UCTXIFG ) );	// USCI_B0 TX buffer ready?
                                  // send address
  UCB0TXBUF = ( (int8_t)(p_setting - (uint8_t*)&cc2500_settings) ) 
                                                                & ADDRESS_MASK; 
    
  //
  // Write data
  //
  while ( !( UCB0IFG & UCTXIFG ) );  // USCI_B0 TX buffer ready?
  UCB0TXBUF = *p_setting;         // Send value
  while ( UCB0STAT & UCBUSY );    // wait for TX to complete
  
  P1OUT |= BIT7;                  // CSn disable
  
}

//
// @fn write_burst_register( uint8_t address, uint8_t* buffer, uint8_t size )
// @brief Write multiple values to CC2500
//
void write_burst_register( uint8_t address, uint8_t* buffer, uint8_t size )
{
  uint16_t index;
  
  P1OUT &= ~BIT7;                 // CSn enable
  
  while ( !( UCB0IFG & UCTXIFG ) );	// USCI_B0 TX buffer ready?
  UCB0TXBUF = address | BURST_BIT;// send address byte
  
  for( index = 0; index < size; index++ )
  {
    while ( !( UCB0IFG & UCTXIFG ) );// USCI_B0 TX buffer ready?
    UCB0TXBUF = buffer[index];
  }
 
  while ( UCB0STAT & UCBUSY );    // wait for TX to complete

  P1OUT |= BIT7;                  // CSn disable
}

//
// @fn uint8_t read_register ( uint8_t* p_setting )
// @brief read single register from CC2500
//
uint8_t read_register ( uint8_t* p_setting )
{
  uint8_t rx_char;

  P1OUT &= ~BIT7;                 // CSn enable
  
  while (!(UCB0IFG & UCTXIFG));	    // USCI_B0 TX buffer ready?
                                  // send address with read bit
  UCB0TXBUF = ( 
  ( (int8_t)(p_setting - (uint8_t*)&cc2500_settings) ) & ADDRESS_MASK )
                                                                     | RW_BIT; 
  while ( !( UCB0IFG & UCTXIFG ) );	// USCI_B0 TX buffer ready?
  UCB0TXBUF = 0;                  // dummy write

  while ( UCB0STAT & UCBUSY );    // wait for TX to complete
  rx_char = UCB0RXBUF;            // Read data
 
  P1OUT |= BIT7;                  // CSn disable
 
  return rx_char; 
}

//
// @fn read_burst_register( uint8_t address, uint8_t* buffer, uint8_t size )
// @brief read multiple registers from CC2500
//
void read_burst_register( uint8_t address, uint8_t* buffer, uint8_t size )
{
  uint16_t index;
  
  P1OUT &= ~BIT7;                 // CSn enable
  
  while ( !( UCB0IFG & UCTXIFG ) );	// USCI_B0 TX buffer ready?
                                  // send address byte
  UCB0TXBUF = address | BURST_BIT | RW_BIT;
  while ( UCB0STAT & UCBUSY );    // wait for TX to complete
  UCB0TXBUF = 0;                  // dummy write

  UCB0IFG &= ~UCRXIFG;            // clear rx flag
                                  // wait for end of first tx byte
  while ( !( UCB0IFG & UCRXIFG ) );
  for( index = 0; index < (size-1); index++ )
  {
    UCB0TXBUF = 0;                // dummy write
    buffer[index] = UCB0RXBUF;    // store data byte in buffer
                                  // wait for RX to finish  
    while ( !( UCB0IFG & UCRXIFG ) );
  }
  buffer[index] = UCB0RXBUF;    // store last data byte
  
  P1OUT |= BIT7;                  // CSn disable
}

//
// @fn uint8_t read_status( uint8_t address )
// @brief send status command and read returned status byte
//
uint8_t read_status( uint8_t address )
{
  uint8_t status;
  
  P1OUT &= ~BIT7;                 // CSn enable  
  while ( !( UCB0IFG & UCTXIFG ) );	// USCI_B0 TX buffer ready?
  UCB0TXBUF = address | BURST_BIT | RW_BIT;
  while ( !( UCB0IFG & UCTXIFG ) );	// USCI_B0 TX buffer ready?
  UCB0TXBUF = 0;                  // dummy write
  while ( UCB0STAT & UCBUSY );    // wait for TX to complete
  status = UCB0RXBUF;             // read status byte
  P1OUT |= BIT7;                  // CSn disable
  
  return status;  
}

//
// @fn void strobe ( uint8_t strobe_byte )
// @brief send strobe command
//
uint8_t strobe ( uint8_t strobe_byte )
{  
  P1OUT &= ~BIT7;                 // CSn enable  
  while ( !( UCB0IFG & UCTXIFG ) );	// USCI_B0 TX buffer ready?
  UCB0TXBUF = strobe_byte;        // send strobe
  while ( UCB0STAT & UCBUSY );    // wait for TX to complete
  P1OUT |= BIT7;                  // CSn disable
  
  return UCB0RXBUF;
}

//
// @fn void initialize_radio()
// @brief reset radio
//
void initialize_radio()
{
  P1OUT |= BIT7;                  // CSn disable
  __delay_cycles(30);
  P1OUT &= ~BIT7;                 // CSn enable
  __delay_cycles(30);
  P1OUT |= BIT7;                  // CSn disable
  __delay_cycles(45);

  strobe( SRES );
}


//
// @fn void write_rf_settings()
// @brief write radio settings
//
void write_rf_settings()
{
  //write_register( &cc2500_settings.iocfg2 );    // GDO2Output Pin Configuration 
  //write_register( &cc2500_settings.iocfg1 );    // GDO1Output Pin Configuration 
  write_register( &cc2500_settings.iocfg0 );    // GDO0Output Pin Configuration 
  //write_register( &cc2500_settings.fifothr );   // RX FIFO and TX FIFO Thresholds
  //write_register( &cc2500_settings.sync1 );     // Sync Word, High Byte 
  //write_register( &cc2500_settings.sync0 );     // Sync Word, Low Byte 
  write_register( &cc2500_settings.pktlen );    // Packet Length 
  write_register( &cc2500_settings.pktctrl1 );  // Packet Automation Control
  write_register( &cc2500_settings.pktctrl0 );  // Packet Automation Control
  write_register( &cc2500_settings.addr );      // Device Address 
  write_register( &cc2500_settings.channr );    // Channel Number 
  write_register( &cc2500_settings.fsctrl1 );   // Frequency Synthesizer Control 
  write_register( &cc2500_settings.fsctrl0 );   // Frequency Synthesizer Control 
  write_register( &cc2500_settings.freq2 );     // Frequency Control Word, High Byte 
  write_register( &cc2500_settings.freq1 );     // Frequency Control Word, Middle Byte 
  write_register( &cc2500_settings.freq0 );     // Frequency Control Word, Low Byte 
  write_register( &cc2500_settings.mdmcfg4 );   // Modem Configuration 
  write_register( &cc2500_settings.mdmcfg3 );   // Modem Configuration 
  write_register( &cc2500_settings.mdmcfg2 );   // Modem Configuration
  write_register( &cc2500_settings.mdmcfg1 );   // Modem Configuration
  write_register( &cc2500_settings.mdmcfg0 );   // Modem Configuration 
  write_register( &cc2500_settings.deviatn );   // Modem Deviation Setting 
  write_register( &cc2500_settings.mcsm2 );     // Main Radio Control State Machine Configuration 
  write_register( &cc2500_settings.mcsm1 );     // Main Radio Control State Machine Configuration
  write_register( &cc2500_settings.mcsm0 );     // Main Radio Control State Machine Configuration 
  write_register( &cc2500_settings.foccfg );    // Frequency Offset Compensation Configuration
  write_register( &cc2500_settings.bscfg );     // Bit Synchronization Configuration
  write_register( &cc2500_settings.agcctrl2 );  // AGC Control
  write_register( &cc2500_settings.agcctrl1 );  // AGC Control
  write_register( &cc2500_settings.agcctrl0 );  // AGC Control
  //write_register( &cc2500_settings.worevt1 );   // High Byte Event0 Timeout 
  //write_register( &cc2500_settings.worevt0 );   // Low Byte Event0 Timeout 
  //write_register( &cc2500_settings.worctrl );   // Wake On Radio Control
  write_register( &cc2500_settings.frend1 );    // Front End RX Configuration 
  write_register( &cc2500_settings.frend0 );    // Front End TX configuration 
  write_register( &cc2500_settings.fscal3 );    // Frequency Synthesizer Calibration 
  write_register( &cc2500_settings.fscal2 );    // Frequency Synthesizer Calibration 
  write_register( &cc2500_settings.fscal1 );    // Frequency Synthesizer Calibration 
  write_register( &cc2500_settings.fscal0 );    // Frequency Synthesizer Calibration 
  //write_register( &cc2500_settings.rcctrl1 );   // RC Oscillator Configuration 
  //write_register( &cc2500_settings.rcctrl0 );   // RC Oscillator Configuration 
  //write_register( &cc2500_settings.fstest );    // Frequency Synthesizer Calibration Control 
  //write_register( &cc2500_settings.ptest );     // Production Test 
  //write_register( &cc2500_settings.agctest );   // AGC Test 
  write_register( &cc2500_settings.test2 );     // Various Test Settings 
  write_register( &cc2500_settings.test1 );     // Various Test Settings 
  write_register( &cc2500_settings.test0 );     // Various Test Settings  
}

