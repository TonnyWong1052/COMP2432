#include <string.h>

#ifndef ORDER_H
#define ORDER_H

struct Order {
    char orderNumber[10];
    char dueDate[20];
    int quantity;
    char productName[50];
};

void setOrderValues(struct Order *order, const char *orderNumber, const char *dueDate, int quantity, const char *productName) {
    strcpy(order->orderNumber, orderNumber);
    strcpy(order->dueDate, dueDate);
    order->quantity = quantity;
    strcpy(order->productName, productName);
}

#endif