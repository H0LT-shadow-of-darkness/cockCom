/**
 * cockClient.h - Client module header for cockCom application
 * 
 * This header file contains declarations and includes needed for the
 * client-side functionality of the cockCom chat application.
 * The client provides a graphical ncurses interface with encryption support.
 */

#ifndef COCKCLIENT_H
#define COCKCLIENT_H

// Include local modules
#include "cockTool.h"       // Common definitions and structures
#include "cockCrypt.h"      // Encryption/decryption functions

// System includes for network programming and user interface      
#include <unistd.h>         // POSIX operating system API (close, etc.)
#include <arpa/inet.h>      // Internet address manipulation (inet_addr, htons, etc.)
#include <sys/socket.h>     // Socket programming functions (socket, connect, etc.)
#include <netinet/in.h>     // Internet protocol family structures (sockaddr_in)
#include <pthread.h>        // POSIX threads for separate read/write operations
#include <termios.h>        // Terminal I/O control (for password input without echo)
#include <ncurses.h>        // Text-based user interface library for chat display

// Client Configuration
#define PASSWORD_MAX_LENGTH 50  // Maximum length for encryption password input

/**
 * Main client function - Entry point for client mode
 * 
 * Connects to a cockCom chat server and starts the interactive chat interface.
 * Handles encryption setup, ncurses UI initialization, and multi-threaded
 * communication (separate threads for reading and writing messages).
 * 
 * @param net Server IP address string to connect to (e.g., "127.0.0.1")
 * @param port Server port number string (e.g., "8080")
 * @param usr Username for the client (max 5 characters, displayed in chat)
 * @return 0 on success, exits with failure code on error
 */
int cockClient(const char *net, const char* port, const char* usr);

#endif