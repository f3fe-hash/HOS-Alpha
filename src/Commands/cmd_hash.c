#include "cmd_base.h"

static struct option cmd_hash_options[] =
{
    {"help",    no_argument,        NULL, 'h'},
    {"algo",    required_argument,  NULL, 'a'},
    {"text",    required_argument,  NULL, 't'},
    {"file",    required_argument,  NULL, 'f'},
    {NULL, 0, NULL, 0}
};

void cmd_hash(int argc, char** argv)
{
    char* input = NULL;
    char* algo = NULL;
    const unsigned char* raw_hash = NULL;
    size_t hash_len = 0;
    bool using_text_flag = false;

    while (true)
    {
        int option_index = 0;
        int c = getopt_long(argc, argv, "ha:t:f:", cmd_hash_options, &option_index);

        if (c == -1)
            break;

        switch (c)
        {
            // Read out the help file
            case 'h':
                read_help(CMD_HASH_HELP);
                return;
        
            // Set the algorithm
            case 'a':
                algo = optarg;
                break;

            // Hash the option argument
            case 't':
                input = optarg;
                using_text_flag = true;
                break;

            // Read input from a file
            case 'f':
                FILE* fp = fopen(optarg, "rb");
                if (!fp) { printf("Error opening file: %s", optarg); return; } // Make sure file exists
                fseek(fp, 0, SEEK_END);
                long len = ftell(fp);
                rewind(fp);

                input = mp_alloc(mpool_, len + 1);
                fread(input, 1, len, fp );
                input[len] = '\0';
                fclose(fp);

                break;
        
            // ? output
            default:
                break;
        }
    }
    
    if (input == NULL)
    {
        read_help(CMD_HASH_HELP);
        return;
    }

    if (using_text_flag)
    {
        size_t total_len = strlen(input);
        for (int i = optind; i < argc; ++i)
            total_len += strlen(argv[i]) + 1;  // +1 for space

        char* combined = mp_alloc(mpool_, total_len + 1);
        if (!combined)
        {
            printf("Memory allocation failed\n");
            return;
        }

        strcpy(combined, input);
        for (int i = optind; i < argc; ++i)
        {
            strcat(combined, " ");
            strcat(combined, argv[i]);
        }

        input = combined;
    }

    if (algo == NULL)
        algo = "sha256";

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
