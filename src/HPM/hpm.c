#include "hpm.h"
#include "hpm_utils.h"

/*
 * Function: get_package
 * ---------------------
 * Retrieves a package from the server.
 *
 * name: The name of the package.
 * version: The version of the package.
 * source: The source of the package (server ip).
 *
 * returns: A pointer to the package_t structure containing the package data.
 *          NULL if an error occurs.
 */
package_t *get_package(const char *name, const char *version, const char *source)
{
    __send_package_request(name, version, source);

    unsigned char *buf;
    read(sockfd, buf, sizeof(buf));

    if (buf == NULL)
    {
        __fail;
        printf("Error reading package data\n");
        return NULL;
    }
    else
    {
        __ok;
        printf("Package data read successfully\n");
    }

    // Check for exact same installation
    FILE *install_db = fopen("packages/.db-install.txt", "r");
    if (install_db == NULL)
    {
        __fail;
        printf("Error opening install database\n");
        return NULL;
    }
    else
    {
        __ok;
        printf("Install database opened successfully\n");
    }

    char line[1024];
    while (fgets(line, sizeof(line), install_db))
    {
        if (strstr(line, name) && strstr(line, version))
        {
            __ok;
            printf("Package already installed\n");
            return NULL;
        }
    }
    fclose(install_db);

    // Parse the package data & install the package
    package_t *package = __parse_package(buf, sizeof(buf) / sizeof(unsigned char));
    if (!package)
        return NULL;
    __install_package(package);

    return package;
}

/*
 * Function: __parse_package
 * -------------------------
 * Parses the package data from the buffer.
 *
 * buffer: The buffer containing the package data.
 * buffer_size: The size of the buffer.
 *
 * returns: A pointer to the package_t structure containing the parsed package data.
 *          NULL if an error occurs.
 */
package_t *__parse_package(const char *buffer, size_t buffer_size)
{
    package_t *package = (package_t *)malloc(sizeof(package_t));
    if (package == NULL)
    {
        __fail;
        printf("Error allocating memory for package\n");
        return NULL;
    }
    else
    {
        __ok;
        printf("Memory allocated for package successfully\n");
    }

    unsigned char *buf = (unsigned char *)malloc(buffer_size);
    size_t data_size;
    size_t hash_size;
    int j = 0;

    for (int i = 0; i < sizeof(buf); i++)
    {
        if (buf[i] == HEADER_SPLIT[0] && buf[i + 1] == HEADER_SPLIT[1])
        {
            j++;
            i++;
            buf[i] = '\0';
            switch (j)
            {
            case 0:
                strncpy(package->name, (char *)buf, MAX_PACKAGE_NAME_LENGTH);
                break;
            case 1:
                strncpy(package->version, (char *)buf, MAX_PACKAGE_VERSION_LENGTH);
                break;
            case 2:
                strncpy(package->description, (char *)buf, MAX_PACKAGE_DESCRIPTION_LENGTH);
                break;
            case 3:
                strncpy(package->author, (char *)buf, MAX_PACKAGE_NAME_LENGTH);
                break;
            case 4:
                package->data_size = sizeof(buf) - i - 1;
                package->data = (unsigned char *)malloc(package->data_size);
                strncpy(package->data, (char *)buf, package->data_size);
                break;
            case 5:
                package->hash_size = sizeof(buf) - i - 1;
                package->hash = (unsigned char *)malloc(package->hash_size);
                strncpy(package->hash, (char *)buf, package->hash_size);
                break;
            default:
                __fail;
                printf("Error parsing package data\n");
                free(package);
                return NULL;
            }
        }
        else
            buf[i] = buf[i];
    }

    free(buf);

    return package;
}

/*
 * Function: __install_package
 * ----------------------------
 * Installs the package by verifying its hash and writing to the install database.
 *
 * package: The package to install.
 *
 * returns: A pointer to the installed package_t structure.
 *          NULL if an error occurs.
 */
package_t *__install_package(package_t *package)
{
    // Verify the package hash
    unsigned char *computed_hash = sha256(package->data, package->data_size);
    if (memcmp(computed_hash, package->hash, package->hash_size) != 0)
    {
        __fail;
        printf("Package hash verification failed\n");
        free(package->data);
        free(package->hash);
        free(package);
        return NULL;
    }
    else
    {
        __ok;
        printf("Package hash verified successfully\n");
    }

    // Write to the install database
    FILE *install_db = fopen("packages/.db-install.txt", "a");
    if (install_db == NULL)
    {
        __fail;
        printf("Error opening install database\n");
        free(package);
        return NULL;
    }
    else
    {
        __ok;
        printf("Install database opened successfully\n");
    }
    fprintf(install_db, "%s%s%s%s%s%s%s", package->name, HEADER_SPLIT, package->version, HEADER_SPLIT, package->description, HEADER_SPLIT, package->author);
    fclose(install_db);
    if (ferror(install_db))
    {
        __fail;
        printf("Error writing to install database\n");
        free(package->data);
        free(package->hash);
        free(package);
        return NULL;
    }
    else
    {
        __ok;
        printf("Install database written successfully\n");
    }

    // Add binary
    char* pack;
    strncpy(pack, package->name, sizeof(package->name));
    strncpy(pack, package->version, sizeof(package->version));
    chdir("packages");
    mkdir(pack, 0777);

    return package;
}

/*
 * Function: __send_package_request
 * ---------------------------------
 * Sends a package request to the server.
 *
 * name: The name of the package.
 * version: The version of the package.
 * source: The source of the package (server ip).
 */
void __send_package_request(const char *name, const char *version, const char *source)
{
    // Set up the server address
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(80);
    inet_pton(AF_INET, source, &server_addr.sin_addr);

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        __fail;
        printf("Error connecting to server\n");
        close(sockfd);
        return;
    }
    else
    {
        __ok;
        printf("Connected to server successfully\n");
    }

    // Send the package request
    char request[1024];
    snprintf(request, sizeof(request), "GET /packages/%s/%s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", name, version, source);
    send(sockfd, request, strlen(request), 0);
}