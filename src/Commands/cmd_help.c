#include "cmd_base.h"

static struct option cmd_help_options[] =
{
    {"cmd",    required_argument,  NULL, 'c'},
    {NULL, 0, NULL, 0}
};

void cmd_help(int argc, char** argv)
{
    int usecmd = 0;
    while (true)
    {
        int option_index = 0;
        int c = getopt_long(argc, argv, "c:", cmd_help_options, &option_index);

        if (c == -1)
            break;

        switch (c)
        {
            // Help on a certain command
            case 'c':
                // Ensure that there is an argument
                if (optarg == NULL)
                {
                    printf("Missing argument for --cmd\n");
                    return;
                }

                // Assuming 'help <command>', read etc/help/<command>.txt
                char* cmd = mp_alloc(mpool_, strlen(optarg) + strlen("etc/help/.txt") + 1);
                sprintf(cmd, "etc/help/%s.txt", optarg);
                read_help(cmd);
                usecmd = 1;
                break;
        
            // ? output
            default:
                break;
        }
    }

    if (!usecmd)
        read_help(HELP_FILE);
}