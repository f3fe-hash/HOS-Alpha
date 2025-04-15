#include "main.h"

int __argc;
char **__argv;

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
    // Process command line arguments
    args(argc, argv);

    // Check if program can be run
    if (geteuid() != 0)
    {
        __fail;
        printf("This tool must be run as root. (sudo %s)\n", argv[0]);
        return 1;
    }

    boot();

    // Print startup message
    clear;
    startup();

    // Main loop
    while (1)
    {
        char *input = readline(prompt_string()); // Function we'll define

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
 * ----------------
 * Processes command line arguments.
 *
 * argc: The number of command line arguments.
 * argv: The array of command line arguments.
 *
 * returns: __null
 */
void args(int argc, char **argv)
{
    __argc = argc;
    __argv = argv;

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
        {
            printf("Usage: %s [options]\n", argv[0]);
            printf("Options:\n");
            printf("  -h, --help       Show this help message\n");
            printf("  -p, --port       Set the port number\n");
            exit(0);
        }
        else if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--port") == 0)
        {
            if (i + 1 < argc)
                port = atoi(argv[++i]);
            else
            {
                fprintf(stderr, "Error: Port number not specified\n");
                exit(1);
            }
        }
        else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0)
        {
            printf("HOS version %s\n", HOS_VERSION);
            exit(0);
        }
        else
        {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            exit(1);
        }
    }
}

/*
 * Function: execute
 * ----------------
 * Executes the given command.
 *
 * input: The command to execute.
 *
 * returns: __null
 */
void execute(char *input)
{
    // Remove trailing newline
    size_t len = strlen(input);
    if (len > 0 && input[len - 1] == '\n')
        input[len - 1] = '\0';

    // Tokenize input
    char *tokens[256];
    int i = 0;

    char *token = strtok(input, " ");
    while (token != NULL && i < 255)
    {
        tokens[i++] = token;
        token = strtok(NULL, " ");
    }
    tokens[i] = NULL; // NULL-terminate the array

    if (i == 0)
        return; // No command entered

    char *cmd = tokens[0];

    // === Built-in Commands ===
    // Exit the OS
    if (strcmp(cmd, "exit") == 0)
        exit(0);

    // Ping a host
    else if (strcmp(cmd, "ping") == 0)
    {
        if (tokens[1] == NULL)
            printf("Usage: ping <packets> <packet size> <hostname/IP> <timeout (microseconds)>\n");
        else
        {
            int packets = atoi(tokens[1]);
            int packet_size = atoi(tokens[2]);
            int timeout = atoi(tokens[4]);
            char *target = tokens[2];

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
                timeout = 10000; // Default to 10ms if less than 2ms

            struct sockaddr_in dest_addr;
            dest_addr.sin_family = AF_INET;
            dest_addr.sin_port = htons(0);
            inet_pton(AF_INET, target, &dest_addr.sin_addr);

            ping(dest_addr.sin_addr.s_addr, packet_size, packets, timeout);
        }
    }

    // Watch a specific network interface
    else if (strcmp(cmd, "netwatch") == 0)
    {
        if (tokens[1] == NULL)
            printf("Usage: netwatch <interface> <max hosts>\n");
        else
        {
            char *interface = tokens[1];
            netwatch_start(interface, atoi(tokens[2]));
        }
    }

    // Hash command line arguments
    else if (strcmp(cmd, "hash") == 0)
    {
        if (tokens[1] == NULL || tokens[2] == NULL)
        {
            printf("Usage: hash <string> <hash-type>\n");
            return;
        }

        const char *input = tokens[1];
        const char *algo = tokens[2];

        const unsigned char *raw_hash = NULL;
        size_t hash_len = 0;

        if (strcmp(algo, "sha1") == 0)
        {
            raw_hash = sha1((byte *)input, strlen(input));
            hash_len = 20;
        }
        else if (strcmp(algo, "sha256") == 0)
        {
            raw_hash = sha256((byte *)input, strlen(input));
            hash_len = 32;
        }
        else if (strcmp(algo, "sha512") == 0)
        {
            raw_hash = sha512((byte *)input, strlen(input));
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

    // Open help file and read it to the console
    else if (strcmp(cmd, "help") == 0)
    {
        FILE *help_file = fopen("help.txt", "r");
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

    // Print current OS version
    else if (strcmp(cmd, "version") == 0)
        printf("HOS %s\n", HOS_VERSION);

    // Echo all arguments to the console
    else if (strcmp(cmd, "echo") == 0)
    {
        for (int j = 1; tokens[j] != NULL; j++)
            printf("%s ", tokens[j]);
        printf("\n");
    }

    // Set curreent port
    else if (strcmp(cmd, "setport") == 0)
    {
        if (tokens[1] == NULL)
            printf("Usage: setport <port>\n");
        else
        {
            port = atoi(tokens[1]);
            printf("Port set to %d\n", port);
        }
    }

    // Clear the screen
    else if (strcmp(cmd, "clear") == 0)
        clear;

    // Command not found
    else
    {
        __fail;
        printf("Unknown Command: %s\n", cmd);
    }
}

/*
 * Function: load_ip_address
 * --------------------------
 * loads current ip address
 *
 * returns: __null
 */
void load_ip_address()
{
    struct ifaddrs *ifaddr, *ifa;

    if (getifaddrs(&ifaddr) == -1)
    {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == NULL)
            continue;

        // Check if the address family is IPv4 and is not a loopback address
        if (ifa->ifa_addr->sa_family == AF_INET &&
            !(ifa->ifa_flags & IFF_LOOPBACK)) // Skip loopback interfaces
        {
            void *addr_ptr = &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
            inet_ntop(AF_INET, addr_ptr, ip, sizeof(ip));

            // If the IP address starts with 127, skip it (loopback addresses)
            if (strncmp(ip, "127.", 4) != 0)
                break;
        }
    }

    freeifaddrs(ifaddr);
}

/*
 * Function: boot_internet
 * ------------------------
 * Initializes the internet connection.
 *
 * returns: __null
 */
void boot_internet()
{
    // Load IP addresses
    load_ip_address();

    // Initilize socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        __fail;
        printf("Error opening socket\n");
    }
    else
    {
        __ok;
        printf("Socket opened successfully\n");
    }

    // Set socket options
    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        __fail;
        printf("Error setting socket options\n");
    }
    else
    {
        __ok;
        printf("Socket options set successfully\n");
    }
}
/*
 * Function: boot
 * ---------------
 * Initializes the system.
 *
 * returns: __null
 */
