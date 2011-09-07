/** @file network.h
*
* @brief Network settings and packet types/structures
*
* @author Alvaro Prieto
*/
#ifndef _NETWORK_H
#define _NETWORK_H

/**
 * Packet header structure. The packet length byte is omitted, since the receive
 * function strips it away. Also, the cc2500_tx_packet function inserts it 
 * automatically.
 */
typedef struct
{
  uint8_t destination;  // Packet destination
  uint8_t source;       // Packet source
  
  // The first four bits of the packet type are used as flags, while the last
  // four are the actual packet type
  uint8_t type;
  uint8_t origin;       // Origin of packet (Used when relaying)
} packet_header_t;

#define MAX_DEVICES (3)

#define BROADCAST_ADDRESS (0x00)

#define RADIO_BUFFER_SIZE (64)

#define TYPE_MASK (0x0F)
#define FLAG_MASK (0xF0)

#define PACKET_START (0x1)
#define PACKET_POLL (0x2)
#define PACKET_SYNC (0x3)

// The ACK flag is used to acknowledge a packet
#define FLAG_ACK ( 0x10 )

// The BURST flag is used to identify a multi-packet transmission
#define FLAG_BURST ( 0x20 )

// Makes the cycle x times longer
#define TIME_SCALE_FACTOR (2)

// Determines the number of timer overflows required to make a 'cycle'
#define TIMER_CYCLES (18 * TIME_SCALE_FACTOR)

// Time to transmit RSSI table
#define TIME_TX_RSSI ( TIMER_CYCLES - ( MAX_DEVICES * TIME_SCALE_FACTOR ) - 2 * TIME_SCALE_FACTOR )

// Since RSSI is in 2's complement form, -128 is the smallest possible value
// 0x80 is the representation for -128. This number will be used as an RSSI when
// no message is received
#define MIN_RSSI (0x80)

// While the DEVICE_ADDRESS of the AP is MAX_DEVICES + 1, it's RSSIs will be
// stored in the first row of the database (index = 0). This is to avoid wasting
// space, since no device has address 0.
#define AP_INDEX (0)

// Used in the device_table array to indicate that the device is active
#define DEVICE_ACTIVE         ( 1 << 0 )

// Used in the device_table array to indicate that a poll needs to be sent
#define DEVICE_POLL_SCHEDULED ( 1 << 1 )

#endif /* _NETWORK_H */\

