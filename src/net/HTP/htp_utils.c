#include "htp.h"

/*-----------------------------------------------------------------
---------------------------RSA FUNCTIONS---------------------------
-----------------------------------------------------------------*/

RSA* generate_rsa_keypair(int bits)
{
    BIGNUM* pE = BN_new();
    RSA* rsa = RSA_new();

    if (!BN_set_word(pE, RSA_F4) || !RSA_generate_key_ex(rsa, bits, pE, NULL))
    {
        RSA_free(rsa);
        BN_free(pE);
        return NULL;
    }

    BN_free(pE);
    return rsa;
}

void save_rsa_keys(RSA* rsa)
{
    FILE* pub = fopen(HTP_PUBLIC_KEY_FILE, "w");
    FILE* priv = fopen(HTP_PRIVATE_KEY_FILE, "w");
    PEM_write_RSAPublicKey(pub, rsa);
    PEM_write_RSAPrivateKey(priv, rsa, NULL, NULL, 0, NULL, NULL);
    fclose(pub);
    fclose(priv);
}

int rsa_sign(RSA* rsa, const unsigned char* msg, size_t msg_len,
             unsigned char* sig, size_t sig_len)
{
    EVP_PKEY* key = EVP_PKEY_new();
    EVP_PKEY_assign_RSA(key, RSAPrivateKey_dup(rsa));
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();

    EVP_SignInit(ctx, EVP_sha256());
    EVP_SignUpdate(ctx, msg, msg_len);
    int result = EVP_SignFinal(ctx, sig, (unsigned int *)sig_len, key);

    EVP_MD_CTX_free(ctx);
    EVP_PKEY_free(key);
    return result;
}

int rsa_verify(RSA* rsa, const unsigned char* msg, size_t msg_len,
               unsigned char* sig, size_t sig_len)
{
    EVP_PKEY* key = EVP_PKEY_new();
    EVP_PKEY_assign_RSA(key, RSAPublicKey_dup(rsa));
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();

    EVP_VerifyInit(ctx, EVP_sha256());
    EVP_VerifyUpdate(ctx, msg, msg_len);
    int result = EVP_VerifyFinal(ctx, sig, sig_len, key);

    EVP_MD_CTX_free(ctx);
    EVP_PKEY_free(key);
    return result;
}

int rsa_encrypt(RSA* rsa, const unsigned char* in, size_t in_len,
                unsigned char* out)
{
    return RSA_public_encrypt((int)in_len, in, out, rsa, RSA_PKCS1_OAEP_PADDING);
}

int rsa_decrypt(RSA* rsa, const unsigned char* in, size_t in_len,
                unsigned char* out)
{
    return RSA_private_decrypt((int)in_len, in, out, rsa, RSA_PKCS1_OAEP_PADDING);
}

/*--------------------------------------------------------------------
---------------------------SOCKET FUNCTIONS---------------------------
--------------------------------------------------------------------*/

int net_tcp_connect(const char* ip, int port)
{
    int sockfd;
    struct sockaddr_in server_addr;

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        return -1;

    // Setup address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    // Convert IP string to binary form
    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0)
    {
        close(sockfd);
        return -1;
    }

    // Connect to server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        close(sockfd);
        return -1;
    }

    return sockfd;
}
