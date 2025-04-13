#ifndef __NETROOT_H__
#define __NETROOT_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if_packet.h>
#include <net/if.h> // <- important: defines struct ifreq, IFNAMSIZ, IFF_PROMISC
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/ethernet.h>

#include "../main.h"

#define IFNAMSIZ 16
#define IFF_PROMISC 0x100

struct ifreq
{
    char ifr_name[IFNAMSIZ]; // Interface name
    union
    {
        struct sockaddr ifr_addr;
        struct sockaddr_in ifr_addr_in;
        struct sockaddr_in6 ifr_addr_in6;
        struct ifreq *ifr_ifru;
        short ifr_flags;
        int ifr_metric;
        int ifr_mtu;
        struct
        {
            struct sockaddr ifr_hwaddr;
            unsigned char ifr_data[64]; // Space for any specific information
        };
    };
};

#define IFF_LOOPBACK 0x8

extern char ip[INET_ADDRSTRLEN];
extern int port;
extern int sockfd;

#endif