void boot()
{

    boot_internet();
}

/*
 * Function: startup
 * ------------------
 * Prints startup message
 *
 * returns: __null
 */
void startup()
{
    printf("\033[31m");
    printf("------------------------------------------------------------------------------------------------\n");
    printf("\033[34m");
    printf("H   H   OOO    SSS    |    BBB   RRRR   IIIII  AAAAA   N   N      RRRR   IIIII  FFFFF  FFFFF\n");
    printf("H   H  O   O  S       |    B  B  R   R    I    A   A   NN  N      R   R    I    F      F    \n");
    printf("HHHHH  O   O   SSS    |    BBB   RRRR     I    AAAAA   N N N      RRRR     I    FFF    FFF  \n");
    printf("H   H  O   O      S   |    B  B  R  R     I    A   A   N  NN      R  R     I    F      F    \n");
    printf("H   H   OOO    SSS    |    BBB   R   R  IIIII  A   A   N   N      R   R  IIIII  F      F    \n");
    printf("\033[0m\033[31m");
    printf("------------------------------------------------------------------------------------------------\n");
    printf("\033[0m\033[35m");
    printf("\t\t\tHOS %s (%s)\n", HOS_VERSION, __RELEASE_DATE__);
    printf("\t\t\t\t%s\n", __AUTHOR__);
    printf("\033[31m");
    printf("------------------------------------------------------------------------------------------------\n");
    printf("\033[0m");
}

/*
 * Function: hash_string
 * ----------------------
 * Hashes a string using the specified algorithm.
 *
 * str: The string to hash.
 * algorithm: The hashing algorithm to use.
 *
 * returns: The hashed string.
 */
char *hash_string(const char *str, const char *algorithm)
{
    if (strcmp(algorithm, "sha1") == 0)
        return (char *)sha1((byte *)str, strlen(str));
    else if (strcmp(algorithm, "sha256") == 0)
        return (char *)sha256((byte *)str, strlen(str));
    else if (strcmp(algorithm, "sha512") == 0)
        return (char *)sha512((byte *)str, strlen(str));
    else
        return NULL;
}

/*
 * Function: to_hex_string
 * -------------------------
 * Converts a byte array to a hexadecimal string.
 *
 * hash: The byte array to convert.
 * len: The length of the byte array.
 *
 * returns: The hexadecimal string.
 */
char *to_hex_string(const unsigned char *hash, size_t len)
{
    static char hexstr[129]; // 64 bytes max * 2 + 1 = 129 (for SHA-512)
    for (size_t i = 0; i < len; ++i)
        sprintf(hexstr + (i * 2), "%02x", hash[i]);
    hexstr[len * 2] = '\0';
    return hexstr;
}

/*
 * Function: prompt_string
 * ------------------------
 * Returns the prompt string.
 *
 * returns: The prompt string.
 */
char *prompt_string()
{
    static char prompt[256];
    if (port > 0)
        snprintf(prompt, sizeof(prompt), "\033[36mHOS\033[34m@\033[32m%s:%d\033[34m$ \033[0m", ip, port);
    else
        snprintf(prompt, sizeof(prompt), "\033[36mHOS\033[34m@\033[32m%s:-\033[31m$ \033[0m", ip);
    return prompt;
}

/*
 * Function: command_generator
 * -----------------------------
 * Generates command completions for readline.
 *
 * text: The text to complete.
 * state: The state of the completion.
 *
 * returns: The next completion string.
 */
char *command_generator(const char *text, int state)
{
    static int list_index, len;
    const char *name;

    if (!state)
    {
        list_index = 0;
        len = strlen(text);
    }

    while ((name = hos_commands[list_index++]))
    {
        if (strncmp(name, text, len) == 0)
            return strdup(name);
    }

    return NULL;
}

/*
 * Function: hos_autocomplete
 * ---------------------------
 * Autocompletes commands for readline.
 *
 * text: The text to complete.
 * start: The start position of the text.
 * end: The end position of the text.
 *
 * returns: The list of completions.
 */
char **hos_autocomplete(const char *text, int start)
{
    // Only autocomplete first word
    if (start == 0)
        return rl_completion_matches(text, command_generator);
    return NULL;
}