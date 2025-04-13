#ifndef __PING_H__
#define __PING_H__

#include "netroot.h"

unsigned short checksum(unsigned short *buffer, int length);

int ping(int ip_address, int packet_size, int packets, int timeout);

#endif