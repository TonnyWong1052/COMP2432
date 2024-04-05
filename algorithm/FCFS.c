#include <stdio.h>
#include <stdlib.h>
#include "../data_structure/LinkedList.h"
#include "../object/order.h"
#include "runPLS_report.c"
#include <unistd.h>
#include <stdbool.h>

// plant_X, plant_Y, plant_Z
#define plantCount 3

void closeChannel(int *channel[4], int index) {
    int x;
    for (x = 0; x < 4; x++)
        close((channel[x])[1]);
}

void childProcess(int pc[2], int cp[2], struct Plant *plant, int period_day, char* start_date, char* end_date) {
    close(pc[1]);
    close(cp[0]);
    bool list_have_order = true;
    int plantProductiveForces = plant->productiveForces;
    int remainAvabiliableDay = period_day;

    while (true) {
        read(pc[0], &list_have_order, sizeof(list_have_order));
        if (!list_have_order)
            break;

//        struct Order order;
        struct Order *order = malloc(sizeof(struct Order));
        // Read the order structure directly.
        read(pc[0], order, sizeof(struct Order));
        int expected_day_production = (order->quantity + plantProductiveForces / 2) / plantProductiveForces;

        int is_receive_order;
        if(remainAvabiliableDay - expected_day_production != -1) {
            remainAvabiliableDay = remainAvabiliableDay - expected_day_production;
            is_receive_order = 1;   // plant have ability to receive the order
        }else{
            is_receive_order = 0;   // plant reject the order
        }
//        printf("plant name %s , day production : %d, order qty: %d, ability: %d, remain dat: %d\n", plant->name,
//               expected_day_production, order.quantity, plantProductiveForces, remainAvabiliableDay);
//        printf("plant %s, qty: %d received order: %s\n", plant->name, order.quantity, order.productName);
        if (is_receive_order == 1)
            addToTail(&plant->myOrder, order);

        write(cp[1], &is_receive_order, sizeof(is_receive_order));
    }
    printOrderList(plant->myOrder);
//    printPlantSchedule(plant, period_day, start_date, end_date);
    int trash_value = -1;
    write(cp[1], &trash_value, sizeof(trash_value));
//    printFirstOrderItem(plant);
    close(pc[0]);
    close(cp[1]);
    exit(0);
}

void processComput(int pc[2], int cp[2], struct Plant *plant, int period_day, char* start_date, char* end_date) {
    int child_id = fork();
    if (child_id == 0) {
        childProcess(pc, cp, plant, period_day, start_date, end_date);
    }
}

void FCFSalgo(Node **order_list, struct Plant plants[3], char* start_date, char* end_date) {
    int pcX[2], pcY[2], pcZ[2];
    int cpX[2], cpY[2], cpZ[2];
    int *pc[] = {pcX, pcY, pcZ};
    int *cp[] = {cpX, cpY, cpZ};

    int period_day = calculateDaysBetweenDate(start_date, end_date) + 1;
    for (int x = 0; x < plantCount; x++) {
        pipe(pc[x]);
        pipe(cp[x]);
        processComput(pc[x], cp[x], &plants[x], period_day, start_date, end_date);
    }

//    closeChannel(pc, 0);
//    closeChannel(cp, 1);

    int round = 0;
    bool list_have_order = true;
    while (true) {
        list_have_order = get_size(*order_list) > 0;
        if (list_have_order == true)
            write((pc[round % plantCount])[1], &list_have_order, sizeof(list_have_order));
        else {
            int completedReport;
            // inform all child process, shut down the while loop and print their report one by one
            write((pc[0])[1], &list_have_order, sizeof(list_have_order));
            read((cp[0])[0], &completedReport, sizeof(completedReport));
//            write((pc[1])[1], &list_have_order, sizeof(list_have_order));
//            read((cp[1])[0], &completedReport, sizeof(completedReport));
//            write((pc[2])[1], &list_have_order, sizeof(list_have_order));
//            read((cp[2])[0], &completedReport, sizeof(completedReport));
            break;
        }

        struct Order *order = get_first(*order_list);
//        printf("round: %d Send order: %s", round % plantCount, order->productName);
        // Send the order to plant (child process)
        write((pc[round % plantCount])[1], order, sizeof(struct Order));

        int is_receive_order;
        read((cp[round % plantCount])[0], &is_receive_order, sizeof(is_receive_order));
        if (is_receive_order)
            delete_begin(order_list);

        round++;
    }

//    printFirstOrderItem(&plants[0]);
    printf("\n");
    int x;
    for (x = 0; x < plantCount; x++)
        wait(NULL);
}
