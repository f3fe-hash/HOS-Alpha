#include "htp.h"

HTPConnection* htp_client_connect(const char* user, const char* ip)
{
    HTPConnection* pConn = mp_alloc(mpool_, sizeof(HTPConnection));
    if (!pConn) return NULL;

    pConn->username = strdup(user);

    // Connect to remote TCP server
    pConn->sock = net_tcp_connect(ip, 22); // Port 22 or your custom port
    if (pConn->sock < 0)
        return NULL;

    // Generate DH key pair
    pConn->dh = DH_new();
    DH_generate_parameters_ex(pConn->dh, 2048, DH_GENERATOR_2, NULL);
    DH_generate_key(pConn->dh);

    // Send public key
    const BIGNUM* pub_key = NULL;
    DH_get0_key(pConn->dh, &pub_key, NULL);
    int pub_len = BN_num_bytes(pub_key);
    unsigned char* pub_bytes = malloc(pub_len);
    BN_bn2bin(pub_key, pub_bytes);

    write(pConn->sock, &pub_len, sizeof(int));
    write(pConn->sock, pub_bytes, pub_len);
    free(pub_bytes);

    // Receive server's public key
    int srv_pub_len = 0;
    read(pConn->sock, &srv_pub_len, sizeof(int));
    unsigned char* srv_pub = malloc(srv_pub_len);
    read(pConn->sock, srv_pub, srv_pub_len);

    BIGNUM* srv_pub_bn = BN_bin2bn(srv_pub, srv_pub_len, NULL);
    free(srv_pub);

    // Compute shared secret
    unsigned char secret[256];
    int secret_len = DH_compute_key(secret, srv_pub_bn, pConn->dh);
    BN_free(srv_pub_bn);

    // Derive AES key from shared secret (SHA-256)
    SHA256(secret, secret_len, pConn->session_key);

    // Initialize AES context
    pConn->aes_ctx.key = pConn->session_key;
    pConn->aes_ctx.iv = NULL;  // or generate an IV if using CBC

    return pConn;
}

void htp_client_authenticate(HTPConnection* pConn)
{
    // Send username
    int user_len = strlen(pConn->username);
    write(pConn->sock, &user_len, sizeof(int));
    write(pConn->sock, pConn->username, user_len);

    // Receive nonce
    int nonce_len = 0;
    read(pConn->sock, &nonce_len, sizeof(int));
    unsigned char* nonce = mp_alloc(mpool_, nonce_len);
    read(pConn->sock, nonce, nonce_len);

    // Load private key
    FILE* fp = fopen(HTP_PRIVATE_KEY_FILE, "r");
    RSA* rsa = PEM_read_RSAPrivateKey(fp, NULL, NULL, NULL);
    fclose(fp);

    // Sign nonce
    unsigned char sig[256];
    size_t sig_len = sizeof(sig);
    rsa_sign(rsa, nonce, nonce_len, sig, sig_len);
    mp_free(mpool_, nonce);
    RSA_free(rsa);

    // Send signature
    write(pConn->sock, &sig_len, sizeof(int));
    write(pConn->sock, sig, sig_len);

    // Receive result
    char auth_result = 0;
    read(pConn->sock, &auth_result, 1);
    if (auth_result != 1)
    {
        close(pConn->sock);
        pConn->sock = -1;
    }
}

int htp_client_send(HTPConnection* pConn, const unsigned char* data, int data_len)
{
    unsigned char iv[16] = {0}; // For now, zero IV. Consider generating random IVs.
    unsigned char enc[1024];
    int out_len = 0;

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, pConn->session_key, iv);
    EVP_EncryptUpdate(ctx, enc, &out_len, data, data_len);

    int final_len = 0;
    EVP_EncryptFinal_ex(ctx, enc + out_len, &final_len);
    out_len += final_len;

    EVP_CIPHER_CTX_free(ctx);

    write(pConn->sock, &out_len, sizeof(int));
    write(pConn->sock, enc, out_len);

    return out_len;
}

int htp_client_recv(HTPConnection* pConn, unsigned char* data, int max_len)
{
    int enc_len = 0;
    read(pConn->sock, &enc_len, sizeof(int));
    unsigned char enc[1024];
    read(pConn->sock, enc, enc_len);

    unsigned char iv[16] = {0};
    int out_len = 0;

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, pConn->session_key, iv);
    EVP_DecryptUpdate(ctx, data, &out_len, enc, enc_len);

    int final_len = 0;
    EVP_DecryptFinal_ex(ctx, data + out_len, &final_len);
    out_len += final_len;

    EVP_CIPHER_CTX_free(ctx);

    return out_len;
}
