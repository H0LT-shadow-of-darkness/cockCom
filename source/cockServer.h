/**
 * cockServer.h - Server module header for cockCom application
 * 
 * This header file contains declarations and includes needed for the
 * server-side functionality of the cockCom chat application.
 * The server handles multiple client connections using threading.
 */

#ifndef COCKSERVER_H
#define COCKSERVER_H

// Include common definitions and structures
#include "cockTool.h"
    
// System includes for network programming and threading
#include <unistd.h>         // POSIX operating system API (close, etc.)
#include <arpa/inet.h>      // Internet address manipulation (inet_addr, htons, etc.)
#include <sys/socket.h>     // Socket programming functions (socket, bind, listen, etc.)
#include <netinet/in.h>     // Internet protocol family structures (sockaddr_in)
#include <pthread.h>        // POSIX threads for handling multiple clients
#include <errno.h>          // Error number definitions for system calls

// Server Configuration
#define MAXSOCKET 100       // Maximum number of concurrent client connections

/**
 * Main server function - Entry point for server mode
 * 
 * Creates and configures a TCP server socket, binds to the specified
 * network address and port, then listens for and accepts client connections.
 * Each client connection is handled in a separate thread.
 * 
 * @param net Network address string (IP address to bind to, e.g., "127.0.0.1")
 * @param port Port number string to bind to (e.g., "8080")
 * @return 0 on success (though function typically doesn't return due to exit calls)
 */
int cockServer(const char *net, const char* port);

#endif