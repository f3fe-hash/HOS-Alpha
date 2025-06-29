#include "netutils.h"

void load_ip_address()
{
    struct ifaddrs* pIfaddr;
    struct ifaddrs* pIfa;

    if (getifaddrs(&pIfaddr) == -1)
    {
        perror("getifaddrs");
        HOS_exit(EXIT_FAILURE);
    }

    for (pIfa = pIfaddr; pIfa != NULL; pIfa = pIfa->ifa_next)
    {
        if (pIfa->ifa_addr == NULL)
            continue;

        if (pIfa->ifa_addr->sa_family == AF_INET &&
            !(pIfa->ifa_flags & IFF_LOOPBACK))
        {
            void* pAddr = &((struct sockaddr_in *)pIfa->ifa_addr)->sin_addr;
            inet_ntop(AF_INET, pAddr, ip, sizeof(ip));

            if (strncmp(ip, "127.", 4) != 0)
                break;
        }
    }

    freeifaddrs(pIfaddr);
}
