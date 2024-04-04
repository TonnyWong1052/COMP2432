#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "../data_structure/LinkedList.h"
#include "../object/order.h"

#ifndef PLANT_H
#define PLANT_H

struct Plant {
    char name[50];
    int productiveForces;
    Node *myOrder;
};

void printFirstOrderItem(struct Plant* plant){
//    char* orderNumber = ((struct Order*)get_first(plant->myOrder))->orderNumber;
    struct Order* firstOrder = (struct Order*)get_first(plant->myOrder);
    int order_size = get_size(plant->myOrder);
    printf("\nThe first order in plant %s is %s.\n", plant->name, firstOrder->orderNumber);
    printf("The size is %d \n", order_size);
//    struct Order *order;
//    char* productName = ((struct Order*)plant->myOrder->data)->productName;
//    printf("this is product %s", orderNumber);
}

void setPlantAttributes(struct Plant* plant, const char* name, int productiveForces) {
    strcpy(plant->name, name);
    plant->productiveForces = productiveForces;
    plant->myOrder = NULL;
//    setOrderValues(order, "1", "1", 1, "1");
//    plant->myOrder = createNode(NULL);
}

void addOrder(struct Plant* plant, struct Order *order){
    addToTail(&plant->myOrder, order);
}

void printAllOrderName(struct Node *order_list){
    int x;
    for (x=0;x<get_size(order_list);x++){
        struct Order order = *(struct Order*)getElementFromIndex(order_list, x);
        printf("Obtained order: %s \n", order.productName);
    }
}

#endif