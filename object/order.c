#include <string.h>

struct Order {
    int orderNumber;
    char dueDate[20];
    int quantity;
    char productName[50];
};

void setOrderValues(struct Order *order, int orderNumber, const char *dueDate, int quantity, const char *productName) {
    order->orderNumber = orderNumber;
    strcpy(order->dueDate, dueDate);
    order->quantity = quantity;
    strcpy(order->productName, productName);
}