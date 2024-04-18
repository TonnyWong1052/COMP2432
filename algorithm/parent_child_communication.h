#include <stdio.h>
#include <stdlib.h>
#include "../LinkedList.h"
#include "../object/order.h"
#include "../object/receive_order_info.h"
#include "runPLS_report.h"
#include <unistd.h>
#include <stdbool.h>

#ifndef COMP2432_PARENT_CHILD_COMMUNICATION_H
#define COMP2432_PARENT_CHILD_COMMUNICATION_H

void childProcess(int pc[2], int cp[2], struct Plant *plant, int period_day, char *start_date, char *end_date) {
    close(pc[1]);
    close(cp[0]);
    int parent_info_message;
    int plantProductiveForces = plant->productiveForces;
    int remainAvabiliableDay = period_day;
    int expected_day_production = 0;
    char plant_new_avab_day[11];
    strcpy(plant_new_avab_day, start_date);

    struct Order *order = malloc(sizeof(struct Order));
    while (true) {
        // waiting for message from parent to do action
        // parent_info_message = 0 : orderlist no order, child should shut down process. parent_info_message = 1 : parent send order to child.
        // parent_info_message = -1 : parent ask for current start date, parent_info_message = 2 : parent directly insert order to order list without alert
        // parent_info_message = 3 : parent ask for remaining day
        read(pc[0], &parent_info_message, sizeof(parent_info_message));
        // special process by parent start
        if (parent_info_message == 0)   // if receive message = 0 that equal order list is null, process should break the while loop
            break;
        else if (parent_info_message == -1) {   // receive message = -1, return back current data to parent
            write(cp[1], &plant_new_avab_day, sizeof(plant_new_avab_day));
            write(cp[1], &remainAvabiliableDay, sizeof(remainAvabiliableDay));
            continue;   // complete this command, then wait for receive command of 'parent_info_message'
        } else if (parent_info_message == 2) {  // parent directly insert order to child without any response to parent
            // directly insert the order to order list of plant
            read(pc[0], order, sizeof(struct Order));
            expected_day_production = calculate_productive_day(order->quantity, plant->productiveForces);
            addDays(plant_new_avab_day, expected_day_production, plant_new_avab_day);
            remainAvabiliableDay -= expected_day_production;
            addToTail(&plant->myOrder, order);
            int qty = order->quantity, current_day_qty;
            while (qty > 0) {   // calculate the expected schedule of the day, such as required qty of the day
                if (qty > plantProductiveForces)
                    current_day_qty = plantProductiveForces;
                else
                    current_day_qty = qty;
                qty -= current_day_qty;
                struct Order *day_order = malloc(sizeof(struct Order));
                setOrderValues(day_order, order->orderNumber, order->dueDate, current_day_qty, order->productName);
                addToTail(&plant->orderDate, day_order);
            }
            continue;
        }else if (parent_info_message == 3) { // parent ask for remaining day only
            write(cp[1], &remainAvabiliableDay, sizeof(remainAvabiliableDay));
            continue;
        } else if (parent_info_message == 4) {
            write(cp[1], &plant->productiveForces, sizeof(plant->productiveForces));
            write(cp[1], &remainAvabiliableDay, sizeof(remainAvabiliableDay));
            write(cp[1], plant_new_avab_day, strlen(plant_new_avab_day) + 1);
            continue;
        }
        // special process by parent end

        // if order received this order, check how many day the plant needs to process
        read(pc[0], order, sizeof(struct Order));

        // calculate the period day, how long the order need to complete
        expected_day_production = calculate_productive_day(order->quantity, plant->productiveForces);

        // can plant hava ability to receive the order. if reject it, return the reason
        // send confirm message : is_receive_order = 1 : receive the oder, 0 : not enough Productive Forces, -1 : order due date not match date
        int is_receive_order;
        if (isDateLater(plant_new_avab_day, order->dueDate)) {
            is_receive_order = -1;  // -1 : order due date not match date
        } else if (remainAvabiliableDay - expected_day_production < 0) {
            is_receive_order = 0;  // 0 : not enough Productive Forces
        } else {
            remainAvabiliableDay = remainAvabiliableDay - expected_day_production;
            is_receive_order = 1;   // 1 : plant received the order
        }

        char temp_date[11];
        strcpy(temp_date, plant_new_avab_day);  // clone 'plant_new_avab_day' to 'temp_Date' without address memory
        addDays(plant_new_avab_day, expected_day_production, temp_date);    // calculate the complete date


        // after detection, if the order is valid, inserting it to order list and send message to parent
        write(cp[1], &is_receive_order, sizeof(is_receive_order));

        if (is_receive_order == 1) {
            addToTail(&plant->myOrder, order);
            int qty = order->quantity, current_day_qty;
            while (qty > 0) {   // calculate the expected schedule of the day, such as required qty of the day
                if (qty > plantProductiveForces)
                    current_day_qty = plantProductiveForces;
                else
                    current_day_qty = qty;
                qty -= current_day_qty;
                struct Order *day_order = malloc(sizeof(struct Order));
                setOrderValues(day_order, order->orderNumber, order->dueDate, current_day_qty, order->productName);
                addToTail(&plant->orderDate, day_order);
            }

            // send data back to parent process in order to print the runPLS_report
            write(cp[1], plant_new_avab_day, strlen(plant_new_avab_day) + 1);
            addDays(plant_new_avab_day, expected_day_production - 1, plant_new_avab_day);
            write(cp[1], plant_new_avab_day, strlen(plant_new_avab_day) + 1);
            write(cp[1], &expected_day_production, sizeof(int));
            write(cp[1], &order->quantity, sizeof(order->quantity));
            addOneDay(plant_new_avab_day, plant_new_avab_day);
        }
    }
    printPlantSchedule(plant, period_day, start_date, end_date);

    int completedReport = 1;
    // send message to inform parent, child process and channel are closed
    write(cp[1], &completedReport, sizeof(completedReport));
    close(pc[0]);
    close(cp[1]);
}

