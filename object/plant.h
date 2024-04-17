#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "../LinkedList.h"
#include "../object/order.h"

#ifndef PLANT_H
#define PLANT_H

struct Plant {
    int id;
    char name[50];
    int productiveForces;
    Node *myOrder;      // record all received order by the plant
    Node *orderDate;    // record the order scheduling in order to print report
};

void setPlantAttributes(struct Plant* plant, int id, const char* name, int productiveForces) {
    plant->id = id;
    strcpy(plant->name, name);
    plant->productiveForces = productiveForces;
    plant->myOrder = NULL;
    plant->orderDate = NULL;
}

void printAllOrderName(struct Node *order_list){
    int x;
    for (x=0;x<get_size(order_list);x++){
        struct Order order = *(struct Order*)getElementFromIndex(order_list, x);
        printf("Obtained order: %s \n", order.productName);
    }
}

void printFirstOrderItem(struct Plant* plant){  // for testing only
    struct Order* firstOrder = (struct Order*)get_first(plant->myOrder);
    int order_size = get_size(plant->myOrder);
    printf("\nThe first order in plant %s is %s.\n", plant->name, firstOrder->orderNumber);
    printf("The size is %d \n", order_size);
}

char* getPlant(int number){
    if(number == 1)
        return "Plant_X";
    if(number == 2)
        return "Plant_Y";
    if(number == 3)
        return "Plant_Z";
    return "Error";
}

void releasePlantData(struct Plant plant){
    while (get_size(plant.orderDate) != 0) {
        delete_begin(&plant.orderDate);
    }
    while (get_size(plant.myOrder) != 0) {
        delete_begin(&plant.myOrder);
    }
}

#endif