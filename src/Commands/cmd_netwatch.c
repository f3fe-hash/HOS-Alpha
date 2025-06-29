#include "cmd_base.h"

void cmd_netwatch(int argc, char** argv)
{
    if (argv[1] == NULL)
    {
        read_help(CMD_NETWATCH_HELP);
        return;
    }

    const char* interface = argv[1];
    int max_hosts = argv[2] ? atoi(argv[2]) : 15;

    if (max_hosts <= 0)
    {
        printf("Invalid max hosts: must be greater than 0\n");
        return;
    }

    netwatch_start(interface, max_hosts);
}
