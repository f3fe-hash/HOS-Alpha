#ifndef __MAIN_H__
#define __MAIN_H__

// Pre-defined paths
#define __modules "../modules/"
#define __libs "../libs/"

// HOS information
#define HOS_VERSION "Alpha 0.0.1"
#define __AUTHOR__          "Brian Riff"
#define __RELEASE_DATE__    "April 10, 2025"

// C Standard headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <stdbool.h>

// OpenSSL headers
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/err.h>

// Networking headers
#include "Networking/netroot.h"
#include "Networking/ping.h"
#include "Networking/netwatch.h"

// HOS
#include "HOS.h"

#define __null ((void*)0)

// Color macros (code-style)
#define __red       printf("\033[31m")
#define __yellow    printf("\033[33m")
#define __green     printf("\033[32m")
#define __blue      printf("\033[34m")
#define __cyan      printf("\033[36m")
#define __magenta   printf("\033[35m")
#define __white     printf("\033[37m")
#define __reset     printf("\033[0m")
#define __bold      printf("\033[1m")
#define __underline printf("\033[4m")

#define __ok   printf("\033[32m[  OK  ]\033[0m ")
#define __info printf("\033[34m[ INFO ]\033[0m ")
#define __warn printf("\033[33m[ WARN ]\033[0m ")
#define __fail printf("\033[31m[FAILED]\033[0m ")

// Global variables (declared, not defined here)
extern int    __argc;
extern char** __argv;

extern struct sockaddr_in server_addr;

// Function declarations
int main(int argc, char** argv);
void args(int argc, char** argv);
void execute(char* input);
void load_ip_address();
void boot_internet();
void boot();

void startup();

#endif // __MAIN_H__
