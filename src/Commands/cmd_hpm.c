#include "cmd_base.h"

void cmd_hpm(int argc, char** argv)
{
    if (!argv[1] || !argv[2])
    {
        read_help(CMD_HPM_HELP);
        return;
    }

    const char* command = argv[1];
    const char* package_name = argv[2];
    const char* repo_url = argv[3];

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
