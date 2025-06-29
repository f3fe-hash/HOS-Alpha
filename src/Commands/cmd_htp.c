#include "cmd_base.h"

void cmd_htp(int argc, char** argv)
{
    if (argv[1] == NULL)
    {
        read_help(CMD_HTP_HELP);
        return;
    }
}
