#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "list.h"

// Insert a new user at the first position in the list
struct node *insertFirstU(struct node *head, int socket, char *username) {
    // Check for duplicates
    if (findU(head, username) == NULL) {
        // Create a new node
        struct node *link = (struct node *)malloc(sizeof(struct node));
        if (!link) {
            fprintf(stderr, "Error: Unable to allocate memory for a new node.\n");
            return head; // Return the unchanged head in case of memory allocation failure
        }

        link->socket = socket;
        strcpy(link->username, username);

        // Point to the old first node
        link->next = head;

        // Set the new node as the head
        head = link;
    } else {
        printf("Duplicate: %s\n", username);
    }
    return head;
}

// Find a user in the list by their username
struct node *findU(struct node *head, char *username) {
    // Start from the first node
    struct node *current = head;

    // If the list is empty
    if (head == NULL) {
        return NULL;
    }

    // Traverse the list
    while (strcmp(current->username, username) != 0) {
        // If it's the last node
        if (current->next == NULL) {
            return NULL;
        } else {
            // Move to the next node
            current = current->next;
        }
    }

    // If username found, return the current node
    return current;
}

// Print all the users in the list
void list_print(struct node *head) {
    struct node *current = head;

    if (head == NULL) {
        printf("The list is empty.\n");
        return;
    }

    printf("Connected Users:\n");
    while (current != NULL) {
        printf("Socket: %d, Username: %s\n", current->socket, current->username);
        current = current->next;
    }
}

// Delete a user from the list by their username
struct node *deleteU(struct node *head, char *username) {
    struct node *current = head;
    struct node *previous = NULL;

    // If the list is empty
    if (head == NULL) {
        printf("The list is empty, cannot delete %s.\n", username);
        return NULL;
    }

    // Traverse the list
    while (strcmp(current->username, username) != 0) {
        // If it's the last node
        if (current->next == NULL) {
            printf("User %s not found.\n", username);
            return head;
        } else {
            // Move to the next node
            previous = current;
            current = current->next;
        }
    }

    // If the user is the head node
    if (current == head) {
        head = head->next;
    } else {
        // Bypass the current node
        previous->next = current->next;
    }

    free(current);
    return head;
}

// Free the entire list
void free_list(struct node *head) {
    struct node *current = head;

    while (current != NULL) {
        struct node *temp = current;
        current = current->next;
        free(temp);
    }
}