void arrangeChildProcess(int pc[2], int cp[2], struct Plant *plant, int period_day, char *start_date, char *end_date) {
    int child_id = fork();
    if (child_id == 0) {
        childProcess(pc, cp, plant, period_day, start_date, end_date);
        exit(0);
    }
}

int determineFastestPlant(int *pc[2], int *cp[2], int plantCount) {
    int inform_child_message = 3; // Signal to ask for remaining processing time
    int remain_time[plantCount];
    int current_round_plant;

    // Ask each child process for its remaining time
    for (int i = 0; i < plantCount; i++) {
        write(pc[i][1], &inform_child_message, sizeof(inform_child_message));
        read(cp[i][0], &remain_time[i], sizeof(remain_time[i]));
    }

    // Determine which plant can process the fastest (has the most remaining time)
    current_round_plant = 0;
    for (int i = 1; i < plantCount; i++) {
        if (remain_time[current_round_plant] < remain_time[i]) {
            current_round_plant = i;
        }
    }

    return current_round_plant;
}

void shutdownChildProcesses(int *pc[2], int *cp[2], int plantCount) {
    int completedReport;
    int inform_child_message = 0;  // Message to child, no more orders are coming

    for (int i = 0; i < plantCount; i++) {
        write(pc[i][1], &inform_child_message, sizeof(inform_child_message));
        read(cp[i][0], &completedReport, sizeof(completedReport));
    }
}



