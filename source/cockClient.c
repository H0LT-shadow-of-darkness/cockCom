#include "cockClient.h"
//GIT TEST

/**
 * Structure to pass arguments to client threads
 * Contains all necessary data for read/write thread communication
 */
typedef struct {
    int* socket;        // Socket file descriptor for server communication
    pthread_t* thread;  // Thread handle for cancellation (read thread from write thread)
    const char* user;   // User nickname for display
    WINDOW* input;      // ncurses window for input line
    WINDOW* output;     // ncurses window for message display
    int* line;          // Current line counter for output window scrolling
    unsigned char* key; // AES encryption key
    unsigned char* iv;  // AES initialization vector
} thread_args;

/**
 * Establish connection to the chat server
 * Creates a TCP socket and connects to the specified server
 * 
 * @param client Pointer to store the client socket descriptor
 * @param server Server address structure
 * @param net Server IP address string
 * @param port Server port number
 * @return 0 on success
 */
static int CcockConnect(int *client, struct sockaddr_in server, const char *net, int port){
    // Create TCP socket
    if ((*client = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("\n Socket creation error \n");
        exit(EXIT_FAILURE);
    }
    
    // Configure server address
    server.sin_family = AF_INET;                    // IPv4
    server.sin_addr.s_addr = inet_addr(net);        // Convert IP string to binary
    server.sin_port = htons(port);                  // Convert port to network byte order

    // Connect to server
    if ((connect(*client, (struct sockaddr*)&server, sizeof(server)))< 0) {
        perror("\nConnection Failed \n");
        exit(EXIT_FAILURE);
    }
    return 0;
}

/**
 * Write thread function - Handles user input and sends messages to server
 * This thread manages the input interface and encrypts outgoing messages
 * 
 * @param arg Pointer to thread_args structure
 * @return NULL when thread exits
 */
static void* CcockWrite(void* arg){
    // Initialize chat log file (overwrite existing)
    FILE *file;
    file = fopen("LastChatLog.txt", "w");
    fclose(file);

    // Extract arguments from thread_args structure
    thread_args* args = (thread_args*)arg;
    int* socket = args->socket;
    pthread_t* thread = args->thread;  // Handle to read thread for cancellation
    const char* user = args->user;
    int* line = args->line;
    unsigned char* key = args->key;    // Encryption key
    unsigned char* iv = args->iv;      // Initialization vector

    // Buffers for message handling
    char encrypted[BUFFERSIZE+AES_BLOCK_SIZE];  // Buffer for encrypted data (with padding)
    char msg[BUFFERSIZE-6];                     // User input buffer (reserve 6 chars for username)
    char buffer[BUFFERSIZE];                    // Complete message buffer (user:message)

    send_args to_send;  // Structure to send encrypted messages

    WINDOW *output = args->output;
    // Create input window at bottom of screen
    WINDOW *input = newwin(1,BUFFERSIZE,T_HEIGHT-1,0);

    // Main input loop (while(69) 
    while(69){
        // Clear and prepare input line
        werase(input);
        wattron(input, COLOR_PAIR(4));              // Red color for prompt
        mvwprintw(input, 0, 0, "%s:",user);         // Display username prompt
        wattroff(input, COLOR_PAIR(4));  
        wrefresh(input);
        
        // Get user input (limited to BUFFERSIZE-6 to leave room for username)
        wgetnstr(input, msg, BUFFERSIZE-6);
        msg[BUFFERSIZE-6] = '\0';                   // Ensure null termination
        snprintf(buffer, sizeof(buffer),"%s:%s",user,msg);  // Format: "username:message"
        
        // Check for exit command
        if(strstr(msg, "exit()") != NULL){
            if (send(*socket, buffer, sizeof(buffer), 0) < 0) {
                perror("Send failed");
                exit(EXIT_FAILURE);
            }
            break;  // Exit the input loop
        }

        // Encrypt the message before sending
        to_send.size = cockEncrypt(buffer, strlen((char*)buffer), key, iv, to_send.encrypted);
        //not Encrypt
        //strncpy(to_send.encrypted, buffer, sizeof(to_send.encrypted));

        // Send encrypted message to server
        if (send(*socket, &to_send, sizeof(to_send), 0) < 0) {
            perror("Send failed");
            exit(EXIT_FAILURE);
        }

        // Display sent message in output window (yellow color)
        wattron(output, COLOR_PAIR(1));
        if(*line<T_HEIGHT-2){
            mvwprintw(output, *line, 0, "%s",buffer);   // Print at current line
        }else{
            scroll(output);                             // Scroll up when window is full
            mvwprintw(output, T_HEIGHT-3, 0, "%s",buffer);
        }
        wattroff(output, COLOR_PAIR(1));
        wrefresh(output);
        (*line)++;  // Increment line counter
        
        // Log message to file
        file = fopen("LastChatLog.txt", "a");
        fprintf(file, "%s\n", buffer);
        fclose(file);
    }
    
    // Cleanup on exit
    pthread_cancel(*thread);  // Cancel the read thread
    pthread_exit(NULL);
}

/**
 * Read thread function - Receives and displays messages from other clients
 * This thread continuously listens for incoming messages and decrypts them
 * 
 * @param arg Pointer to thread_args structure
 * @return NULL when thread exits
 */
static void* CcockRead(void* arg){
    int len;  // Length of decrypted message

    // Initialize chat log file 
    FILE *file;
    file = fopen("LastChatLog.txt", "w");
    fclose(file);

    // Extract arguments from thread_args structure
    thread_args* args = (thread_args*)arg;
    int *socket = args->socket;
    const char* user = args->user;
    int* line = args->line;
    unsigned char* key = args->key;    // Decryption key (same as encryption)
    unsigned char* iv = args->iv;      // Initialization vector
    WINDOW* output = args->output;

    send_args rec;              // Structure to receive encrypted messages
    char buffer[BUFFERSIZE];    // Buffer for decrypted messages

    // Main receive loop
    while(69){
        // Receive encrypted message from server
        if (recv(*socket, &rec, sizeof(rec), 0) < 0) {
            perror("Recv failed");
            exit(EXIT_FAILURE);
        }

        // Decrypt the received message
        len = cockDecrypt(rec.encrypted, rec.size, key, iv, buffer);
        buffer[len] = '\0';  // Null-terminate the decrypted string
        //not crypted
        //strncpy(buffer, rec.encrypted, sizeof(buffer));

        // Display received message in output window (green color)
        wattron(output, COLOR_PAIR(3));
        if(*line<T_HEIGHT-2){
            mvwprintw(output, *line, 0, "%s",buffer);   // Print at current line
        }else{
            scroll(output);                             // Scroll up when window is full
            mvwprintw(output, T_HEIGHT-3, 0, "%s",buffer);
        }
        wattroff(output, COLOR_PAIR(3));
        wrefresh(output);
        (*line)++;  // Increment line counter
        
        // Log received message to file
        file = fopen("LastChatLog.txt", "a");
        fprintf(file, "%s\n", buffer);
        fclose(file);
    }
    pthread_exit(NULL);
}

/**
 * Main communication function - Sets up the chat interface and manages threads
 * Handles password input, ncurses initialization, and thread management
 * 
 * @param socket Pointer to connected socket descriptor
 * @param user Username string for the client
 * @return 0 on success
 */
static int CcockCom(int *socket, const char* user){
    pthread_t writeThread, readThread;  // Thread handles for read/write operations
    int line = 0;                       // Line counter for output window
    char* password = (char*) malloc(sizeof(char)*PASSWORD_MAX_LENGTH);

    // Disable echo for password input (security measure)
    struct termios new_settings;
    tcgetattr(STDIN_FILENO, &new_settings);
    new_settings.c_lflag &= ~ECHO;      // Turn off echo
    tcsetattr(STDIN_FILENO, TCSANOW, &new_settings);

    // Get password from user (hidden input)
    printf("Insert Crypto password ");
    fgets(password,PASSWORD_MAX_LENGTH,stdin);

    // Re-enable echo for normal terminal operation
    tcgetattr(STDIN_FILENO, &new_settings);
    new_settings.c_lflag |= ECHO;       // Turn on echo
    tcsetattr(STDIN_FILENO, TCSANOW, &new_settings);

    // Set up thread arguments and derive encryption keys
    thread_args* args = malloc(sizeof(thread_args));
    args->key = malloc(AES_KEYLENGTH);  // Allocate memory for AES key (32 bytes)
    args->iv = malloc(AES_BLOCK_SIZE);  // Allocate memory for IV (16 bytes)
    cockKeyIv(password, args->key, args->iv);  // Derive key and IV from password
    
    // Securely clear password from memory
    memset(password, 0, PASSWORD_MAX_LENGTH * sizeof(char));
    free(password);
    password = NULL;
    
    // Initialize ncurses interface
    initscr();
    if (!has_colors()) {
        perror("Error no color supported");
        endwin();
        exit(EXIT_FAILURE);
    }
    start_color();
    // Define color pairs for different message types
    init_pair(1, COLOR_YELLOW, COLOR_BLACK);    // Own messages
    init_pair(2, COLOR_BLUE, COLOR_BLACK);      // Title/headers
    init_pair(3, COLOR_GREEN, COLOR_BLACK);     // Received messages
    init_pair(4, COLOR_RED, COLOR_BLACK);       // Input prompt

    // Create output window (main chat area)
    WINDOW *output = newwin(T_HEIGHT-2,BUFFERSIZE,1,0);
    scrollok(output, TRUE);  // Enable scrolling
    idlok(output, TRUE);     // Enable insert/delete line operations
    
    // Display application title
    attron(COLOR_PAIR(2));
    mvprintw(0, 0,"::cockCommunication::");
    attroff(COLOR_PAIR(2));
    refresh();

    // Initialize thread arguments
    args->socket = socket;
    args->thread = &readThread;  // Write thread needs read thread handle for cancellation
    args->user = user;
    args->output = output;
    args->line = &line;
    
    // Create write thread (handles user input and sending)
    if (pthread_create(&writeThread, NULL, CcockWrite, (void*)args) != 0) {
        perror("Failed to create writethread");
        exit(EXIT_FAILURE);
    }

    // Create read thread (handles receiving and displaying messages)
    if (pthread_create(&readThread, NULL, CcockRead, (void*)args) != 0) {
        perror("Failed to create readthread");
        exit(EXIT_FAILURE);
    }
    
    // Wait for write thread to finish (happens when user types "exit()")
    if(pthread_join(writeThread, NULL)) {
        perror("failed to join thread");
        exit(EXIT_FAILURE);
    }

    // Cleanup ncurses
    endwin();

    // Free allocated memory
    free(args->key);
    args->key = NULL;
    free(args->iv);
    args->iv = NULL;
    free(args);
    args = NULL;
    
    return 0;
}

/**
 * Main client function - Entry point for client mode
 * Connects to server and starts the communication interface
 * 
 * @param net Server IP address string
 * @param port Server port number string
 * @param usr Username for the client (max 5 characters)
 * @return 0 on success
 */
int cockClient(const char *net, const char* port, const char* usr){
    int socket;                        // Client socket descriptor
    struct sockaddr_in server_addr;    // Server address structure
   
    // Connect to the chat server
    if(CcockConnect(&socket, server_addr, net, atoi(port))<0){
       perror("Failed to connect socket");
        exit(EXIT_FAILURE);
    }
    
    // Start the communication interface (ncurses chat UI)
    if(CcockCom(&socket, usr)<0){
        perror("Failed to start cockCom client side");
        exit(EXIT_FAILURE);
    }    
    
    // Clean up and close connection
    close(socket);
    return 0;
}