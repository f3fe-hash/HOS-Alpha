#include "install_database.h"

cJSON* load_db()
{
    FILE* f = fopen(INSTALL_DB, "r");
    if (!f)
    {
        // File doesn't exist, create an empty root object
        return cJSON_CreateObject();
    }

    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    rewind(f);

    char* data = mp_alloc(mpool_, len + 1);
    fread(data, 1, len, f);
    data[len] = '\0';
    fclose(f);

    cJSON* root = cJSON_Parse(data);
    mp_free(mpool_, data);

    if (!root || !cJSON_IsObject(root))
    {
        // Invalid JSON or not an object, return new empty object
        if (root) cJSON_Delete(root);
        return cJSON_CreateObject();
    }

    return root;
}

int save_db(cJSON* root)
{
    char* str = cJSON_Print(root);
    if (!str) return 1;

    FILE* f = fopen(INSTALL_DB, "w");
    if (!f)
    {
        free(str);
        return 2;
    }

    fputs(str, f);
    fclose(f);
    free(str);
    return 0;
}

void add_package_to_db(cJSON* db, const char* name, const char* version, const char* url, const char* path)
{
    cJSON* pkg = cJSON_CreateObject();
    cJSON_AddStringToObject(pkg, "name", name);
    cJSON_AddStringToObject(pkg, "version", version);
    cJSON_AddStringToObject(pkg, "url", url);
    cJSON_AddStringToObject(pkg, "path", path);

    cJSON_DeleteItemFromObjectCaseSensitive(db, name);
    cJSON_AddItemToObject(db, name, pkg);
}
