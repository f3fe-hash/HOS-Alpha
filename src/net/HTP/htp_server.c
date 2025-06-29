#include "htp.h"

HTPConnection* htp_server_connect(const char* user, const char* ip)
{
    HTPConnection* pConn = mp_alloc(mpool_, sizeof(HTPConnection));
    if (!pConn) return NULL;

    pConn->sock = socket(AF_INET, SOCK_STREAM, 0);
    if (pConn->sock < 0) return NULL;

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(HTP_PORT);  // define HTP_PORT appropriately
    inet_pton(AF_INET, ip, &addr.sin_addr);

    if (connect(pConn->sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        close(pConn->sock);
        return NULL;
    }

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

    // Receive peer public key
    int peer_pub_len = 0;
    read(pConn->sock, &peer_pub_len, sizeof(int));
    unsigned char* peer_pub = malloc(peer_pub_len);
    read(pConn->sock, peer_pub, peer_pub_len);

    BIGNUM* peer_pub_bn = BN_bin2bn(peer_pub, peer_pub_len, NULL);
    free(peer_pub);

    // Compute shared secret
    unsigned char secret[256];
    int secret_len = DH_compute_key(secret, peer_pub_bn, pConn->dh);
    BN_free(peer_pub_bn);

    // Derive session key
    SHA256(secret, secret_len, pConn->session_key);

    // Setup AES context
    pConn->aes_ctx.key = pConn->session_key;
    pConn->aes_ctx.iv = NULL;  // Set this up securely as discussed earlier

    return pConn;
}

int htp_server_authenticate(HTPConnection* pConn)
{
    // Receive username
    int user_len = 0;
    read(pConn->sock, &user_len, sizeof(int));
    char* username = mp_alloc(mpool_, user_len + 1);
    read(pConn->sock, username, user_len);
    username[user_len] = 0;
    pConn->username = username;

    // Generate and send nonce
    unsigned char nonce[32];
    RAND_bytes(nonce, sizeof(nonce));
    write(pConn->sock, &(int){sizeof(nonce)}, sizeof(int));
    write(pConn->sock, nonce, sizeof(nonce));

    // Receive signature
    int sig_len = 0;
    read(pConn->sock, &sig_len, sizeof(int));
    unsigned char* sig = mp_alloc(mpool_, sig_len);
    read(pConn->sock, sig, sig_len);

    // Load public key for user
    char pubkey_path[256];
    snprintf(pubkey_path, sizeof(pubkey_path), "keys/%s.pub", username);
    FILE* fp = fopen(pubkey_path, "r");
    if (!fp) return 0;
    RSA* rsa = PEM_read_RSAPublicKey(fp, NULL, NULL, NULL);
    fclose(fp);

    // Verify signature
    int auth_result = rsa_verify(rsa, nonce, sizeof(nonce), sig, sig_len);
    RSA_free(rsa);
    mp_free(mpool_, sig);

    char success = auth_result ? 1 : 0;
    write(pConn->sock, &success, 1);
    if (!auth_result)
    {
        close(pConn->sock);
        pConn->sock = -1;
    }
    return auth_result;
}

int htp_server_send(HTPConnection* pConn, const unsigned char* data, int data_len)
{
    unsigned char iv[16] = {0};
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

int htp_server_recv(HTPConnection* pConn, unsigned char* data, int max_len)
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
