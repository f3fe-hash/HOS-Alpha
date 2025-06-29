/*
 * HTP - HOS Tunneling Protocol
 * 
 * Create a safe communication tunnel between a client and server using RSA
 * public/private key pairs alog with AES-256 encryption for a 100%
 * safe protocol.
 * 
 * Added: Thu May 22, 2025
 * Latest update: Fri May 23, 2025
 */

#ifndef __HTP_H__
#define __HTP_H__

#include <openssl/dh.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <stdint.h>

#include "Crypto/crypto.h"
#include "lib/memory.h"
#include "net/netroot.h"

#define HTP_PUBLIC_KEY_FILE  "etc/htp/public.txt"
#define HTP_PRIVATE_KEY_FILE "etc/htp/private.txt"

#define HTP_PORT 42315

typedef struct
{
    int sock;
    DH* dh;
    unsigned char session_key[32];
    AESkey_t aes_ctx;
    char* username;
} HTPConnection;

// Client-side
HTPConnection* htp_client_connect(const char* user, const char* ip);
void htp_client_authenticate(HTPConnection* pConn);
int htp_client_send(HTPConnection* pConn, const unsigned char* data, int data_len);
int htp_client_recv(HTPConnection* pConn, unsigned char* data, int max_len);

// Server-side
HTPConnection* htp_server_connect(const char* user, const char* ip);
int htp_server_authenticate(HTPConnection* pConn);
int htp_server_send(HTPConnection* pConn, const unsigned char* data, int data_len);
int htp_server_recv(HTPConnection* pConn, unsigned char* data, int max_len);

// Socket Functions
int net_tcp_connect(const char* ip, int port);

// RSA Functions
RSA* generate_rsa_keypair(int bits);
void save_rsa_keys(RSA* rsa);
int rsa_sign(RSA* rsa, const unsigned char* msg, size_t msg_len, unsigned char* sig, size_t sig_len);
int rsa_verify(RSA* rsa, const unsigned char* msg, size_t msg_len, unsigned char* sig, size_t sig_len);
int rsa_encrypt(RSA* rsa, const unsigned char* in, size_t in_len, unsigned char* out);
int rsa_decrypt(RSA* rsa, const unsigned char* in, size_t in_len, unsigned char* out);

#endif
