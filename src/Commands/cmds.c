#include "cmd_base.h"

void read_help(const char* file)
{
    char* buff = mp_alloc(mpool_, 16384);

    FILE* fp = fopen(file, "r");
    if (fp == NULL)
    {
        __fail;
        printf("Cannot open help file\n");
        return;
    }
    fread(buff, 1, 16384, fp);
    fclose(fp);

    printf("%s\n", buff);

    mp_free(mpool_, buff);
}

/*
 * Function: execute
 * ------------------
 * Executes a command in the HOS shell.
 *
 * input: The command to execute.
 *
 * returns: __null
 */
void execute(char* input)
{
    size_t len = strlen(input);
    if (len > 0 && input[len - 1] == '\n')
        input[len - 1] = '\0';

    char* tokens[256];
    int i = 0;

    char* token = strtok(input, " ");
    while (token != NULL && i < 255)
    {
        tokens[i++] = token;
        token = strtok(NULL, " ");
    }
    tokens[i] = NULL;

    if (i == 0)
        return;

    char* cmd = tokens[0];

    if (strcmp(cmd, "clear") == 0)
        cmd_clear();

    else if (strcmp(cmd, "echo") == 0)
        cmd_echo(i, tokens);

    else if (strcmp(cmd, "exit") == 0)
        cmd_exit();

    else if (strcmp(cmd, "hash") == 0)
        cmd_hash(i, tokens);

    else if (strcmp(cmd, "help") == 0)
        cmd_help(i, tokens);

    else if (strcmp(cmd, "hpm") == 0)
        cmd_hpm(i, tokens);

    else if (strcmp(cmd, "netwatch") == 0)
        cmd_netwatch(i, tokens);

    else if (strcmp(cmd, "ping") == 0)
        cmd_ping(i, tokens);

    else if (strcmp(cmd, "version") == 0)
        cmd_version();
    
    else if (strcmp(cmd, "gui") == 0)
        cmd_gui();
    
    else if (strcmp(cmd, "run") == 0)
        cmd_run(i, tokens);

    else
    {
        __fail;
        printf("Unknown Command: %s\n", cmd);
    }
}
