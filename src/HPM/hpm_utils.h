#ifndef __HPM_UTILS_H__
#define __HPM_UTILS_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../Networking/netroot.h"
#include "../Crypto/crypto.h"

#define MAX_PACKAGE_NAME_LENGTH 256
#define MAX_PACKAGE_VERSION_LENGTH 64
#define MAX_PACKAGE_DESCRIPTION_LENGTH 512

#define HEADER_SPLIT "\0\e"

typedef struct
{
    char name[MAX_PACKAGE_NAME_LENGTH];
    char version[MAX_PACKAGE_VERSION_LENGTH];
    char description[MAX_PACKAGE_DESCRIPTION_LENGTH];
    char author[MAX_PACKAGE_NAME_LENGTH];

    unsigned char* data;
    size_t data_size;

    unsigned char* hash;
    size_t hash_size;
} package_t;

package_t *package_create(const char* name, const char* version, const char* description, const char* author);
void package_destroy(package_t* package);

#endif