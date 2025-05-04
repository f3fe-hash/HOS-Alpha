#include "netutils.h"

void load_ip_address()
{
    struct ifaddrs *ifaddr, *ifa;

    if (getifaddrs(&ifaddr) == -1)
    {
        perror("getifaddrs");
        HOS_exit(EXIT_FAILURE);
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == NULL)
            continue;

        if (ifa->ifa_addr->sa_family == AF_INET &&
            !(ifa->ifa_flags & IFF_LOOPBACK))
        {
            void *addr_ptr = &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
            inet_ntop(AF_INET, addr_ptr, ip, sizeof(ip));

            if (strncmp(ip, "127.", 4) != 0)
                break;
        }
    }

    freeifaddrs(ifaddr);
}