#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <arpa/inet.h> // For socket functions
#include "list.h"
#include "server.h"

#define SHUTDOWN_DELAY 5
#define DEFAULT_ROOM "Lobby"
#define PORT 9090
#define BACKLOG 5
#define MAXBUFF 1024

int chat_serv_sock_fd; // Server socket
struct node *head = NULL; // Head of the linked list for chat rooms
char *server_MOTD = "Welcome to the chat server! Please be respectful and follow the rules.";

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// Function to create a default room
void create_default_room() {
    struct node *default_room = malloc(sizeof(struct node));
    if (default_room == NULL) {
        perror("Failed to allocate memory for default room");
        exit(1);
    }
    strcpy(default_room->username, DEFAULT_ROOM);
    default_room->socket = -1; // Set to -1 as it's a room, not a user
    default_room->next = NULL;
    head = default_room;

    printf("Default room '%s' created.\n", default_room->username);
}

// Function to send a message to all connected clients
void broadcast_message(const char *message) {
    struct node *current = head;
    while (current != NULL) {
        if (current->socket != -1) { // Skip rooms or disconnected users
            send(current->socket, message, strlen(message), 0);
        }
        current = current->next;
    }
}

// Signal handler for graceful shutdown
void sigintHandler(int sig_num) {
    printf("Received SIGINT (Ctrl-C). Initiating graceful shutdown...\n");

    char shutdown_message[100];
    snprintf(shutdown_message, sizeof(shutdown_message), 
             "Server is shutting down in %d seconds.\n", SHUTDOWN_DELAY);
    
    broadcast_message(shutdown_message);
    
    sleep(SHUTDOWN_DELAY);

    printf("Closing all connections and freeing resources...\n");

    pthread_mutex_lock(&mutex);
    
    struct node *current = head;
    struct node *temp;
    while (current != NULL) {
        if (current->socket != -1) {
            close(current->socket);
        }
        temp = current;
        current = current->next;
        free(temp);
    }
    head = NULL;

    pthread_mutex_unlock(&mutex);

    close(chat_serv_sock_fd);
    printf("Server shutdown complete.\n");
    exit(0);
}

// Function to create and return a server socket
int get_server_socket() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    return sockfd;
}

// Function to start the server and bind it to a port
int start_server(int serv_socket, int backlog) {
    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(serv_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Bind failed");
        return -1;
    }

    if (listen(serv_socket, backlog) == -1) {
        perror("Listen failed");
        return -1;
    }

    printf("Server started on port %d.\n", PORT);
    return 0;
}

// Function to accept a new client connection
int accept_client(int serv_socket) {
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    int client_sock = accept(serv_socket, (struct sockaddr *)&client_addr, &addr_len);
    if (client_sock == -1) {
        perror("Client accept failed");
        return -1;
    }

    printf("Client connected from %s:%d\n", 
           inet_ntoa(client_addr.sin_addr), 
           ntohs(client_addr.sin_port));
    return client_sock;
}

// Function to get the current Message of the Day (MOTD)
char *get_server_MOTD() {
    return server_MOTD;  // Return the MOTD string
}

// Function to set a new MOTD (can be called from the server admin or for configuration changes)
void set_server_MOTD(const char *new_MOTD) {
    if (new_MOTD != NULL && strlen(new_MOTD) < MAXBUFF) {
        server_MOTD = strdup(new_MOTD);  // Dynamically allocate memory for the new MOTD
        printf("New MOTD set: %s\n", server_MOTD);
    } else {
        printf("Error: New MOTD is too long or invalid.\n");
    }
}

// Function to handle the communication with a client
void *client_receive(void *client_socket) {
    int sock = *(int *)client_socket;
    char buffer[MAXBUFF];

    // Send the Message of the Day (MOTD) to the client when they first connect
    send(sock, get_server_MOTD(), strlen(get_server_MOTD()), 0);

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(sock, buffer, sizeof(buffer) - 1, 0);

        if (bytes_received <= 0) {
            printf("Client disconnected.\n");
            close(sock);
            break;
        }

        printf("Received message: %s\n", buffer);
        // Broadcast the message to all connected clients
        broadcast_message(buffer);
    }

    return NULL;
}

int main(int argc, char **argv) {
    signal(SIGINT, sigintHandler);

    // Create the default room for all clients to join when initially connecting
    create_default_room();

    // Open server socket
    chat_serv_sock_fd = get_server_socket();

    // Get ready to accept connections
    if (start_server(chat_serv_sock_fd, BACKLOG) == -1) {
        printf("Start server error\n");
        exit(1);
    }
   
    printf("Server Launched! Listening on PORT: %d\n", PORT);
    
    // Main execution loop
    while (1) {
        // Accept a connection, start a thread
        int new_client = accept_client(chat_serv_sock_fd);
        if (new_client != -1) {
            pthread_t new_client_thread;
            pthread_create(&new_client_thread, NULL, client_receive, (void *)&new_client);
            pthread_detach(new_client_thread);  // Detach the thread so it cleans up automatically
        }
    }

    close(chat_serv_sock_fd);
    return 0;
}
