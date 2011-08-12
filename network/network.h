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
  uint8_t destination;
  uint8_t source;
  
  // The first four bits of the packet type are used as flags, while the last
  // four are the actual packet type
  uint8_t type;
} packet_header_t;

#define PACKET_START (0x1)
#define PACKET_POLL (0x2)
#define PACKET_SYNC (0x3)

// The ACK flag is used to acknowledge a packet
#define FLAG_ACK ( 1 < 4 )

// The BURST flag is used to identify a multi-packet transmission
#define FLAG_BURST ( 1 < 5 )


#endif /* _NETWORK_H */\

