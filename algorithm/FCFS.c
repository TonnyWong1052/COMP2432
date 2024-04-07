#include <stdio.h>
#include <stdlib.h>
#include "../LinkedList.h"
#include "../object/order.h"
#include "../object/receive_order_info.h"
#include "runPLS_report.h"
#include <unistd.h>
#include <stdbool.h>

// plant_X, plant_Y, plant_Z
#define plantCount 3

void childProcess(int pc[2], int cp[2], struct Plant *plant, int period_day, char *start_date, char *end_date) {
    close(pc[1]);
    close(cp[0]);
    bool list_have_order = true;
    int plantProductiveForces = plant->productiveForces;
    int remainAvabiliableDay = period_day;
    char plant_new_avab_day[11];
    strcpy(plant_new_avab_day, start_date);
    while (true) {
        read(pc[0], &list_have_order, sizeof(list_have_order));
        if (!list_have_order)
            break;

        struct Order *order = malloc(sizeof(struct Order));
        read(pc[0], order, sizeof(struct Order));
        int expected_day_production = (order->quantity + plantProductiveForces / 2) / plantProductiveForces;

        int is_receive_order;
//        printf("%d - %d \n", remainAvabiliableDay, expected_day_production);

        char temp_date[11];
        strcpy(temp_date, plant_new_avab_day);
        addDays(plant_new_avab_day, expected_day_production, temp_date);
//        printf("two date %s %s\n", temp_date, order->dueDate);

        if (remainAvabiliableDay - expected_day_production > 0 && !isDateLater(temp_date, order->dueDate)) {
            remainAvabiliableDay = remainAvabiliableDay - expected_day_production;
            is_receive_order = 1;   // plant have ability to receive the order
        } else {
            is_receive_order = 0;   // plant reject the order
        }
//        printf("plant name %s , day production : %d, order qty: %d, ability: %d, remain dat: %d\n", plant->name,
//               expected_day_production, order.quantity, plantProductiveForces, remainAvabiliableDay);
//        printf("plant %s, qty: %d received order: %s\n", plant->name, order.quantity, order.productName);
        write(cp[1], &is_receive_order, sizeof(is_receive_order));
        if (is_receive_order == 1) {
            addToTail(&plant->myOrder, order);
            int qty = order->quantity, current_day_qty;
            while (qty > 0) {
                if (qty > plantProductiveForces)
                    current_day_qty = plantProductiveForces;
                else
                    current_day_qty = qty;
                qty -= current_day_qty;
                struct Order *day_order = malloc(sizeof(struct Order));
                setOrderValues(day_order, order->orderNumber, order->dueDate, current_day_qty, order->productName);
                addToTail(&plant->orderDate, day_order);
            }
            write(cp[1], plant_new_avab_day, strlen(plant_new_avab_day) + 1);
            addDays(plant_new_avab_day, expected_day_production - 1, plant_new_avab_day);
            write(cp[1], plant_new_avab_day, strlen(plant_new_avab_day) + 1);
            write(cp[1], &expected_day_production, sizeof(int));
            write(cp[1], &order->quantity, sizeof(order->quantity));
            addOneDay(plant_new_avab_day, plant_new_avab_day);
        }
    }
    printPlantSchedule(plant, period_day, start_date, end_date);
    int trash_value = -1;
    write(cp[1], &trash_value, sizeof(trash_value));
    close(pc[0]);
    close(cp[1]);
}

void processComput(int pc[2], int cp[2], struct Plant *plant, int period_day, char *start_date, char *end_date) {
    int child_id = fork();
    if (child_id == 0) {
        childProcess(pc, cp, plant, period_day, start_date, end_date);
        exit(0);
    }
}

void FCFSalgo(Node **order_list, struct Plant plants[3], char *start_date, char *end_date, char *output_file_name) {
    int pcX[2], pcY[2], pcZ[2];
    int cpX[2], cpY[2], cpZ[2];
    int *pc[] = {pcX, pcY, pcZ};
    int *cp[] = {cpX, cpY, cpZ};
    int x;
    int period_day = calculateDaysBetweenDate(start_date, end_date) + 1;
    for (x = 0; x < plantCount; x++) {
        pipe(pc[x]);
        pipe(cp[x]);
        processComput(pc[x], cp[x], &plants[x], period_day, start_date, end_date);
    }

    closeChannel(pc, 0);    // close all pipe channel with their index
    closeChannel(cp, 1);
    int round = 0, reject_order_count = 0;
    bool list_have_order = true;
    Node *reject_order_list = NULL, *receive_order_list = NULL;;
    while (true) {
        list_have_order = get_size(*order_list) > 0;
        if (list_have_order == true)
            write((pc[round % plantCount])[1], &list_have_order, sizeof(list_have_order));
        else {
            int completedReport;
            // inform all child process, shut down the while loop and print their report one by one
            write((pc[0])[1], &list_have_order, sizeof(list_have_order));
            read((cp[0])[0], &completedReport, sizeof(completedReport));
            write((pc[1])[1], &list_have_order, sizeof(list_have_order));
            read((cp[1])[0], &completedReport, sizeof(completedReport));
            write((pc[2])[1], &list_have_order, sizeof(list_have_order));
            read((cp[2])[0], &completedReport, sizeof(completedReport));
            break;
        }

        struct Order *order = get_first(*order_list);
//        printf("round: %d Send order: %s", round % plantCount, order->productName);
        // Send the order to plant (child process)
        write((pc[round % plantCount])[1], order, sizeof(struct Order));

        int is_receive_order;
        read((cp[round % plantCount])[0], &is_receive_order, sizeof(is_receive_order));

//        printf("is_receive_order: %d", is_receive_order);
        if (is_receive_order == 1) {
            delete_begin(order_list);   // plant accept the order
            reject_order_count = 0;

            int day_period, qty;
            struct receive_order_info *order_info = malloc(sizeof(struct receive_order_info));
            char temp_start_date[11], temp_end_date[11];
            char *plant_name;
            read((cp[round % plantCount])[0], temp_start_date, sizeof(temp_start_date));
            read((cp[round % plantCount])[0], temp_end_date, sizeof(temp_end_date));
            read((cp[round % plantCount])[0], &day_period, sizeof(day_period));
            read((cp[round % plantCount])[0], &qty, sizeof(qty));
            plant_name = getPlant(round % plantCount + 1);

            setReceiveOrderInfoValues(order_info, order->orderNumber, temp_start_date, temp_end_date, day_period, qty,
                                      plant_name);
            addToTail(&receive_order_list, order_info);
            addToTail(&plants[round % plantCount].orderDate, order_info);
        } else
            reject_order_count++;   // the order has been rejected by the plant

        if (reject_order_count >= plantCount) { // no one plant accept the order, move to reject list
            addToTail(&reject_order_list, order);
            delete_begin(order_list);
        }
        round++;
//        printf("round\n");
    }

    for (x = 0; x < plantCount; x++)
        wait(NULL); // wait for all child process is finish

    closeChannel(pc, 1);
    closeChannel(cp, 0);

    printf("\n");
    writeOutputFile("FCFS", reject_order_list, receive_order_list, plants, period_day, output_file_name);
}