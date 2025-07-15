#include "cockServer.h"

// Global variables for managing client connections
int sockets[MAXSOCKET];         // Array to store client socket file descriptors
pthread_t threads[MAXSOCKET];   // Array to store thread IDs for each client
int utenti=0;                   // Counter for active users/clients

/**
 * Broadcast a message to all connected clients except the sender
 * 
 * @param frame The message frame to broadcast
 * @param except Socket descriptor of the sender (to exclude from broadcast)
 * @return 0 on success
 */
static int cockShare(send_args frame, int except){
    // Iterate through all possible socket slots
    for(int i=0;i<MAXSOCKET;i++){
        // Send to active sockets that are not the sender
        if(sockets[i]!=except && sockets[i]>0){
            if (send(sockets[i], &frame, sizeof(frame), 0) < 0) {
                perror("Send failed");
                exit(EXIT_FAILURE);
            }
        }
    }
    return 0;
}

/**
 * Thread function to handle communication with a single client
 * Each client gets its own thread running this function
 * 
 * @param arg Pointer to the client socket descriptor
 * @return NULL when thread exits
 */
static void* ScockCom(void* arg){
    int* socket = (int*)arg;    // Cast argument back to socket pointer
    send_args rec;              // Structure to receive messages
    
    // Main communication loop 
    while(69){ 
        // Receive message from client
        if (recv(*socket, &rec, sizeof(rec), 0) < 0) {
            perror("Recv failed");
            exit(EXIT_FAILURE);
        }
        
        // Display received message info on server console
        printf("===========================\n");
        printf("echo %s\n", rec.encrypted);        // Show encrypted message
        printf("utenti attivi %d\n", utenti);      // Show active user count
        printf("===========================\n");
        
        // Check if client wants to exit
        if(strstr(rec.encrypted, "exit()") != NULL){
            break;  // Exit the communication loop
        }
        
        // Broadcast the message to all other clients
        cockShare(rec, *socket);
    }

    // Client disconnection cleanup
    utenti--;  // Decrease active user counter
    
    // If no users left, shutdown the entire server
    if(utenti==0){
        exit(EXIT_SUCCESS);
    }
    
    // Close client socket and mark it as inactive
    close(*socket);
    *socket*=-1;   // Mark socket as closed 
}


/**
 * Set up and configure the server socket
 * Creates socket, sets options, binds to address, and starts listening
 * 
 * @param server Pointer to server socket descriptor
 * @param opt Pointer to socket option value
 * @param address Socket address structure
 * @param net Network address string (IP address)
 * @param port Port number for the server
 * @return 0 on success
 */
static int ScockConnect(int *server, int *opt, struct sockaddr_in address, const char *net, int port){
    socklen_t addrlen = sizeof(address);
    
    // Create TCP socket
    if (((*server) = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    
    // Set socket option to reuse address (prevents "Address already in use" error)
    if (setsockopt((*server), SOL_SOCKET, SO_REUSEADDR, opt, sizeof(*opt)) < 0) {
        perror("setsockopt SO_REUSEADDR failed");
        close(*server);
        exit(EXIT_FAILURE);
    }
    
    // Configure server address structure
    memset(&address, 0, sizeof(address));           // Clear the structure
    address.sin_family = AF_INET;                   // IPv4
    address.sin_addr.s_addr = inet_addr(net);       // Convert IP string to binary
    address.sin_port = htons(port);                 // Convert port to network byte order

    // Bind socket to the specified address and port
    if (bind((*server), (struct sockaddr*)&address, sizeof(address))< 0) {
        perror("Bind failed");
        close((*server));
        exit(EXIT_FAILURE);
    }
    
    // Start listening for incoming connections (queue up to MAXSOCKET connections)
    if (listen((*server), MAXSOCKET) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    return 0;
}

/**
 * Accept incoming client connections and create threads for each client
 * This function will accept up to MAXSOCKET clients
 * 
 * @param socket Pointer to server socket descriptor
 * @param address Socket address structure for client connections
 * @return 0 on success
 */
static int ScockListen(int *socket, struct sockaddr_in address){
    socklen_t addrlen = sizeof(address);
    
    // Accept connections up to the maximum allowed
    for(int i=0;i<MAXSOCKET;i++){
        // Accept a new client connection
        if ((sockets[i] = accept((*socket), (struct sockaddr*)&address, &addrlen))< 0) {
            perror("accept new connection failed");
            exit(EXIT_FAILURE);
        }
        
        utenti++;  // Increment active user count
        
        // Create a new thread to handle this client's communication
        if (pthread_create(&threads[i], NULL, ScockCom, (void*)&sockets[i]) != 0) {
            perror("thread failed");
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}

/**
 * Main server function - Entry point for server mode
 * Initializes the server, starts listening for connections, and manages the server lifecycle
 * 
 * @param net Network address string (IP address to bind to)
 * @param port Port number string to bind to
 * @return 0 on success (though function doesn't actually return due to exit calls)
 */
int cockServer(const char *net, const char* port){
    int socket;                    // Server socket descriptor
    struct sockaddr_in address;   // Server address structure
    int opt = 1;                  // Socket option value for SO_REUSEADDR

    // Display server startup message in blue
    printf("\033[1;34m::CockCom started::\033[0m\n");
    
    // Set up the server socket (create, bind, listen)
    if(ScockConnect(&socket, &opt, address, net, atoi(port))<0){
        perror("Failed to connect socket");
        exit(EXIT_FAILURE);
    }
    
    // Start accepting client connections (this function blocks and runs the server)
    if(ScockListen(&socket, address)<0){
        perror("Failed to listen socket");
        exit(EXIT_FAILURE);
    }
    
    // This code is reached when server shuts down
    printf("\n\033[1;34m::Exiting::\033[0m");
    close(socket);  // Close the server socket
}

