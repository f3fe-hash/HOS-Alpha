#ifndef __CRYPTO_H__
#define __CRYPTO_H__

#define USE_FOOTPRINT

// OpenSSL headers
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/err.h>

#include "root.h"

#define AES_128 128
#define AES_192 192
#define AES_256 256

typedef struct
{
    unsigned char* key;
    unsigned char* iv;
} AESkey_t;

unsigned char* rand_bytes(int length);

unsigned char* sha1(const unsigned char* data, size_t len);
unsigned char* sha256(const unsigned char* data, size_t len);
unsigned char* sha512(const unsigned char* data, size_t len);

unsigned char* aes_encrypt(const unsigned char* data, size_t len, AESkey_t key, int key_size, int* out_len);

unsigned char* aes128_enc(const unsigned char* data, size_t len, AESkey_t key);
unsigned char* aes192_enc(const unsigned char* data, size_t len, AESkey_t key);
unsigned char* aes256_enc(const unsigned char* data, size_t len, AESkey_t key);

unsigned char* aes_decrypt(const unsigned char* data, size_t len, AESkey_t key, int key_size, int* out_len);

unsigned char* aes128_dec(const unsigned char* data, size_t len, AESkey_t key);
unsigned char* aes192_dec(const unsigned char* data, size_t len, AESkey_t key);
unsigned char* aes256_dec(const unsigned char* data, size_t len, AESkey_t key);

#endif
