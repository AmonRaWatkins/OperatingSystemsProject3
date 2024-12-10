#include "server.h"  // Include the header with the declaration of client_receive
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <ctype.h>

#define MAXBUFF 1024
#define DELIMITERS " \t\r\n\a"

// External variables
extern int numReaders;
extern pthread_mutex_t rw_lock;
extern pthread_mutex_t mutex;

extern struct node *head;
extern char *server_MOTD;  // This should be defined in server.c

// Function to trim leading and trailing whitespace from a string
char *trimwhitespace(char *str) {
    char *end;

    // Trim leading space
    while (isspace((unsigned char)*str)) str++;

    if (*str == 0)  // All spaces?
        return str;

    // Trim trailing space
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;

    // Write new null terminator character
    end[1] = '\0';

    return str;
}

// Function to handle client communication (no longer implemented here)
void handle_client(int client) {
    // This function can be called to handle the specific logic for a connected client.
    // For instance, this function can be invoked in the main server loop after accepting connections.

    int received;
    char buffer[MAXBUFF], sbuffer[MAXBUFF];  // Data buffer
    char tmpbuf[MAXBUFF];  // Data temp buffer
    char cmd[MAXBUFF], username[20];
    char *arguments[80];
    struct node *currentUser;

    send(client, server_MOTD, strlen(server_MOTD), 0);  // Send Welcome Message of the Day

    // Creating the guest username
    sprintf(username, "guest%d", client);
    head = insertFirstU(head, client, username);

    // Add the GUEST to the DEFAULT ROOM (i.e., Lobby)
    // Here, you'd call a function to join the default room.

    while (1) {
        if ((received = read(client, buffer, MAXBUFF)) != 0) {
            buffer[received] = '\0';
            strcpy(cmd, buffer);
            strcpy(sbuffer, buffer);

            // Tokenize the input (split it on whitespace)
            arguments[0] = strtok(cmd, DELIMITERS);

            int i = 0;
            while (arguments[i] != NULL) {
                arguments[++i] = strtok(NULL, DELIMITERS);
                strcpy(arguments[i - 1], trimwhitespace(arguments[i - 1]));
            }

            // Arg[0] = command, Arg[1] = user or room
            if (strcmp(arguments[0], "create") == 0) {
                printf("create room: %s\n", arguments[1]);
                // Perform the operations to create room arguments[1]
                sprintf(buffer, "\nchat>");
                send(client, buffer, strlen(buffer), 0);  // Send back to client
            }
            else if (strcmp(arguments[0], "join") == 0) {
                printf("join room: %s\n", arguments[1]);
                // Perform the operations to join room arguments[1]
                sprintf(buffer, "\nchat>");
                send(client, buffer, strlen(buffer), 0);  // Send back to client
            }
            else if (strcmp(arguments[0], "leave") == 0) {
                printf("leave room: %s\n", arguments[1]);
                // Perform the operations to leave room arguments[1]
                sprintf(buffer, "\nchat>");
                send(client, buffer, strlen(buffer), 0);  // Send back to client
            }
            else if (strcmp(arguments[0], "connect") == 0) {
                printf("connect to user: %s\n", arguments[1]);
                // Perform the operations to connect user with socket = client from arguments[1]
                sprintf(buffer, "\nchat>");
                send(client, buffer, strlen(buffer), 0);  // Send back to client
            }
            else if (strcmp(arguments[0], "disconnect") == 0) {
                printf("disconnect from user: %s\n", arguments[1]);
                // Perform the operations to disconnect user with socket = client from arguments[1]
                sprintf(buffer, "\nchat>");
                send(client, buffer, strlen(buffer), 0);  // Send back to client
            }
            else if (strcmp(arguments[0], "rooms") == 0) {
                printf("List all the rooms\n");
                // Add list of rooms to buffer to send to client
                strcat(buffer, "\nchat>");
                send(client, buffer, strlen(buffer), 0);  // Send back to client
            }
            else if (strcmp(arguments[0], "users") == 0) {
                printf("List all the users\n");
                // Add list of users to buffer to send to client
                strcat(buffer, "\nchat>");
                send(client, buffer, strlen(buffer), 0);  // Send back to client
            }
            else if (strcmp(arguments[0], "login") == 0) {
                // Rename their guestID to username. Make sure any room or DMs have the updated username.
                sprintf(buffer, "\nchat>");
                send(client, buffer, strlen(buffer), 0);  // Send back to client
            }
            else if (strcmp(arguments[0], "help") == 0) {
                sprintf(buffer, "login <username> - \"login with username\"\ncreate <room> - \"create a room\"\njoin <room> - \"join a room\"\nleave <room> - \"leave a room\"\nusers - \"list all users\"\nrooms -  \"list all rooms\"\nconnect <user> - \"connect to user\"\nexit - \"exit chat\"\n");
                send(client, buffer, strlen(buffer), 0);  // Send back to client
            }
            else if (strcmp(arguments[0], "exit") == 0 || strcmp(arguments[0], "logout") == 0) {
                // Remove the initiating user from all rooms and direct connections, then close the socket descriptor.
                close(client);
            }
            else {
                // Send a message to all users in the chat
                sprintf(tmpbuf, "\n::%s> %s\nchat>", "PUTUSERFROM", sbuffer);
                strcpy(sbuffer, tmpbuf);

                currentUser = head;
                while (currentUser != NULL) {
                    if (client != currentUser->socket) {  // Don't send to yourself
                        send(currentUser->socket, sbuffer, strlen(sbuffer), 0);
                    }
                    currentUser = currentUser->next;
                }
            }

            memset(buffer, 0, sizeof(1024));
        }
    }
}