int evaluateAndDistributeOrders(int *pc[2], int *cp[2], struct Plant plants[], struct Order *order, Node **order_list,
                                 Node **reject_order_list, Node **receive_order_list, char *start_date, int plant_index,
                                 int reject_order_count, int plantCount, int order_index) {
    int inform_child_message = -1;  // -1 is ask for child their status
    int plant_production_to_order_due[plantCount];
    int remain_avabiliable_day[plantCount];


    // ask all child process for their current plant status such like available start time
    write((pc[plant_index % plantCount])[1], &inform_child_message, sizeof(inform_child_message));
    write((pc[(plant_index + 1) % plantCount])[1], &inform_child_message, sizeof(inform_child_message));
    write((pc[(plant_index + 2) % plantCount])[1], &inform_child_message, sizeof(inform_child_message));

    char temp_start_date_first[11], temp_start_date_second[11], temp_start_date_thir[11];
    char *temp_start_dates[] = {temp_start_date_first, temp_start_date_second, temp_start_date_thir};

    // get value by child process (plant)
    read((cp[plant_index % plantCount])[0], temp_start_dates[plant_index % plantCount], sizeof(temp_start_date_first));
    read((cp[plant_index % plantCount])[0], &remain_avabiliable_day[plant_index % plantCount],
         sizeof(remain_avabiliable_day[0]));
    read((cp[(plant_index + 1) % plantCount])[0], temp_start_dates[(plant_index + 1) % plantCount],
         sizeof(temp_start_date_second));
    read((cp[(plant_index + 1) % plantCount])[0], &remain_avabiliable_day[(plant_index + 1) % plantCount],
         sizeof(remain_avabiliable_day[1]));
    read((cp[(plant_index + 2) % plantCount])[0], temp_start_dates[(plant_index + 2) % plantCount],
         sizeof(temp_start_date_thir));
    read((cp[(plant_index + 2) % plantCount])[0], &remain_avabiliable_day[(plant_index + 2) % plantCount],
         sizeof(remain_avabiliable_day[2]));
    // calculate the available production from the available start date of plant to due date of order

    plant_production_to_order_due[plant_index % plantCount] =
            remain_avabiliable_day[plant_index % plantCount] * plants[(plant_index + 0) % plantCount].productiveForces;
    plant_production_to_order_due[(plant_index + 1) % plantCount] =
            remain_avabiliable_day[(plant_index + 1) % plantCount] *
            plants[(plant_index + 1) % plantCount].productiveForces;
    plant_production_to_order_due[(plant_index + 2) % plantCount] =
            remain_avabiliable_day[(plant_index + 2) % plantCount] *
            plants[(plant_index + 2) % plantCount].productiveForces;

    // if current all plant unable to handle the qty of order, let's move it to reject_order_list
    int current_plant_production = plant_production_to_order_due[plant_index % plantCount] +
                                   plant_production_to_order_due[(plant_index + 1) % plantCount] +
                                   plant_production_to_order_due[(plant_index + 2) % plantCount];
//            printf("current productive forces: %d %d %d", plant_production_to_order_due[plant_index % plantCount], plant_production_to_order_due[(plant_index + 1) % plantCount], plant_production_to_order_due[(plant_index + 2) % plantCount]);
    if (current_plant_production < order->quantity) { // if all plant unable to afford the order
//        set reject_order_count = 3; // move it to reject_order_list (met the following condition directly)
        return 3;
    } else {
        // evenly place this order to plant_X, plant_Y, and plant_Z
        int remain_qty = order->quantity;
        inform_child_message = 2;
        if (plant_production_to_order_due[plant_index % plantCount] > 0) {
            order->quantity = plant_production_to_order_due[plant_index % plantCount];

            write((pc[plant_index % plantCount])[1], &inform_child_message, sizeof(inform_child_message));
            write((pc[plant_index % plantCount])[1], order, sizeof(struct Order)); //double test
            remain_qty -= plant_production_to_order_due[plant_index % plantCount];

            // Record the data to list
            struct receive_order_info *plant_order_info = malloc(sizeof(struct receive_order_info));
            setReceiveOrderInfoValuesV2(plant_order_info, order->orderNumber, start_date);
            int order_period = calculate_productive_day(order->quantity,
                                                        plants[plant_index % plantCount].productiveForces);
            setReceiveOrderInfoValuesV3(plant_order_info, order_period, order->quantity,
                                        plants[plant_index % plantCount].name);
            addToTail(receive_order_list, plant_order_info);
            addToTail(&plants[plant_index % plantCount].orderDate, plant_order_info);
        }

        if (plant_production_to_order_due[(plant_index + 1) % plantCount] > 0) {
            if (remain_qty < plant_production_to_order_due[(plant_index + 1) % plantCount])
                order->quantity = remain_qty;   // if the plant can handle all remain qty
            else   // if unable handle all remaining qty, as well as possible to process remaining qty
                order->quantity = plant_production_to_order_due[(plant_index + 1) % plantCount];

            write((pc[(plant_index + 1) % plantCount])[1], &inform_child_message, sizeof(inform_child_message));
            write((pc[(plant_index + 1) % plantCount])[1], order, sizeof(struct Order)); //double test
            remain_qty -= plant_production_to_order_due[(plant_index + 1) % plantCount];

            struct receive_order_info *plant_order_info = malloc(sizeof(struct receive_order_info));
            setReceiveOrderInfoValuesV2(plant_order_info, order->orderNumber, start_date);
            int order_period = calculate_productive_day(order->quantity,
                                                        plants[(plant_index + 1) % plantCount].productiveForces);
            setReceiveOrderInfoValuesV3(plant_order_info, order_period, order->quantity,
                                        plants[(plant_index + 1) % plantCount].name);
            addToTail(receive_order_list, plant_order_info);
            addToTail(&plants[(plant_index + 1) % plantCount].orderDate, plant_order_info);
        }

        if (remain_qty > 0 && plant_production_to_order_due[(plant_index + 2) % plantCount] > 0) {
            order->quantity = remain_qty;
            write((pc[(plant_index + 2) % plantCount])[1], &inform_child_message, sizeof(inform_child_message));
            write((pc[(plant_index + 2) % plantCount])[1], order, sizeof(struct Order)); //double test

            struct receive_order_info *plant_order_info = malloc(sizeof(struct receive_order_info));
            setReceiveOrderInfoValuesV2(plant_order_info, order->orderNumber, start_date);
            int order_period = calculate_productive_day(order->quantity,
                                                        plants[(plant_index + 2) % plantCount].productiveForces);
            setReceiveOrderInfoValuesV3(plant_order_info, order_period, order->quantity,
                                        plants[(plant_index + 2) % plantCount].name);
            addToTail(receive_order_list, plant_order_info);
            addToTail(&plants[(plant_index + 2) % plantCount].orderDate, plant_order_info);
        }
        deleteElementFromIndex(order_list, order_index);
        return 0;
    }
}

