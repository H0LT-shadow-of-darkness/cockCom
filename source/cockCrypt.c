#include "cockCrypt.h"

/**
 * Derive encryption key and initialization vector (IV) from a password
 * Uses PBKDF2 (Password-Based Key Derivation Function 2) with HMAC-SHA256
 * 
 * @param password Input password string
 * @param key Output buffer for the derived AES key (32 bytes for AES-256)
 * @param iv Output buffer for the derived IV (16 bytes for AES block size)
 */
void cockKeyIv(const char *password, unsigned char *key, unsigned char *iv) {
    // Fixed salt for key derivation 
    const unsigned char *salt = (const unsigned char *)"emma";
    
    // Derive the encryption key using PBKDF2
    // Parameters: password, password_len, salt, salt_len, iterations, hash_func, key_len, output
    if (PKCS5_PBKDF2_HMAC(password, strlen(password), salt, SALT_LENGTH, 600000, EVP_sha256(), AES_KEYLENGTH, key) != 1) {
        perror("failed to derive key and iv from psw");
        exit(EXIT_FAILURE);
    }

    // Derive the initialization vector using the same process
    if (PKCS5_PBKDF2_HMAC(password, strlen(password), salt, SALT_LENGTH, 600000, EVP_sha256(), AES_BLOCK_SIZE, iv) != 1) {
        perror("failed to derive key and iv from psw");
        exit(EXIT_FAILURE);
    }
}

/**
 * Handle OpenSSL errors by printing them and aborting the program
 * This is a helper function called when OpenSSL operations fail
 */
static void handleErrors(void){
    ERR_print_errors_fp(stderr);  // Print OpenSSL error stack to stderr
    abort();                      // Terminate the program immediately
}

/**
 * Encrypt plaintext using AES-256-CBC encryption
 * 
 * @param plaintext Input data to encrypt
 * @param plaintext_len Length of the plaintext in bytes
 * @param key AES encryption key (32 bytes for AES-256)
 * @param iv Initialization vector (16 bytes for AES block size)
 * @param ciphertext Output buffer for encrypted data
 * @return Length of the encrypted ciphertext
 */
int cockEncrypt(unsigned char *plaintext, int plaintext_len, unsigned char *key,unsigned char *iv, unsigned char *ciphertext){
    EVP_CIPHER_CTX *ctx;    // OpenSSL cipher context
    int len;                // Temporary length variable
    int ciphertext_len;     // Total length of encrypted data

    // Create and initialize the encryption context
    if(!(ctx = EVP_CIPHER_CTX_new()))
        handleErrors();

    // Initialize the encryption operation with AES-256-CBC
    // AES-256 uses a 256-bit (32-byte) key
    // CBC mode requires a 128-bit (16-byte) initialization vector
    if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
        handleErrors();

    // Provide the message to be encrypted and obtain the encrypted output
    // EVP_EncryptUpdate can be called multiple times for streaming encryption
    if(1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len))
        handleErrors();
    ciphertext_len = len;

    // Finalize the encryption process
    // This may write additional ciphertext bytes (padding)
    if(1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len))
        handleErrors();
    ciphertext_len += len;

    // Clean up the context to free memory
    EVP_CIPHER_CTX_free(ctx);

    return ciphertext_len;  // Return total length of encrypted data
}

/**
 * Decrypt ciphertext using AES-256-CBC decryption
 * 
 * @param ciphertext Input encrypted data to decrypt
 * @param ciphertext_len Length of the ciphertext in bytes
 * @param key AES decryption key (32 bytes for AES-256) - must match encryption key
 * @param iv Initialization vector (16 bytes) - must match the one used for encryption
 * @param plaintext Output buffer for decrypted data
 * @return Length of the decrypted plaintext
 */
int cockDecrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *key, unsigned char *iv, unsigned char *plaintext){
    EVP_CIPHER_CTX *ctx;    // OpenSSL cipher context
    int len;                // Temporary length variable
    int plaintext_len;      // Total length of decrypted data

    // Create and initialize the decryption context
    if(!(ctx = EVP_CIPHER_CTX_new()))
        handleErrors();

    // Initialize the decryption operation with AES-256-CBC
    // Must use the same cipher mode (AES-256-CBC) that was used for encryption
    if(1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
        handleErrors();

    // Provide the message to be decrypted and obtain the plaintext output
    // EVP_DecryptUpdate can be called multiple times for streaming decryption
    if(1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len))
        handleErrors();
    plaintext_len = len;

    // Finalize the decryption process
    // This step removes padding and may write additional plaintext bytes
    if(1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len))
        handleErrors();
    plaintext_len += len;

    // Clean up the context to free memory
    EVP_CIPHER_CTX_free(ctx);

    return plaintext_len;  // Return total length of decrypted data
}
