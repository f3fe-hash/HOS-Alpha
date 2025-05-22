#include "hpm.h"

void ensure_dir(const char* path)
{
    struct stat st = {0};
    if (stat(path, &st) == -1)
        mkdir(path, 0755);
}

int run_command(char* const argv[])
{
    pid_t pid = fork();
    if (pid == 0)
    {
        // Child process
        execvp(argv[0], argv);
        perror("execvp failed");
        exit(1);
    }
    else if (pid > 0)
    {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
        return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
    }
    else
    {
        perror("fork failed");
        return -1;
    }
}

int download_pkg(const char* pkg_name)
{
    char* repo_url = (char*) find_url(pkg_name);
    char download_path[256];
    snprintf(download_path, sizeof(download_path), "%s/%s", INSTALL_DIR, pkg_name);

    ensure_dir(INSTALL_DIR);

    printf("Cloning %s into %s...\n", repo_url, download_path);

    git_repository* repo = NULL;
    git_clone_options opts = GIT_CLONE_OPTIONS_INIT;
    int error = git_clone(&repo, repo_url, download_path, &opts);

    if (error < 0)
    {
        const git_error* e = git_error_last();
        printf("Clone failed: %s\n", e && e->message ? e->message : "Unknown error\n");
        return 1;
    }

    git_repository_free(repo); // free the repo pointer after cloning
    mp_free(mpool_, repo_url);

    __ok;
    printf("Installed '%s' to %s\n", pkg_name, download_path);
    return 0;
}

int install_pkg(const char* pkg_name)
{
    char install_path[256];
    snprintf(install_path, sizeof(install_path), "%s/%s", INSTALL_DIR, pkg_name);

    if (chdir(install_path) != 0)
    {
        printf("Failed to change directory to %s\n", install_path);
        return 1;
    }

    FILE* f = fopen(INSTALL_JSON, "r");
    if (!f)
    {
        printf("Could not open %s\n", INSTALL_JSON);
        return 1;
    }

    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    rewind(f);

    char* data = mp_alloc(mpool_, len + 1);
    fread(data, 1, len, f);
    data[len] = '\0';
    fclose(f);

    cJSON* json = cJSON_Parse(data);
    if (!json)
    {
        printf("Failed to parse JSON: %s\n", cJSON_GetErrorPtr());
        mp_free(mpool_, data);
        return 1;
    }

    const cJSON* compile_time = cJSON_GetObjectItem(json, "compile-time");

    const cJSON* name = cJSON_GetObjectItemCaseSensitive(json, "name");
    const cJSON* version = cJSON_GetObjectItemCaseSensitive(json, "version");
    const cJSON* url = cJSON_GetObjectItemCaseSensitive(json, "url");

    const cJSON* compile_dir = cJSON_GetObjectItemCaseSensitive(compile_time, "compile-dir");
    const cJSON* executable = cJSON_GetObjectItemCaseSensitive(compile_time, "executable");
    const cJSON* compile_script = cJSON_GetObjectItemCaseSensitive(compile_time, "compile-script");

    const char* build_dir = (cJSON_IsString(compile_dir)) ? compile_dir->valuestring : "build";
    const char* exe_name = (cJSON_IsString(executable)) ? executable->valuestring : pkg_name;
    const char* script = (cJSON_IsString(compile_script)) ? compile_script->valuestring : "compile.sh";

    // Run compile script
    const char* compile[] = {"bash", script, NULL};
    if (run_command((char* const*)compile) != 0)
    {
        __fail;
        printf("Compilation failed\n");
        return 1;
    }

    __ok;
    printf("Build complete\n");

    // Move the specified executable
    size_t plen = strlen(name->valuestring) + 1 + strlen(pkg_name) + 1;
    char *path = mp_alloc(mpool_, plen);
    if (path == NULL)
    {
        perror("malloc => NULL");
        exit(1);
    }
    sprintf(path, "%s/%s", build_dir, exe_name);

    move(path, "../../bin");

    // Change back to home directory (Currently in lib/<pkg>)
    chdir("../..");

    // Update install DB
    cJSON* db = load_db();
    if (db)
    {
        const char* pkg_url = (cJSON_IsString(url)) ? url->valuestring : "unknown";
        add_package_to_db(db, name->valuestring, version->valuestring, pkg_url, install_path);
        int x = save_db(db);
        if (x != 0)
        {
            __warn;
            printf("Failed to update install database %d\n", x);
        }
        cJSON_Delete(db);
    }
    else
    {
        __warn;
        printf("Failed to load install database\n");
    }

    return 0;
}

int purge_pkg(const char* pkg_name)
{
    char path[256];

    // Remove binary
    snprintf(path, sizeof(path), "bin/%s", pkg_name);
    remove(path);  // Ignore error, maybe it was never installed

    // Remove lib/<pkg> recursively
    snprintf(path, sizeof(path), "lib/%s", pkg_name);
    remove_directory_recursive(path);

    __ok;
    printf("Package %s purged\n", pkg_name);
    return 0;
}

int update_pkg(const char* pkg_name)
{
    purge_pkg(pkg_name);
    install_pkg(pkg_name);
    return 0;
}

int remove_directory_recursive(const char* path)
{
    DIR* dir = opendir(path);
    if (!dir)
    {
        perror("opendir");
        return -1;
    }

    struct dirent* entry;
    char filepath[512];

    while ((entry = readdir(dir)) != NULL)
    {
        // Skip "." and ".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        snprintf(filepath, sizeof(filepath), "%s/%s", path, entry->d_name);

        struct stat statbuf;
        if (stat(filepath, &statbuf) == 0)
        {
            if (S_ISDIR(statbuf.st_mode))
            {
                // Recursively delete subdirectory
                remove_directory_recursive(filepath);
            }
            else
            {
                // Delete file
                if (unlink(filepath) != 0)
                    perror("unlink");
            }
        }
    }

    closedir(dir);

    // Delete the (now empty) directory
    if (rmdir(path) != 0)
    {
        perror("rmdir");
        return -1;
    }

    return 0;
}

const char* find_url(const char* pkg_name)
{
    char* url = mp_alloc(mpool_, 512);
    if (!url)
        return NULL;

    snprintf(url, 512, "https://github.com/f3fe-hash/%s-package", pkg_name);
    return url;
}