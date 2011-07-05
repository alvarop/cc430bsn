/** @file spitest.c
*
* @brief Set up a bridge between serial port and SPI port to test radio
*
* @author Alvaro Prieto
*/
#include "common.h"

#include "intrinsics.h"
#include "leds.h"
#include "uart.h"
#include "cc2500.h"

extern cc2500_settings_t cc2500_settings;

int main( void )
{
  uint8_t string[] = "UART Test Program!\r\nEcho mode...\r\n";
  uint8_t status_byte;
  /* Init watchdog timer to off */
  WDTCTL = WDTPW|WDTHOLD;
  
  BCSCTL1 = CALBC1_12MHZ;
  DCOCTL = CALDCO_12MHZ; 
  
  __delay_cycles(0x40000);
  
  setup_leds();
  
  setup_uart();
  
  __bis_SR_register(GIE);		// enable general interrupts
  
  uart_write( string, sizeof(string) );
  
  spi_put_char( SRES );
    
  spi_put_char( SNOP );
  
  write_default_radio_register( &cc2500_settings.iocfg2 );           // GDO2Output Pin Configuration 
  write_default_radio_register( &cc2500_settings.iocfg1 );           // GDO1Output Pin Configuration 
  write_default_radio_register( &cc2500_settings.iocfg0 );           // GDO0Output Pin Configuration 
  write_default_radio_register( &cc2500_settings.fifothr );          // RX FIFO and TX FIFO Thresholds
  write_default_radio_register( &cc2500_settings.sync1 );            // Sync Word, High Byte 
  write_default_radio_register( &cc2500_settings.sync0 );            // Sync Word, Low Byte 
  write_default_radio_register( &cc2500_settings.pktlen );           // Packet Length 
  write_default_radio_register( &cc2500_settings.pktctrl1 );         // Packet Automation Control
  write_default_radio_register( &cc2500_settings.pktctrl0 );         // Packet Automation Control
  write_default_radio_register( &cc2500_settings.addr );             // Device Address 
  write_default_radio_register( &cc2500_settings.channr );           // Channel Number 
  write_default_radio_register( &cc2500_settings.fsctrl1 );          // Frequency Synthesizer Control 
  write_default_radio_register( &cc2500_settings.fsctrl0 );          // Frequency Synthesizer Control 
  write_default_radio_register( &cc2500_settings.freq2 );            // Frequency Control Word, High Byte 
  write_default_radio_register( &cc2500_settings.freq1 );            // Frequency Control Word, Middle Byte 
  write_default_radio_register( &cc2500_settings.freq0 );            // Frequency Control Word, Low Byte 
  write_default_radio_register( &cc2500_settings.mdmcfg4 );          // Modem Configuration 
  write_default_radio_register( &cc2500_settings.mdmcfg3 );          // Modem Configuration 
  write_default_radio_register( &cc2500_settings.mdmcfg2 );          // Modem Configuration
  write_default_radio_register( &cc2500_settings.mdmcfg1 );          // Modem Configuration
  write_default_radio_register( &cc2500_settings.mdmcfg0 );          // Modem Configuration 
  write_default_radio_register( &cc2500_settings.deviatn );          // Modem Deviation Setting 
  write_default_radio_register( &cc2500_settings.mcsm2 );            // Main Radio Control State Machine Configuration 
  write_default_radio_register( &cc2500_settings.mcsm1 );            // Main Radio Control State Machine Configuration
  write_default_radio_register( &cc2500_settings.mcsm0 );            // Main Radio Control State Machine Configuration 
  write_default_radio_register( &cc2500_settings.foccfg );           // Frequency Offset Compensation Configuration
  write_default_radio_register( &cc2500_settings.bscfg );            // Bit Synchronization Configuration
  write_default_radio_register( &cc2500_settings.agcctrl2 );         // AGC Control
  write_default_radio_register( &cc2500_settings.agcctrl1 );         // AGC Control
  write_default_radio_register( &cc2500_settings.agcctrl0 );         // AGC Control
  write_default_radio_register( &cc2500_settings.worevt1 );          // High Byte Event0 Timeout 
  write_default_radio_register( &cc2500_settings.worevt0 );          // Low Byte Event0 Timeout 
  write_default_radio_register( &cc2500_settings.worctrl );          // Wake On Radio Control
  write_default_radio_register( &cc2500_settings.frend1 );           // Front End RX Configuration 
  write_default_radio_register( &cc2500_settings.frend0 );           // Front End TX configuration 
  write_default_radio_register( &cc2500_settings.fscal3 );           // Frequency Synthesizer Calibration 
  write_default_radio_register( &cc2500_settings.fscal2 );           // Frequency Synthesizer Calibration 
  write_default_radio_register( &cc2500_settings.fscal1 );           // Frequency Synthesizer Calibration 
  write_default_radio_register( &cc2500_settings.fscal0 );           // Frequency Synthesizer Calibration 
  write_default_radio_register( &cc2500_settings.rcctrl1 );          // RC Oscillator Configuration 
  write_default_radio_register( &cc2500_settings.rcctrl0 );          // RC Oscillator Configuration 
  write_default_radio_register( &cc2500_settings.fstest );           // Frequency Synthesizer Calibration Control 
  write_default_radio_register( &cc2500_settings.ptest );            // Production Test 
  write_default_radio_register( &cc2500_settings.agctest );          // AGC Test 
  write_default_radio_register( &cc2500_settings.test2 );            // Various Test Settings 
  write_default_radio_register( &cc2500_settings.test1 );            // Various Test Settings 
  write_default_radio_register( &cc2500_settings.test0 );            // Various Test Settings 
  
  spi_put_char( SRX );
  
  for (;;) {
    
    //led1_toggle();
    // NOP Strobe
    //spi_put_char( SNOP );
    //hex_to_string( string, &status_byte, 1 ); uart_write( string, 2 );

    __delay_cycles(0x40000);
    __delay_cycles(0x40000);
    __delay_cycles(0x40000);
    
  }  /* while */
}





