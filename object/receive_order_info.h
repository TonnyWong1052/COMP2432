#include <string.h>

#ifndef RECEIVE_ORDER_H
#define RECEIVE_ORDER_H

struct receive_order_info {  // object for print analysis report only
    char orderNumber[10];
    char startDate[11];
    char dueDate[11];
    int day;
    int quantity;
    char plantName[10];
};

void setReceiveOrderInfoValues(struct receive_order_info *receive_order_info, const char *orderNumber, const char *startDate, const char *dueDate, int day, int quantity, const char *plantName) {
    strcpy(receive_order_info->orderNumber, orderNumber);
    strcpy(receive_order_info->startDate, startDate);
    strcpy(receive_order_info->dueDate, dueDate);
    receive_order_info->day = day;
    receive_order_info->quantity = quantity;
    strcpy(receive_order_info->plantName, plantName);
}

#endif