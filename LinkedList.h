#include <stdio.h>
#include <stdlib.h>
#include "object/order.h"

#ifndef LINKED_LIST_H
#define LINKED_LIST_H

typedef struct Node {
    void* data;
    struct Node* next;
} Node;

void appendNode(Node** head, void* data);
void printList(Node* head);
void printOrderList(Node* head);
void addToHead(Node** head, void* data);
void addToTail(Node** head, void* data);
void delete_begin(Node** head);
void delete_end(Node** head);
int get_size(Node* head);
void* get_first(Node* head);
void* get_tail(Node* head);
void* getElementFromIndex(Node* head, int index);

Node* createNode(void* data) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->data = data;
    newNode->next = NULL;
    return newNode;
}

void appendNode(Node** head, void* data) {
    Node* newNode = createNode(data);
    if (*head == NULL) {
        *head = newNode;
    } else {
        Node* temp = *head;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = newNode;
    }
}

void printList(Node* head) {
    Node* temp = head;
    while (temp != NULL) {
        printf("%p -> ", temp->data);
        temp = temp->next;
    }
    printf("NULL\n");
}

void printOrderList(Node* head) {
    Node* temp = head;
    while (temp != NULL) {
        void* temp_data = temp->data;
        char* orderNumber = ((struct Order*)temp_data)->orderNumber;
        char* dueDate = ((struct Order*)temp_data)->dueDate;
        int quantity = ((struct Order*)temp_data)->quantity;
        char* productName = ((struct Order*)temp_data)->productName;

        printf("Order Number: %s, Due Date: %s, Quantity: %d, Product Name: %s\n",
               orderNumber, dueDate, quantity, productName);
        temp = temp->next;
    }
    printf("\n");
}

void addToHead(Node** head, void* data) {
    Node* newNode = createNode(data);
    newNode->next = *head;
    *head = newNode;
}

void addToTail(Node** head, void* data) {
    Node* newNode = createNode(data);
    newNode->next = NULL;

    if (*head == NULL) {
        *head = newNode;
        return;
    }

    Node* current = *head;
    while (current->next != NULL) {
        current = current->next;
    }

    current->next = newNode;
}

void delete_begin(Node** head) {
    if (*head == NULL) {
        return;
    }
    Node* temp = *head;
    *head = (*head)->next;
    free(temp);
}

void delete_end(Node** head) {
    if (*head == NULL) {
        return;
    }
    if ((*head)->next == NULL) {
        free(*head);
        *head = NULL;
    } else {
        Node* temp = *head;
        Node* prev = NULL;
        while (temp->next != NULL) {
            prev = temp;
            temp = temp->next;
        }
        prev->next = NULL;
        free(temp);
    }
}

int get_size(Node* head) {
    int size = 0;
    Node* current = head;
    while (current != NULL) {
        size++;
        current = current->next;
    }
    return size;
}

void* get_first(Node* head) {
    if (head == NULL) {
        return NULL;
    }
    return head->data;
}

void* get_tail(Node* head) {
    if (head == NULL) {
        return NULL;
    }
    Node* temp = head;
    while (temp->next != NULL) {
        temp = temp->next;
    }
    return temp->data;
}

void* getElementFromIndex(Node* head, int index) {
    // Check if the index is negative or the list is empty
    if (index < 0 || head == NULL) {
        return NULL;
    }

    Node* current = head;
    int count = 0;

    // Traverse the list until the end or the desired index is reached
    while (current != NULL) {
        if (count == index) {
            return current->data; // Return the data of the current node
        }
        count++;
        current = current->next;
    }

    // If the loop completes without returning, the index was out of bounds
    return NULL;
}


#endif // LINKED_LIST_H