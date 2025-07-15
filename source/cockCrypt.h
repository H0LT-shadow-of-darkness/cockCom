/**
 * cockCrypt.h - Encryption module header for cockCom application
 * 
 * This header file contains declarations for cryptographic functions
 * used to provide end-to-end encryption for chat messages.
 * Uses OpenSSL library for AES-256-CBC encryption with PBKDF2 key derivation.
 */

#ifndef COCKCRYPT_H
#define COCKCRYPT_H

// Include common definitions and structures
#include "cockTool.h"

// OpenSSL library includes for cryptographic operations
#include <openssl/evp.h>    // High-level cryptographic functions (EVP interface)
#include <openssl/aes.h>    // AES encryption algorithm definitions
#include <openssl/err.h>    // OpenSSL error handling functions
#include <openssl/rand.h>   // Random number generation functions
#include <openssl/conf.h>   // Configuration file processing

// Cryptographic Configuration
#define SALT_LENGTH 16      // Length of salt used in PBKDF2 key derivation (16 bytes)

/**
 * Derive encryption key and initialization vector from password
 * 
 * Uses PBKDF2 (Password-Based Key Derivation Function 2) with HMAC-SHA256
 * to derive cryptographically strong key and IV from a user password.
 * 
 * @param password Input password string
 * @param key Output buffer for derived AES key (must be AES_KEYLENGTH bytes)
 * @param iv Output buffer for derived IV (must be AES_BLOCK_SIZE bytes)
 */
void cockKeyIv(const char *password, unsigned char *key, unsigned char *iv);

/**
 * Encrypt plaintext using AES-256-CBC encryption
 * 
 * Encrypts data using AES-256 in CBC (Cipher Block Chaining) mode.
 * The output may be larger than input due to PKCS padding.
 * 
 * @param plaintext Input data to encrypt
 * @param plaintext_len Length of plaintext in bytes
 * @param key AES encryption key (32 bytes for AES-256)
 * @param iv Initialization vector (16 bytes for AES block size)
 * @param ciphertext Output buffer for encrypted data (should be plaintext_len + AES_BLOCK_SIZE)
 * @return Length of encrypted data in bytes
 */
int cockEncrypt(unsigned char *plaintext, int plaintext_len, unsigned char *key,unsigned char *iv, unsigned char *ciphertext);

/**
 * Decrypt ciphertext using AES-256-CBC decryption
 * 
 * Decrypts data using AES-256 in CBC mode. The key and IV must match
 * those used for encryption.
 * 
 * @param ciphertext Input encrypted data to decrypt
 * @param ciphertext_len Length of ciphertext in bytes
 * @param key AES decryption key (32 bytes, must match encryption key)
 * @param iv Initialization vector (16 bytes, must match encryption IV)
 * @param plaintext Output buffer for decrypted data
 * @return Length of decrypted data in bytes
 */
int cockDecrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *key, unsigned char *iv, unsigned char *plaintext);

#endif