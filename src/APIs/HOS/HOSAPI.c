#include "HOSAPI.h"

HOSAPI* HOSAPI_init()
{
    HOSAPI* api = mp_alloc(mpool_, sizeof(HOSAPI));
    api->mpool_ = mpool_;
    return api;
}

unsigned char* HOSAPI_denc(HOSAPI* api, unsigned char* data, size_t len, AESkey_t key, int* out_len)
{
    return aes_encrypt(data, len, key, sizeof(key.key), out_len);
}

unsigned char* HOSAPI_ddec(HOSAPI* api, unsigned char* data, size_t len, AESkey_t key, int* out_len)
{
    return aes_decrypt(data, len, key, sizeof(key.key), out_len);
}

unsigned char* HOSAPI_fenc(HOSAPI* api, const char* file, size_t fsize, AESkey_t key, int* out_len)
{
    unsigned char* data = mp_alloc(api->mpool_, fsize);

    FILE* pFile = fopen(file, "rb");
    if (!pFile) return NULL;
    fread(data, 1, fsize, pFile);
    fclose(pFile);

    unsigned char* enc = HOSAPI_denc(api, data, fsize, key, out_len);
    mp_free(api->mpool_, data);
    return enc;
}

unsigned char* HOSAPI_fdec(HOSAPI* api, const char* file, size_t fsize, AESkey_t key, int* out_len)
{
    unsigned char* data = mp_alloc(api->mpool_, fsize);

    FILE* pFile = fopen(file, "rb");
    if (!pFile) return NULL;
    fread(data, 1, fsize, pFile);
    fclose(pFile);

    unsigned char* dec = HOSAPI_ddec(api, data, fsize, key, out_len);
    mp_free(api->mpool_, data);
    return dec;
}
