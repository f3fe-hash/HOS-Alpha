#ifndef __PING_H__
#define __PING_H__

#include "netroot.h"

unsigned short calculate_checksum(unsigned short *buffer, int length);

int ping(in_addr_t ip_address, int packet_size, int packets, int timeout);

#endif