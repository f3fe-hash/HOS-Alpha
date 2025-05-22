#include "main.h"

/*
 * Function: main
 * ----------------
 * The main function of HOS.
 *
 * argc: The number of command line arguments.
 * argv: The array of command line arguments.
 *
 * returns: 0 on success, non-zero on failure.
 */
int main(int argc, char **argv)
{
    __argc = argc;
    __argv = argv;

    // Process arguments
    args(argc, argv);

    // Boot HOS
    boot();

    // Main loop
    while (1)
    {
        char *input = readline(prompt_string());

        if (input && *input)
        {
            add_history(input);
            execute(input);
            free(input);
        }
        else if (!input)
            break; // Ctrl+D or error
    }

    return 0;
}

/*
 * Function: args
 * ---------------
 * Processes command line arguments for HOS.
 *
 * argc: The number of command line arguments.
 * argv: The array of command line arguments.
 *
 * returns: __null
 */
void args(int argc, char **argv)
{
    while (true)
    {
        int option_index = 0;
        int c = getopt_long(argc, argv, "hvgp:", long_options, &option_index);

        if (c == -1)
            break;

        switch (c)
        {
        case 'h':
            cmd_help();
            HOS_exit(0);
            break;
        case 'v':
            cmd_version();
            HOS_exit(0);
            break;
        case 'p':
            cmd_setport(optarg);
            break;
        case 'g':
            use_gtk = true;
            break;
        default:
            break;
        }
    }
}

/*
 * Function: boot
 * ---------------
 * Initializes the HOS environment.
 *
 * returns: __null
 */
void boot()
{
    mp_init(mpool_);
    PROCAPI_init(4);

    // Check if program can be run
    if (geteuid() != 0)
    {
        __fail;
        printf("This tool must be run as root. (sudo %s)\n", __argv[0]);
        HOS_exit(0);
    }

    // Initialize OS files / directories
    printf("Creating HOS environment...\n");
    mkdir(".HOS", 0777);
    chmod(".HOS", 0700);
    chdir(".HOS");

    mkdir("usr", 0777);
    mkdir("bin", 0777);
    mkdir("lib", 0777);
    mkdir("etc", 0777);
    mkdir("var", 0777);
    newfile(HOSTS);
    newfile(INSTALL_DB);
    chdir("..");
    move("help.txt", ".HOS/etc");
    move("textures", ".HOS/etc");
    chdir(".HOS");
    __ok;
    printf("HOS environment created successfully\n");

    // Prep libgit2
    git_libgit2_init();
    git_libgit2_opts(GIT_OPT_SET_SEARCH_PATH, "lib");

    __clear;
    fflush(stdout);
    if (use_gtk)
    {
        // Initialize/run gtk
        initgtk();
        HOS_exit(0);
    }
    else
    {
        // Print startup message if not using gtk
        startup();
        __reset;
    }
}