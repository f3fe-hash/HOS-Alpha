#ifndef __NETWATCH_H__
#define __NETWATCH_H__

#include "../netroot.h"
#include "../ping.h"
#include <poll.h>
#include <time.h>
#include <math.h>
#include <errno.h>

#define MAX_HOSTS 256

// Data structure to hold IP address counts
struct ip_count
{
    struct in_addr ip;
    unsigned long count;
    const char *hostname; // cache hostname
};

typedef char* ip_t;

const char *timestamp();

const char *timestamp();
void handle_interrupt(int sig);
const char *resolve_hostname(struct in_addr ip);

void netwatch_start(const char* interface, int max_hosts);

#endif
