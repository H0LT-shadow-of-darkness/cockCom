/**
 * cockTool.h - Common definitions and structures for cockCom application
 * 
 * This header file contains shared constants, macros, and data structures
 * used throughout the cockCom chat application for consistency and
 * configuration management.
 */

#ifndef COCKTOOL_H
#define COCKTOOL_H

// Standard C library includes
#include <stdio.h>      // Standard input/output functions
#include <stdlib.h>     // General utilities (malloc, exit, etc.)
#include <string.h>     // String manipulation functions

// AES Encryption Configuration
#define AES_KEYLENGTH 32        // AES-256 key length in bytes (256 bits / 8 = 32 bytes)
#define AES_BLOCK_SIZE 16       // AES block size in bytes (128 bits / 8 = 16 bytes)

// Application Configuration
#define BUFFERSIZE 128          // Maximum message buffer size AND terminal width
#define T_HEIGHT 35             // Terminal height in lines

// Note: AES_KEYLENGTH and AES_BLOCK_SIZE are defined twice (lines 8-9 and 11-12)
// This appears to be a duplicate definition that should be cleaned up
#define AES_KEYLENGTH 32          
#define AES_BLOCK_SIZE 16         

/**
 * Structure for sending encrypted messages between client and server
 * Contains both the encrypted data and metadata needed for decryption
 */
typedef struct {
    char encrypted[BUFFERSIZE]; // Buffer containing the encrypted message data
    int size;                   // Actual size of encrypted data (needed for proper decryption)
} send_args;

#endif