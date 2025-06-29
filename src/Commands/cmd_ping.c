#include "cmd_base.h"

void cmd_ping(int argc, char** argv)
{
    if (argv[1] == NULL)
    {
        read_help(CMD_PING_HELP);
        return;
    }

    char* target = argv[1];
    int packets = argv[2] ? atoi(argv[2]) : 16;
    int packet_size = argv[3] ? atoi(argv[3]) : 64;
    int timeout = argv[4] ? atoi(argv[4]) : 1000;

    if (packets <= 0)
    {
        printf("Invalid number of packets: %d\n", packets);
        return;
    }
    if (packet_size <= 0)
    {
        printf("Invalid packet size: %d\n", packet_size);
        return;
    }

    struct sockaddr_in dest_addr;
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(0);
    if (inet_pton(AF_INET, target, &dest_addr.sin_addr) != 1)
    {
        printf("Invalid IP address: %s\n", target);
        return;
    }

    ping(dest_addr.sin_addr.s_addr, packet_size, packets, timeout);
}
