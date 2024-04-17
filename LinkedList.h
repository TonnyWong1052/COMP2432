#include <stdio.h>
#include <stdlib.h>
#include "object/order.h"
#include "date.h"

#ifndef LINKED_LIST_H
#define LINKED_LIST_H

typedef struct Node {
    void* data;
    struct Node* next;
} Node;

void printOrderList(Node* head);
void addToTail(Node** head, void* data);
void delete_begin(Node** head);
int get_size(Node* head);
void* get_first(Node* head);
void* getElementFromIndex(Node* head, int index);
void deleteElementFromIndex(Node** head, int index);

Node* createNode(void* data) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->data = data;
    newNode->next = NULL;
    return newNode;
}

void printList(Node* head) {
    Node* temp = head;
    while (temp != NULL) {
        printf("%p -> ", temp->data);
        temp = temp->next;
    }
    printf("NULL\n");
}

int getNumDay(Node* head){
    int count = 0;
    Node* temp = head;
    while (temp != NULL) {
        void* temp_data = temp->data;
        int quantity = ((struct Order*)temp_data)->quantity;
        count += quantity;
        temp = temp->next;
    }
    return count;
}

// for testing only 
void printOrderList(Node* head) {
    Node* temp = head;
    printf("\n%-15s %-15s %-15s %-15s\n", "ORDER NUMBER", "Due Date", "Quantity", "Product Name");

    while (temp != NULL) {
        void* temp_data = temp->data;
        char* orderNumber = ((struct Order*)temp_data)->orderNumber;
        char* dueDate = ((struct Order*)temp_data)->dueDate;
        int quantity = ((struct Order*)temp_data)->quantity;
        char* productName = ((struct Order*)temp_data)->productName;
        removeNewline(productName);
        printf("%-15s %-15s %-15d %-15s\n", orderNumber, dueDate, quantity, productName);
//        printf("Order Number: %s, Due Date: %s, Quantity: %d, Product Name: %s",
//               orderNumber, dueDate, quantity, productName);
        temp = temp->next;
    }
    printf("\n");
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
        //        printf("Order Number: %s, Due Date: %s, Quantity: %d, Product Name: %s",
//               orderNumber, dueDate, quantity, productName);
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

void* getElementFromIndex(Node* head, int index) {
    if (index < 0 || head == NULL) {
        return NULL;
    }

    Node* current = head;
    int count = 0;

    while (current != NULL) {
        if (count == index) {
            return current->data;
        }
        count++;
        current = current->next;
    }

    return NULL;
}

void deleteElementFromIndex(Node** head, int index) {
    if (index < 0 || *head == NULL) {
        return;
    }

    Node* temp = *head;
    if (index == 0) {
        *head = temp->next;
        free(temp);
        return;
    }

    int i;
    for (i = 0; temp != NULL && i < index - 1; i++) {
        temp = temp->next;
    }

    if (temp == NULL || temp->next == NULL) {
        return;
    }

    Node* next = temp->next->next;

    free(temp->next);

    temp->next = next;
}


#endif // LINKED_LIST_H