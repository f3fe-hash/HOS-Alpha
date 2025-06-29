#ifndef __INSTALL_DATABASE_H__
#define __INSTALL_DATABASE_H__

#include "../cJSON/cJSON.h"
#include "../root.h"
#include "../types.h"

typedef struct
{
    int PACKAGE_VERSION_MAJOR;
    int PACKAGE_VERSION_MINOR;
    int PACKAGE_VERSION_PATCH;
} PackageHistoryTimestamp;

cJSON* load_db();
int save_db(cJSON* root);
void add_package_to_db(cJSON* db, const char* name, const char* version, const char* url, const char* path);

#endif