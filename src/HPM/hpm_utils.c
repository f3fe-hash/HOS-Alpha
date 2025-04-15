#include "hpm_utils.h"

package_t *package_create(const char *name, const char *version, const char *description, const char *author)
{
    package_t *package = (package_t *)malloc(sizeof(package_t));
    if (package == NULL)
    {
        fprintf(stderr, "Error allocating memory for package\n");
        return NULL;
    }

    strncpy(package->name, name, MAX_PACKAGE_NAME_LENGTH);
    strncpy(package->version, version, MAX_PACKAGE_VERSION_LENGTH);
    strncpy(package->description, description, MAX_PACKAGE_DESCRIPTION_LENGTH);
    strncpy(package->author, author, MAX_PACKAGE_NAME_LENGTH);

    package->data = NULL;
    package->data_size = 0;

    package->hash = NULL;
    package->hash_size = 0;

    return package;
}

void package_destroy(package_t *package)
{
    if (package != NULL)
    {
        // Remove package from install database
        FILE *db = fopen("packages/.db-install.txt", "r");
        FILE *tmp = fopen("packages/.db-install.tmp", "w");
        if (db != NULL && tmp != NULL)
        {
            char line[1024];
            while (fgets(line, sizeof(line), db) != NULL)
            {
                if (strstr(line, package->name) == NULL)
                    fputs(line, tmp);
            }
            fclose(db);
            fclose(tmp);

            // Replace the original file with the temp file
            if (remove("packages/.db-install.txt") != 0)
            {
                __fail;
                printf("Error deleting original database file");
            }
            else if (rename("packages/.db-install.tmp", "packages/.db-install.txt") != 0)
            {
                __fail;
                printf("Error renaming temporary database file");
            }
        }
        else
        {
            fprintf(stderr, "Error opening install database\n");
            if (db) fclose(db);
            if (tmp) fclose(tmp);
        }

        // Free memory
        free(package->data);
        free(package->hash);
        free(package);
    }
}