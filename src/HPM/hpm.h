#ifndef __HPM_H__
#define __HPM_H__

#include <git2.h>
#include <dirent.h>
#include "../cJSON/cJSON.h"

#include "../root.h"
#include "install_database.h"

#include "../lib/ll.h"
#include "../lib/memory.h"

#define INSTALL_DIR  "lib"
#define INSTALL_JSON "hpm.json"
#define INSTALL_DB "lib/.db-install.json"

void ensure_dir(const char* path);
int download_pkg(const char* pkg_name);
int install_pkg(const char* pkg_name);
int purge_pkg(const char* pkg_name);

int remove_directory_recursive(const char* path);

const char* find_url(const char* pkg_name);

#endif