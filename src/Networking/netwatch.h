#ifndef __NETWATCH_H__
#define __NETWATCH_H__

#include "netroot.h"

static const char *timestamp();
void handle_interrupt(int sig);
const char *resolve_hostname(struct in_addr ip);

void netwatch_start(const char* interface, int max_hosts);

#endif
