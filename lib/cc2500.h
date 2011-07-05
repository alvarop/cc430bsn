/** @file cc2500.h
*
* @brief Radio functions for CC2500
*
* @author Alvaro Prieto
*/
#ifndef _CC2500_H
#define _CC2500_H

#include <stdint.h>

//
// Definitions for cc2500 communication
//
#define RW_BIT (1<<7)
#define BURST_BIT (1<<6)
#define ADDRESS_MASK (0x3f)

//
// Strobe Definitions
//
#define SRES    0x30
#define SFSTXON 0x31
#define SXOFF   0x32
#define SCAL    0x33
#define SRX     0x34
#define STX     0x35
#define SIDLE   0x36
#define SWOR    0x38
#define SPWD    0x39
#define SFRX    0x3a
#define SFTX    0x3b
#define SWORRST 0x3c
#define SNOP    0x3d

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
// TX power = 0 

typedef struct {
    uint8_t iocfg2;           // GDO2Output Pin Configuration 
    uint8_t iocfg1;           // GDO1Output Pin Configuration 
    uint8_t iocfg0;           // GDO0Output Pin Configuration 
    uint8_t fifothr;          // RX FIFO and TX FIFO Thresholds
    uint8_t sync1;            // Sync Word, High Byte 
    uint8_t sync0;            // Sync Word, Low Byte 
    uint8_t pktlen;           // Packet Length 
    uint8_t pktctrl1;         // Packet Automation Control
    uint8_t pktctrl0;         // Packet Automation Control
    uint8_t addr;             // Device Address 
    uint8_t channr;           // Channel Number 
    uint8_t fsctrl1;          // Frequency Synthesizer Control 
    uint8_t fsctrl0;          // Frequency Synthesizer Control 
    uint8_t freq2;            // Frequency Control Word, High Byte 
    uint8_t freq1;            // Frequency Control Word, Middle Byte 
    uint8_t freq0;            // Frequency Control Word, Low Byte 
    uint8_t mdmcfg4;          // Modem Configuration 
    uint8_t mdmcfg3;          // Modem Configuration 
    uint8_t mdmcfg2;          // Modem Configuration
    uint8_t mdmcfg1;          // Modem Configuration
    uint8_t mdmcfg0;          // Modem Configuration 
    uint8_t deviatn;          // Modem Deviation Setting 
    uint8_t mcsm2;            // Main Radio Control State Machine Configuration 
    uint8_t mcsm1;            // Main Radio Control State Machine Configuration
    uint8_t mcsm0;            // Main Radio Control State Machine Configuration 
    uint8_t foccfg;           // Frequency Offset Compensation Configuration
    uint8_t bscfg;            // Bit Synchronization Configuration
    uint8_t agcctrl2;         // AGC Control
    uint8_t agcctrl1;         // AGC Control
    uint8_t agcctrl0;         // AGC Control
    uint8_t worevt1;          // High Byte Event0 Timeout 
    uint8_t worevt0;          // Low Byte Event0 Timeout 
    uint8_t worctrl;          // Wake On Radio Control
    uint8_t frend1;           // Front End RX Configuration 
    uint8_t frend0;           // Front End TX configuration 
    uint8_t fscal3;           // Frequency Synthesizer Calibration 
    uint8_t fscal2;           // Frequency Synthesizer Calibration 
    uint8_t fscal1;           // Frequency Synthesizer Calibration 
    uint8_t fscal0;           // Frequency Synthesizer Calibration 
    uint8_t rcctrl1;          // RC Oscillator Configuration 
    uint8_t rcctrl0;          // RC Oscillator Configuration 
    uint8_t fstest;           // Frequency Synthesizer Calibration Control 
    uint8_t ptest;            // Production Test 
    uint8_t agctest;          // AGC Test 
    uint8_t test2;            // Various Test Settings 
    uint8_t test1;            // Various Test Settings 
    uint8_t test0;            // Various Test Settings 
} cc2500_settings_t;

//uint8_t write_radio_register ( uint8_t*, uint8_t );
uint8_t write_default_radio_register ( uint8_t* p_setting );
#endif /* _CC2500_H */\

