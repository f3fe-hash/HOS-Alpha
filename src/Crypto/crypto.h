#ifndef __CRYPTO_H__
#define __CRYPTO_H__

#define USE_FOOTPRINT

// Standard libs
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// OpenSSL headers
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/err.h>

#ifdef USE_FOOTPRINT
#define FOOTPRINT_SIZE 0x200 // 512 bits
#define FOOTPRINT_SIZE_BYTES (FOOTPRINT_SIZE / 8)

#define FOOTPRINT_TYPE uint64_t
#define FOOTPRINT_TYPE_SIZE sizeof(FOOTPRINT_TYPE)
#define FOOTPRINT_TYPE_SIZE_BITS (FOOTPRINT_TYPE_SIZE << 3) * FOOTPRINT_SIZE_BYTES  // << 3 = * 8
#define FOOTPRINT_TYPE_SIZE_BYTES (FOOTPRINT_TYPE_SIZE >> 3) * FOOTPRINT_SIZE_BYTES // >> 3 = / 8

extern FOOTPRINT_TYPE footprint[FOOTPRINT_TYPE_SIZE_BYTES];
#endif

typedef unsigned char byte;

typedef struct
{
    byte *key;
    byte *iv;
} AESkey_t;

byte *rand_bytes(int length);

byte *sha1(const byte *data, size_t len);
byte *sha256(const byte *data, size_t len);
byte *sha512(const byte *data, size_t len);

byte *aes_encrypt(const byte *data, size_t len, AESkey_t key, int key_size, int *out_len);

byte *aes128_enc(const byte *data, size_t len, AESkey_t key);
byte *aes192_enc(const byte *data, size_t len, AESkey_t key);
byte *aes256_enc(const byte *data, size_t len, AESkey_t key);

byte *aes_decrypt(const byte *data, size_t len, AESkey_t key, int key_size, int *out_len);

byte *aes128_dec(const byte *data, size_t len, AESkey_t key);
byte *aes192_dec(const byte *data, size_t len, AESkey_t key);
byte *aes256_dec(const byte *data, size_t len, AESkey_t key);

#endif
