#ifndef __NETROOT_H__
#define __NETROOT_H__

// Standard C headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

// POSIX + Networking headers (ordered by dependency)
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/if_ether.h>
#include <net/ethernet.h>
#include <arpa/inet.h>

// Color macros (code-style)
#define __red printf("\033[31m")
#define __yellow printf("\033[33m")
#define __green printf("\033[32m")
#define __blue printf("\033[34m")
#define __cyan printf("\033[36m")
#define __magenta printf("\033[35m")
#define __white printf("\033[37m")
#define __reset printf("\033[0m")
#define __bold printf("\033[1m")
#define __underline printf("\033[4m")

#define __ok printf("\033[32m[  OK  ]\033[0m ")
#define __info printf("\033[34m[ INFO ]\033[0m ")
#define __warn printf("\033[33m[ WARN ]\033[0m ")
#define __fail printf("\033[31m[FAILED]\033[0m ")

// Safety checks (in case platform headers are incomplete)
#ifndef IFNAMSIZ
#define IFNAMSIZ 16
#endif

#ifndef IFF_PROMISC
#define IFF_PROMISC 0x100
#endif

#ifndef IFF_LOOPBACK
#define IFF_LOOPBACK 0x8
#endif

// Extern globals
extern char ip[INET_ADDRSTRLEN];
extern int port;
extern int sockfd;

#endif // __NETROOT_H__
