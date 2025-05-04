/*
 * root.h: Main header file for the HOS project.
 * This file includes all necessary headers and defines constants
 * and macros used throughout the project.
 */

#ifndef __ROOT_H__
#define __ROOT_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>

#include "lib/macro.h"

// Pre-defined paths
#define HOSTS           "etc/hosts"
#define HELP_FILE       "etc/help.txt"
#define INSTALL_DB      "lib/.db-install.txt"
#define GIT_TOKEN_PATH  "var/.git_token.txt"

// Global variables
extern int __argc;
extern char **__argv;
extern bool use_gtk;
extern struct sockaddr_in server_addr;

extern const char *hos_commands[];

void HOS_exit(int status);
void HOS_mkdir(const char* path);

char* hash_string(const char* str, const char* algorithm);
char* to_hex_string(const unsigned char* hash, size_t len);

#endif