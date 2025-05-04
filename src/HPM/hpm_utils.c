#include "hpm_utils.h"

git_repository *clone_repo(package_t *pkg)
{
    const char *repo_url = pkg_to_url(pkg);
    if (!repo_url)
    {
        printf("Invalid repository URL: %s\n", repo_url);
        return NULL;
    }

    git_repository *repo = NULL;
    git_clone_options clone_opts = GIT_CLONE_OPTIONS_INIT;
    clone_opts.fetch_opts.callbacks.credentials = cred_cb;

    char path[256];
    snprintf(path, sizeof(path), "lib/%s-%s", pkg->name, pkg->version);
    mkdir(path, 0777);

    if (git_clone(&repo, repo_url, path, &clone_opts) != 0)

    {
        const git_error *err = git_error_last();
        printf("Error cloning repository: %s\n", err->message);
        return NULL;
    }

    return repo;
}

const char *pkg_to_url(package_t *pkg)
{
    static char url[256];
    sprintf(url, "https://www.github.com/f3fe-hash/HOS-Packages-%s-%s.git", pkg->name, pkg->version);
    return url;
}

package_t *init_package()
{
    package_t *pkg = (package_t *)malloc(sizeof(package_t));
    if (pkg == NULL)
    {
        printf("Error allocating memory for package\n");
        return NULL;
    }

    pkg->name = NULL;
    pkg->version = NULL;
    pkg->author = NULL;
    pkg->id = 0;

    return pkg;
}

int cred_cb(git_credential **out, const char *url, const char *username_from_url,
            unsigned int allowed_types, void *payload)
{

    char *token = read_git_token();
    if (!token)
    {
        __fail;
        printf("Could not read GitHub token\n");
        return -1;
    }

    int result = git_credential_userpass_plaintext_new(out, "x-access-token", token);
    free(token);
    return result;
}

char *read_git_token()
{
    const char *path = getenv("HOME");
    if (!path)
        return NULL;

    char full_path[512];
    snprintf(full_path, sizeof(full_path), "%s/%s", path, GIT_TOKEN_PATH);

    FILE *fp = fopen(full_path, "r");
    if (!fp)
        return NULL;

    char *token = malloc(256); // Allocate memory for token
    if (!token)
    {
        fclose(fp);
        return NULL;
    }

    if (!fgets(token, 256, fp))
    {
        free(token);
        fclose(fp);
        return NULL;
    }

    // Remove newline
    token[strcspn(token, "\n")] = '\0';
    fclose(fp);
    return token;
}