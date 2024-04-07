#include "../date.h"
#include "../object/plant.h"
#include "../date.h"
#include <stdio.h>

#ifndef PLS_REPORT_H
#define PLS_REPORT_H

void writeOutputFile(char *algo_name, Node *reject_order_list, Node *receive_order_list, struct Plant plants[3], int period_day, char* output_file_name) {
    FILE *file = fopen(output_file_name, "w");

    if (file == NULL) {
        printf("Code error 505 : File problem.\n");
        return;
    }

    int x;
    int plants_number_day[] = {get_size(plants[0].orderDate), get_size(plants[1].orderDate),
                               get_size(plants[2].orderDate)};
    double plants_number_products[] = {0, 0, 0};
    double plants_utilization[3];

    for (x = 0; x < 3; x++) {
        while (get_size(plants[x].orderDate) != 0) {
            struct receive_order_info *temp = (struct receive_order_info *) get_first(plants[x].orderDate);
            plants_number_products[x] += temp->quantity;
            delete_begin(&plants[x].orderDate);
        }
        int total_productiveForces = plants[x].productiveForces * period_day;
        plants_utilization[x] = (plants_number_products[x]/total_productiveForces) * 100;
    }

    fprintf(file, "\n%s\n", "***PLS Schedule Analysis Report***");
    fprintf(file, "\nAlgorithm used: %s\n", "FCFS");
    fprintf(file, "\nThere are %d Orders ACCEPTED. Details are as follows:\n", get_size(receive_order_list));
    fprintf(file, "\n%-15s %-15s %-15s %-8s %-10s %-15s", "ORDER NUMBER", "START", "END", "DAYS", "QUANTITY", "PLANT");
    fprintf(file, "=================================================================================");

//    printf("\norder: %s\n", order->orderNumber);
    int round = 0;
    while (get_size(receive_order_list) != 0) {
        struct receive_order_info *order = (struct receive_order_info *) get_first(receive_order_list);
        char *orderNumber = order->orderNumber;
        char *startDate = order->startDate;
        char *dueDate = order->dueDate;
        int day_period = order->day;
        int quantity = order->quantity;
        char *plantName = order->plantName;

        fprintf(file, "\n%-15s %-15s %-15s %-8d %-10d %-15s", orderNumber, startDate, dueDate, day_period, quantity,
                plantName);
        delete_begin(&receive_order_list);
        round++;
    }
    fprintf(file, "\n\n                  %s\n\n", "- End -");
    fprintf(file, "=================================================================================");

    fprintf(file, "\n\nThere are %d Orders REJECTED. Details are as follows:\n", get_size(reject_order_list));
    fprintf(file, "\n%-15s %-15s %-15s %-8s ", "ORDER NUMBER", "PRODUCT NAME", "Due Date", "QUANTITY");
    fprintf(file, "=================================================================================");
    // reject loop here
    while (get_size(reject_order_list)!=0){
        struct Order *temp = (struct Order *) get_first(reject_order_list);
        removeNewline(temp->productName);
        fprintf(file, "\n%-15s %-15s %-15s %-8d ", temp->orderNumber, temp->productName, temp->dueDate, temp->quantity);
        delete_begin(&reject_order_list);
    }

    fprintf(file, "\n\n                  %s\n\n", "- End -");
    fprintf(file, "=================================================================================");

    fprintf(file, "\n\n***PERFORMANCE");
    for (x = 0; x < 3; x++) {
        fprintf(file, "\n\n%s:", plants[x].name);
        fprintf(file, "\n            ");
        fprintf(file, "%-30s", "Number of days in use:");
        fprintf(file, "%15d day", plants_number_day[x]);
        fprintf(file, "\n            ");
        fprintf(file, "%-30s", "Number of products produced:");
        fprintf(file, "%15d (in total)", (int)plants_number_products[x]);
        fprintf(file, "\n            ");
        fprintf(file, "%-30s", "Utilization of the plant:");
        fprintf(file, "%15.1f %%", plants_utilization[x]);
    }
    fprintf(file, "\n\nOverall of utilization:");
    fprintf(file, "%34.1f %%", (plants_utilization[0]+plants_utilization[1]+plants_utilization[2])/3);
    fprintf(file, "\n\n");
    fclose(file);
}

void printPlantSchedule(struct Plant *plant, int period_day, char *start_date, char *end_date) {
    int x;
    char *date_table = start_date;
//    int order_size = get_size(plant->myOrder);
    int order_size = get_size(plant->orderDate);

    printf("\n%s (%d per day)\n", plant->name, plant->productiveForces);
    printf("%s to %s\n", start_date, end_date);
    printf("%-15s| %-15s| %-15s| %-15s| %-15s|\n", "Date", "Product Name", "Order Number", "Quantity", "Due Date");
    for (x = 0; x < period_day; x++) {
        if (x < order_size) {
            struct Order *order = (struct Order *) get_first(plant->orderDate);
            char *orderNumber = order->orderNumber;
            char *dueDate = order->dueDate;
            int quantity = order->quantity;
            char *productName = order->productName;
            removeNewline(productName);
//            printf("name %s\n", productName);
            printf("%-15s| %-15s| %-15s| %-15d| %-15s|\n", date_table, productName, orderNumber, quantity, dueDate);
            delete_begin(&plant->orderDate);
        } else
            printf("%-15s| %-15s| %-15s| %-15s| %-15s|\n", date_table, "NA", "", "", "");
        addOneDay(date_table, date_table);
    }
}

#endif