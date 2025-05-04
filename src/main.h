/*
 * main.h: Main header file for the HOS project.
 * This file includes all necessary headers and defines constants
 * and macros used throughout the project.
 */

#ifndef __MAIN_H__
#define __MAIN_H__

// C Standard headers
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <getopt.h>
#include <sys/stat.h>

// POSIX headers
#include <readline/readline.h>
#include <readline/history.h>

// OpenSSL headers
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/err.h>

// Security headers
#include "Crypto/crypto.h"

// Networking headers
#include "Networking/netroot.h"
#include "Networking/netutils.h"
#include "Networking/ping.h"
#include "Networking/NetWatch/netwatch.h"

// HPM headers
#include "HPM/hpm.h"

// GTK GUI
#include "lib/gtk/gtk_main.h"

// Commands
#include "lib/cmds.h"

// Etc
#include "lib/macro.h"
#include "lib/HOS.h"

static struct option long_options[] =
{
    {"help",    no_argument,       NULL, 'h'},
    {"version", no_argument,       NULL, 'v'},
    {"gui",     no_argument,       NULL, 'g'},
    {"port",    required_argument, NULL, 'p'},
    {NULL, 0, NULL, 0}
};

// Function declarations
int main(int argc, char **argv);
void args(int argc, char **argv);
void execute(char *input);
void boot_internet();
void boot();
void startup();

void newfile(const char *filename);
void move(const char *file, const char *dest_dir);

char *prompt_string();
char *hash_string(const char *str, const char *algorithm);
char *to_hex_string(const unsigned char *hash, size_t len);

#endif // __MAIN_H__
