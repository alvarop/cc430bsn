/** @file radio.h
*
* @brief radio functions
*
* @author Alvaro Prieto
*     derived from SLAA465 examples
*/
#ifndef _RADIO_H
#define _RADIO_H

#include "common.h"
#include "RF1A.h"
#include "hal_pmm.h"

#define PACKET_LEN (54) // PACKET_LEN <= 61
#define RSSI_IDX_OFFSET (-2) // Index of appended RSSI
#define CRC_LQI_IDX_OFFSET (-1) // Index of appended LQI, checksum
#define CRC_OK (BIT7) // CRC_OK bit
#define PATABLE_VAL (0x51) // 0 dBm output

#define RADIO_RX 0
#define RADIO_TX 1

#define RX_BUFFER_SIZE 255

// Packet type and flag definitions
// Should have some structure eventually, but assigning arbitrary values for now

#define ACK_FLAG (1 << 4)
#define REPEATER_FLAG (1 << 2)

#define POWER_PACKET (0x05)

void setup_radio( uint8_t (*)(uint8_t*, uint8_t) );
void radio_tx( uint8_t*, uint8_t );


#endif /* _RADIO_H */\

