#ifndef __HPM_UTILS_H__
#define __HPM_UTILS_H__

#include "../root.h"
#include "../lib/macro.h"
#include "../Crypto/crypto.h"

#include <git2.h>

#define INSTALL_DB "lib/.db-install.txt"
#define INSTALL_DIR "lib/"

// Structure for holding basic package info
typedef struct
{
    char *name;
    char *version;
    char *author;

    int id;
} package_t;

git_repository *clone_repo(package_t *pkg);

const char *pkg_to_url(package_t *pkg);

package_t *init_package();

char *read_git_token();

// Callback function for Git credentials
int cred_cb(git_credential **out, const char *url, const char *username_from_url,
    unsigned int allowed_types, void *payload);

#endif