void processAcceptedOrder(int *pc[2], int *cp[2], Node **order_list, struct Plant plants[], int plant_index, Node **receive_order_list, int plantCount, int order_index) {
    // Assume get_first returns the first element of the order list
    struct Order *order = getElementFromIndex(*order_list, order_index);
    deleteElementFromIndex(order_list, order_index);

    int day_period, qty;
    struct receive_order_info *plant_order_info = malloc(sizeof(struct receive_order_info));

    char temp_start_date[11], temp_end_date[11];
    char *plant_name;

    // Read order processing details from the child process
    read((cp[plant_index % plantCount])[0], temp_start_date, sizeof(temp_start_date));
    read((cp[plant_index % plantCount])[0], temp_end_date, sizeof(temp_end_date));
    read((cp[plant_index % plantCount])[0], &day_period, sizeof(day_period));
    read((cp[plant_index % plantCount])[0], &qty, sizeof(qty));
    plant_name = getPlant(plant_index % plantCount + 1);

    // Set the received order info
    setReceiveOrderInfoValues(plant_order_info, order->orderNumber, temp_start_date, temp_end_date, day_period, qty,
                              plant_name);

    // Add the order info to the received order list and the plant's order date list
    addToTail(receive_order_list, plant_order_info);
    addToTail(&plants[plant_index % plantCount].orderDate, plant_order_info);
}

#endif //COMP2432_PARENT_CHILD_COMMUNICATION_H
