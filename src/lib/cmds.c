#include "cmds.h"

void cmd_clear()
{
    __clear;
}

void cmd_echo(char** tokens)
{
    for (int j = 1; tokens[j] != NULL; j++)
        printf("%s ", tokens[j]);
}

void cmd_exit()
{
    git_libgit2_shutdown();
    HOS_exit(0);
}

void cmd_help()
{
    FILE* help_file = fopen(HELP_FILE, "r");
    if (help_file == NULL)
    {
        __fail;
        printf("Error opening help file\n");
        return;
    }
    char line[256];
    while (fgets(line, sizeof(line), help_file) != NULL)
        printf("%s", line);
    printf("\n");
    fclose(help_file);
}

void cmd_version()
{
    printf("HOS %s\n", HOS_VERSION);
}

/* Networking commands */

void cmd_netwatch(char** tokens)
{
    if (tokens[1] == NULL)
        printf("Usage: netwatch <interface> <max hosts>\n");
    else
    {
        const char* interface = tokens[1];
        int max_hosts = atoi(tokens[2]);

        // Ensure that max_hosts is a positive number
        if (max_hosts <= 0)
        {
            printf("Invalid max hosts: must be greater than 0\n");
            return;
        }

        netwatch_start(tokens[1], atoi(tokens[2]));
    }
}

void cmd_ping(char** tokens)
{
    if (tokens[1] == NULL)
        printf("Usage: ping <packets> <packet size> <hostname/IP> <timeout (microseconds)>\n");
    else
    {
        int packets = atoi(tokens[1]);
        int packet_size = atoi(tokens[2]);
        int timeout = atoi(tokens[4]);
        char* target = tokens[2];

        if (packets <= 0)
        {
            printf("Invalid number of packets: %s\n", tokens[1]);
            return;
        }
        if (packet_size <= 0)
        {
            printf("Invalid packet size: %s\n", tokens[2]);
            return;
        }
        if (timeout <= 2000)
            timeout = 10000;

        struct sockaddr_in dest_addr;
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(0);
        inet_pton(AF_INET, target, &dest_addr.sin_addr);

        ping(dest_addr.sin_addr.s_addr, packet_size, packets, timeout);
    }
}

void cmd_setport(char** tokens)
{
    if (tokens[1] == NULL)
        printf("Usage: setport <port>\n");
    else
    {
        port = atoi(tokens[1]);
        printf("Port set to %d\n", port);
    }

    __ok;
    printf("Port set to %d\n", port);
}

/* Crypto commands */

void cmd_hash(char** tokens)
{
    if (tokens[1] == NULL || tokens[2] == NULL)
    {
        printf("Usage: hash <string> <hash-type>\n");
        return;
    }

    const char* input = tokens[1];
    const char* algo = tokens[2];
    const unsigned char* raw_hash = NULL;
    size_t hash_len = 0;

    if (strcmp(algo, "sha1") == 0)
    {
        raw_hash = sha1((unsigned char*)input, strlen(input));
        hash_len = 20;
    }
    else if (strcmp(algo, "sha256") == 0)
    {
        raw_hash = sha256((unsigned char*)input, strlen(input));
        hash_len = 32;
    }
    else if (strcmp(algo, "sha512") == 0)
    {
        raw_hash = sha512((unsigned char*)input, strlen(input));
        hash_len = 64;
    }
    else
    {
        printf("Unsupported hash type: %s\n", algo);
        return;
    }

    if (raw_hash != NULL)
        printf("Hash (%s): %s\n", algo, to_hex_string(raw_hash, hash_len));
    else
        printf("Error hashing string: %s\n", input);
}

/* Package manager commands */
void cmd_hpm(char** tokens)
{
    if (!tokens[1] || !tokens[2])
    {
        printf("Usage: hpm <command> <package>\n");
        return;
    }

    const char* command = tokens[1];
    const char* package_name = tokens[2];
    const char* repo_url = tokens[3];

    if (strcmp(command, "download") == 0)
    {
        if (download_pkg(package_name) != 0)
            printf("Download failed for %s\n", package_name);
    }
    else if (strcmp(command, "install") == 0)
    {
        if (download_pkg(package_name) != 0)
        {
            printf("Download failed for %s\n", package_name);
            return;
        }

        if (install_pkg(package_name) != 0)
            printf("Install failed for %s\n", package_name);
    }
    else if (strcmp(command, "purge") == 0)
    {
        purge_pkg(package_name);
    }
    else if (strcmp(command, "update") == 0)
    {
        update_pkg(package_name);
    }
    else
    {
        printf("Unknown HPM command: %s\n", command);
    }
}

void cmd_run(char** tokens)
{
    if (tokens[1] == NULL)
    {
        __fail;
        printf("Usage: run <binary> [args...]\n");
        return;
    }

    char path[256];
    snprintf(path, sizeof(path), "bin/%s", tokens[1]);

    // Shift tokens[1...] to tokens[0...], dropping "run"
    char* argv[256];
    int i = 1;
    int j = 0;
    while (tokens[i] != NULL && j < 255)
    {
        argv[j++] = tokens[i++];
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

/*
                GUI
*/

void cmd_gui()
{
    initgtk();
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
        cmd_echo(tokens);

    else if (strcmp(cmd, "exit") == 0)
        cmd_exit();

    else if (strcmp(cmd, "hash") == 0)
        cmd_hash(tokens);

    else if (strcmp(cmd, "help") == 0)
        cmd_help();

    else if (strcmp(cmd, "hpm") == 0)
        cmd_hpm(tokens);

    else if (strcmp(cmd, "netwatch") == 0)
        cmd_netwatch(tokens);

    else if (strcmp(cmd, "ping") == 0)
        cmd_ping(tokens);

    else if (strcmp(cmd, "version") == 0)
        cmd_version();
    
    else if (strcmp(cmd, "gui") == 0)
        cmd_gui();
    
    else if (strcmp(cmd, "run") == 0)
        cmd_run(tokens);

    else
    {
        __fail;
        printf("Unknown Command: %s\n", cmd);
    }
}