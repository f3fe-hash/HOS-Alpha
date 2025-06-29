#include "cmd_base.h"

void cmd_clear()
{
    __clear;
    startup();
}

void cmd_echo(int argc, char** argv)
{
    for (int j = 1; argv[j] != NULL; j++)
        printf("%s ", argv[j]);
}

void cmd_exit()
{
    git_libgit2_shutdown();
    HOS_exit(0);
}

void cmd_version()
{
    printf("HOS %d.%d.%d\n", HOS_VERSION_MAJOR, HOS_VERSION_MINOR, HOS_VERSION_PATCH);
}

void cmd_setport(int argc, char** argv)
{
    if (argv[1] == NULL)
        printf("Usage: setport <port>\n");
    else
    {
        port = atoi(argv[1]);
        printf("Port set to %d\n", port);
    }

    __ok;
    printf("Port set to %d\n", port);
}

void cmd_gui()
{
    initgtk();
}
