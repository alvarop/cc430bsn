/** @file RF1A.h
*
* @brief CC430 Radio communication functions
*
* @author M. Morales/D. Dang
* modified by Alvaro Prieto  
*/
#include "common.h"

/* ------------------------------------------------------------------------------------------------
 *                                          Defines
 * ------------------------------------------------------------------------------------------------
 */

/********************
 * Variable definition
 */
typedef struct S_RF_SETTINGS {
    uint8_t fsctrl1;   // Frequency synthesizer control.
    uint8_t fsctrl0;   // Frequency synthesizer control.
    uint8_t freq2;     // Frequency control word, high byte.
    uint8_t freq1;     // Frequency control word, middle byte.
    uint8_t freq0;     // Frequency control word, low byte.
    uint8_t mdmcfg4;   // Modem configuration.
    uint8_t mdmcfg3;   // Modem configuration.
    uint8_t mdmcfg2;   // Modem configuration.
    uint8_t mdmcfg1;   // Modem configuration.
    uint8_t mdmcfg0;   // Modem configuration.
    uint8_t channr;    // Channel number.
    uint8_t deviatn;   // Modem deviation setting (when FSK modulation is enabled).
    uint8_t frend1;    // Front end RX configuration.
    uint8_t frend0;    // Front end RX configuration.
    uint8_t mcsm0;     // Main Radio Control State Machine configuration.
    uint8_t foccfg;    // Frequency Offset Compensation Configuration.
    uint8_t bscfg;     // Bit synchronization Configuration.
    uint8_t agcctrl2;  // AGC control.
    uint8_t agcctrl1;  // AGC control.
    uint8_t agcctrl0;  // AGC control.
    uint8_t fscal3;    // Frequency synthesizer calibration.
    uint8_t fscal2;    // Frequency synthesizer calibration.
    uint8_t fscal1;    // Frequency synthesizer calibration.
    uint8_t fscal0;    // Frequency synthesizer calibration.
    uint8_t fstest;    // Frequency synthesizer calibration control
    uint8_t test2;     // Various test settings.
    uint8_t test1;     // Various test settings.
    uint8_t test0;     // Various test settings.
    uint8_t fifothr;   // RXFIFO and TXFIFO thresholds.
    uint8_t iocfg2;    // GDO2 output pin configuration
    uint8_t iocfg0;    // GDO0 output pin configuration
    uint8_t pktctrl1;  // Packet automation control.
    uint8_t pktctrl0;  // Packet automation control.
    uint8_t addr;      // Device address.
    uint8_t pktlen;    // Packet length.
} RF_SETTINGS;

void ResetRadioCore (void);
uint8_t Strobe(uint8_t strobe);

void WriteRfSettings(RF_SETTINGS *pRfSettings);

void WriteSingleReg(uint8_t addr, uint8_t value);
void WriteBurstReg(uint8_t addr, uint8_t *buffer, uint8_t count);
uint8_t ReadSingleReg(uint8_t addr);
void ReadBurstReg(uint8_t addr, uint8_t *buffer, uint8_t count);
void WriteSinglePATable(uint8_t value);
void WriteBurstPATable(uint8_t *buffer, uint8_t count); 
