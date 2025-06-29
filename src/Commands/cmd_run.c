#include "cmd_base.h"

void cmd_run(int argc, char** argv)
{
    if (argv[1] == NULL)
    {
        read_help(CMD_RUN_HELP);
        return;
    }

    char path[256];
    snprintf(path, sizeof(path), "bin/%s", argv[1]);

    // Shift argv[1...] to argv[0...], dropping "run"
    int i = 1;
    int j = 0;
    while (argv[i] != NULL && j < 255)
    {
        argv[j++] = argv[i++];
    }
    argv[j] = NULL;

    pid_t pid = fork();

    if (pid < 0)
    {
        __fail;
        printf("Failed to fork process\n");
    }
    else if (pid == 0)
    {
        execv(path, argv);
        __fail;
        printf("Failed to execute: %s\n", path);
        HOS_exit(1);
    }
    else
    {
        int status;
        waitpid(pid, &status, 0);
    }
}
