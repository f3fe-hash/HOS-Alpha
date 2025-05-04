#include "root.h"

int __argc;
char **__argv;

bool use_gtk = false;

const char *hos_commands[] =
{
    "exit", "ping", "netwatch", "hash", "help", "version",
    "echo", "setport", "clear", NULL
};

void HOS_exit(int status)
{
    // Exit the program with the given status
    exit(status);
}