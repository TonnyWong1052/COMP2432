#include <string.h>
#include "../date.h"

#ifndef RECEIVE_ORDER_H
#define RECEIVE_ORDER_H

// the object for recoding the data detail in order to print analysis report
struct receive_order_info {
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

void setReceiveOrderInfoValuesV2(struct receive_order_info *receive_order_info, const char *orderNumber, const char *startDate) {
    strcpy(receive_order_info->orderNumber, orderNumber);
    strcpy(receive_order_info->startDate, startDate);
}

void setReceiveOrderInfoValuesV3(struct receive_order_info *receive_order_info, int day, int quantity, const char *plantName) {
    receive_order_info->day = day;
    receive_order_info->quantity = quantity;
    strcpy(receive_order_info->plantName, plantName);
    char dueDate[11];
    addDays(receive_order_info->startDate, day-1, dueDate);
    strcpy(receive_order_info->dueDate, dueDate);
}

#endif