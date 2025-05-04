#include "hpm.h"

void download_package(package_t *pkg)
{

    git_repository *repo = clone_repo(pkg);
    if (repo == NULL)
    {
        __fail;
        printf("Failed to clone repository: %s\n", pkg->name);
        return;
    }
    else
    {
        __ok;
        printf("Cloned repository: %s\n", pkg->name);
    }

    git_repository_free(repo);
}

void install_package(package_t *pkg)
{
    if (!pkg || !pkg->name)
    {
        __fail;
        printf("Invalid package data.\\n");
        return;
    }

    // Check if the package is already installed
    FILE *file = fopen(INSTALL_DB, "r");
    if (file == NULL)
    {
        __fail;
        printf("Failed to open installation database: %s\n", INSTALL_DB);
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), file) != NULL)
    {
        if (strstr(line, pkg->name) != NULL)
        {
            __warn;
            printf("Package %s is already installed.\n", pkg->name);
            fclose(file);
            return;
        }
    }
    fclose(file);

    // Download the package
    download_package(pkg);

    // Add the package to the installation database
    file = fopen(INSTALL_DB, "a");
    if (file == NULL)
    {
        __fail;
        printf("Failed to open install database for writing: %s\n", INSTALL_DB);
        return;
    }
    fprintf(file, "%s\n", pkg->name);
    fclose(file);

    // Build the package
    if (chdir("lib") != 0 || chdir(pkg->name) != 0)
    {
        __fail;
        printf("Failed to enter package directory: lib/%s\\n", pkg->name);
        return;
    }

    system("source install.sh");
}
