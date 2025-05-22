#ifndef __HOSAPI_H__
#define __HOSAPI_H__

#include "../../Crypto/crypto.h"
#include "../../lib/memory.h"
#include "../Process/proc_api.h"

typedef struct
{
    MemoryPool* mpool_;
} HOSAPI;

HOSAPI* HOSAPI_init();

// Encrypt / Decrypt
unsigned char* HOSAPI_denc(HOSAPI* api, unsigned char* data, size_t len, AESkey_t key, int* out_len);
unsigned char* HOSAPI_ddec(HOSAPI* api, unsigned char* data, size_t len, AESkey_t key, int* out_len);
unsigned char* HOSAPI_fenc(HOSAPI* api, const char* file, size_t fsize, AESkey_t key, int* out_len);
unsigned char* HOSAPI_fdec(HOSAPI* api, const char* file, size_t fsize, AESkey_t key, int* out_len);

#endif