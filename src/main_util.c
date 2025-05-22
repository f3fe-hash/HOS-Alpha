#include "main.h"

/*
 * ------------------------------------------------------------------------------------------------------------------------
 * -----------------------------------------------HOS UTILITITY FUNCTIONS--------------------------------------------------
 * ------------------------------------------------------------------------------------------------------------------------
 */

/*
 * Function: startup
 * ------------------
 * Prints the startup message for HOS.
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
 * Function: prompt_string
 * -----------------------
 * Generates the command prompt string for the HOS shell.
 *
 * returns: A string containing the prompt.
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
 * ---------------------------
 * Generates command-line autocompletion suggestions for the HOS shell.
 *
 * text: The text to autocomplete.
 * state: The current state of the autocompletion.
 *
 * returns: A string containing the next possible completion.
 *          If no more completions are available, returns NULL.
 *          The function uses static variables to maintain state between calls.
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
 * --------------------------
 * Provides command-line autocompletion for the HOS shell.
 *
 * text: The text to autocomplete.
 * start: The starting position of the text.
 *
 * returns: A list of possible completions.
 *          If no completions are found, returns NULL.
 *          If the start position is 0, it uses the command_generator function.
 */
char **hos_autocomplete(const char *text, int start)
{
    if (start == 0)
        return rl_completion_matches(text, command_generator);
    return NULL;
}

/*
 * ------------------------------------------------------------------------------------------------------------------
 * -----------------------------------------------HASHING FUNCTIONS--------------------------------------------------
 * ------------------------------------------------------------------------------------------------------------------
 */

/*
 * Function: hash_string
 * ----------------------
 * Hashes a string using the specified algorithm.
 *
 * str: The string to hash.
 * algorithm: The hashing algorithm to use (e.g., "sha1", "sha256", "sha512").
 *
 * returns: A string containing the hexadecimal representation of the hash.
 */
char *hash_string(const char *str, const char *algorithm)
{
    if (strcmp(algorithm, "sha1") == 0)
        return (char *)sha1((unsigned char *)str, strlen(str));
    else if (strcmp(algorithm, "sha256") == 0)
        return (char *)sha256((unsigned char *)str, strlen(str));
    else if (strcmp(algorithm, "sha512") == 0)
        return (char *)sha512((unsigned char *)str, strlen(str));
    else
        return NULL;
}

/*
 * Function: to_hex_string
 * ------------------------
 * Converts a binary hash to a hexadecimal string.
 *
 * hash: The binary hash to convert.
 * len: The length of the hash.
 *
 * returns: A string containing the hexadecimal representation of the hash.
 */
char *to_hex_string(const unsigned char *hash, size_t len)
{
    static char hexstr[129];
    for (size_t i = 0; i < len; ++i)
        sprintf(hexstr + (i * 2), "%02x", hash[i]);
    hexstr[len * 2] = '\0';
    return hexstr;
}

/*
 * --------------------------------------------------------------------------------------------------------------------------
 * -----------------------------------------------FILE MANAGEMENT FUNCTIONS--------------------------------------------------
 * --------------------------------------------------------------------------------------------------------------------------
 */

/*
 * Function: newfile
 * --------------------
 * Creates a new file with the specified filename.
 *
 * filename: The name of the file to create.
 *
 * returns: __null
 */
void newfile(const char *filename)
{
    FILE *file = fopen(filename, "w");
    if (file == NULL)
    {
        __fail;
        printf("Error creating file: %s\n", filename);
        return;
    }
    fclose(file);
}

/*
 * Function: move
 * ---------------
 * Moves a file to a new destination.
 *
 * file: The name of the file to move.
 * dest: The destination path.
 *
 * returns: __null
 */
void move(const char *file, const char *dest_dir)
{
    struct stat st;

    if (stat(file, &st) != 0)
    {
        __fail;
        printf("Source file does not exist: %s\n", file);
        return;
    }

    if (stat(dest_dir, &st) != 0 || !S_ISDIR(st.st_mode))
    {
        __fail;
        printf("Destination is not a valid directory: %s\n", dest_dir);
        return;
    }

    // Extract base name from file path
    char *file_copy = strdup(file);
    if (!file_copy)
    {
        fprintf(stderr, "Memory allocation failed\n");
        return;
    }
    char *base = basename(file_copy);

    size_t path_len = strlen(dest_dir) + 1 + strlen(base) + 1;
    char *dest = malloc(path_len);
    if (!dest)
    {
        fprintf(stderr, "Memory allocation failed\n");
        free(file_copy);
        return;
    }

    snprintf(dest, path_len, "%s/%s", dest_dir, base);

    if (rename(file, dest) != 0)
    {
        __fail;
        perror("Error moving file");
        printf("Failed to move %s to %s\n", file, dest);
    }
    else
    {
        __ok;
        printf("File moved successfully: %s to %s\n", file, dest);
    }

    free(dest);
    free(file_copy);
}