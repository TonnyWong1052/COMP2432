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

void SJFchildProcess(int pc[2], int cp[2], struct Plant *plant, int period_day, char *start_date, char *end_date) {
    close(pc[1]);
    close(cp[0]);
    int list_have_order = 1;
    int plantProductiveForces = plant->productiveForces;
    int remainAvabiliableDay = period_day;
    int expected_day_production = 0;
    char plant_new_avab_day[11];
    strcpy(plant_new_avab_day, start_date);

    struct Order *order = malloc(sizeof(struct Order));
    while (true) {
        // list_have_order = 0 : not order, shut down the process. list_have_order = 1 : parent send order to child.
        // list_have_order = -1, parent ask for current start date
        read(pc[0], &list_have_order, sizeof(list_have_order));
        if (list_have_order == 0)
            break;
        else if (list_have_order == -1) {
            write(cp[1], &plant_new_avab_day, sizeof(plant_new_avab_day));
            write(cp[1], &remainAvabiliableDay, sizeof(remainAvabiliableDay));
            continue;   // complete this command, then wait for receive command of 'list_have_order'
        } else if (list_have_order == 2) {  // parent directly insert order to child without any response to parent
            // directly insert the order to order list of plant
            read(pc[0], order, sizeof(struct Order));
            char temp_date[11];
            strcpy(temp_date, plant_new_avab_day);
            expected_day_production = calculate_productive_day(order->quantity, plant->productiveForces);
            addDays(plant_new_avab_day, expected_day_production, temp_date);
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
        }

        // if order received this order, check how many day the plant needs to process
        read(pc[0], order, sizeof(struct Order));

        // calculate the period day, how long the order need to complete
        expected_day_production = calculate_productive_day(order->quantity, plant->productiveForces);

        char temp_date[11];
        strcpy(temp_date, plant_new_avab_day);  // clone 'plant_new_avab_day' to 'temp_Date' without address memory
        addDays(plant_new_avab_day, expected_day_production, temp_date);    // calculate the complete date

        // check should the plant receive the order. if reject it, return the reason
        // send confirm message : is_receive_order = 1 : receive the oder, 0 : not enough Productive Forces, -1 : order due date not match date
        int is_receive_order;
        if (isDateLater(temp_date, order->dueDate)) {
            is_receive_order = -1;  // -1 : order due date not match date
        } else if (remainAvabiliableDay - expected_day_production < 0)
            is_receive_order = 0;  // 0 : not enough Productive Forces
        else {
            remainAvabiliableDay = remainAvabiliableDay - expected_day_production;
            is_receive_order = 1;   // 1 : plant have ability to receive the order
        }

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
            addOneDay(plant_new_avab_day);
        }
    }

    printPlantSchedule(plant, period_day, start_date, end_date);
    int trash_value = -1;
    write(cp[1], &trash_value, sizeof(trash_value));   // send trash_value to inform parent, child process is close
    close(pc[0]);
    close(cp[1]);
}

void SJFprocessComput(int pc[2], int cp[2], struct Plant *plant, int period_day, char *start_date, char *end_date) {
    int child_id = fork();
    if (child_id == 0) {
        SJFchildProcess(pc, cp, plant, period_day, start_date, end_date);
        exit(0);
    }
}

void SJFalgo(Node **order_list, struct Plant plants[3], char *start_date, char *end_date, char *output_file_name) {
    int pcX[2], pcY[2], pcZ[2];
    int cpX[2], cpY[2], cpZ[2];
    int *pc[] = {pcX, pcY, pcZ};
    int *cp[] = {cpX, cpY, cpZ};
    int x;
    int period_day = calculateDaysBetweenDate(start_date, end_date) + 1;
    for (x = 0; x < plantCount; x++) {
        pipe(pc[x]);
        pipe(cp[x]);
        SJFprocessComput(pc[x], cp[x], &plants[x], period_day, start_date, end_date);
    }

    closeChannel(pc, 0);    // close all pipe channel with their index
    closeChannel(cp, 1);
    int round = 0, reject_order_count = 0;
    int inform_child_message = 1;

    Node *reject_order_list = NULL, *receive_order_list = NULL;;
    while (true) {
        inform_child_message = get_size(*order_list) > 0;   // seek does order list have any order
        if (inform_child_message == 1)
            write((pc[round % plantCount])[1], &inform_child_message, sizeof(inform_child_message));
        else {
            int completedReport;
            inform_child_message = 0;    // inform_child_message = 0 is equals list do not have order
            // inform all child process, shut down while loop and print their report one by one
            write((pc[0])[1], &inform_child_message, sizeof(inform_child_message));
            read((cp[0])[0], &completedReport, sizeof(completedReport));
            write((pc[1])[1], &inform_child_message, sizeof(inform_child_message));
            read((cp[1])[0], &completedReport, sizeof(completedReport));
            write((pc[2])[1], &inform_child_message, sizeof(inform_child_message));
            read((cp[2])[0], &completedReport, sizeof(completedReport));
            break;
        }
        int shortest_job_index = 0, shortest_job = ((struct Order *)get_first(*order_list))->quantity;
        for (x = 0; x < get_size(*order_list); x++){
            struct Order *temp = (struct Order *) getElementFromIndex(*order_list, x);
            int current_qty = temp->quantity;
            if(current_qty < shortest_job)  // 1 is minimum value(Product_A), 4 is maximum value
                shortest_job_index = x, shortest_job = current_qty;
        }


        struct Order *order = getElementFromIndex(*order_list, shortest_job_index);
        // Send the order to plant (child process)
        write((pc[round % plantCount])[1], order, sizeof(struct Order));

        int is_receive_order;
        read((cp[round % plantCount])[0], &is_receive_order, sizeof(is_receive_order));
        // receive message from child:
        // is_receive_order = 1 : receive the order . if equal 0 : not enough Productive Forces . if equal -1 : order due date not match date
        if (is_receive_order == 1) {
            deleteElementFromIndex(order_list, shortest_job_index); // plant accept the order

            int day_period, qty;
            struct receive_order_info *plant_order_info = malloc(sizeof(struct receive_order_info));
            char temp_start_date[11], temp_end_date[11];
            char *plant_name;
            read((cp[round % plantCount])[0], temp_start_date, sizeof(temp_start_date));
            read((cp[round % plantCount])[0], temp_end_date, sizeof(temp_end_date));
            read((cp[round % plantCount])[0], &day_period, sizeof(day_period));
            read((cp[round % plantCount])[0], &qty, sizeof(qty));
            plant_name = getPlant(round % plantCount + 1);

            setReceiveOrderInfoValues(plant_order_info, order->orderNumber, temp_start_date, temp_end_date, day_period,
                                      qty,
                                      plant_name);
            addToTail(&receive_order_list, plant_order_info);
            addToTail(&plants[round % plantCount].orderDate, plant_order_info);
        }

        if (is_receive_order == -1) { // if order due date not match date, let's find other plant
            reject_order_count++;   //  the order has been rejected by the plant
        } else if (is_receive_order == 0) {
            // Due to plant do not have enough productive forces
            // the parent would ask for all child status and evenly place this order to plants (like send product_a to plant_X and plant_Y)
            inform_child_message = -1;  // -1 is ask for child their status
            // ask all child process for their current plant status such like available start time
            write((pc[round % plantCount])[1], &inform_child_message, sizeof(inform_child_message));
            write((pc[(round + 1) % plantCount])[1], &inform_child_message, sizeof(inform_child_message));
            write((pc[(round + 2) % plantCount])[1], &inform_child_message, sizeof(inform_child_message));

            char temp_start_date_first[11], temp_start_date_second[11], temp_start_date_thir[11];
            char *temp_start_dates[] = {temp_start_date_first, temp_start_date_second, temp_start_date_thir};
            int remain_avabiliable_day_first = -1, remain_avabiliable_day_second = -1, remain_avabiliable_day_thir = -1;
            int remain_avabiliable_day[] = {remain_avabiliable_day_first, remain_avabiliable_day_second,
                                            remain_avabiliable_day_thir};
            // get value by child process (plant)
            read((cp[round % plantCount])[0], temp_start_dates[0], sizeof(temp_start_date_first));
            read((cp[round % plantCount])[0], &remain_avabiliable_day[0],
                 sizeof(remain_avabiliable_day[0]));
            read((cp[(round + 1) % plantCount])[0], temp_start_dates[1], sizeof(temp_start_date_second));
            read((cp[(round + 1) % plantCount])[0], &remain_avabiliable_day[1],
                 sizeof(remain_avabiliable_day[1]));
            read((cp[(round + 2) % plantCount])[0], temp_start_dates[2], sizeof(temp_start_date_thir));
            read((cp[(round + 2) % plantCount])[0], &remain_avabiliable_day[2],
                 sizeof(remain_avabiliable_day[2]));

            // calculate the available production from the available start date of plant to due date of order
            int plant_production_to_order_due[] = {0, 0, 0};
            for (x = 0; x < plantCount; x++) {
                int count_temp = 0;
                while (strcmp(temp_start_dates[(round + x) % plantCount], order->dueDate) != 0 &&
                       remain_avabiliable_day[(round + x) % plantCount] != count_temp) {
                    plant_production_to_order_due[(round + x) % plantCount] += plants[(round + x) %
                                                                                      plantCount].productiveForces;
                    addOneDay(temp_start_dates[(round + x) % plantCount]);
                    count_temp++;
                }
            }

            // if current all plant unable to handle the qty of order, let's move it to reject_order_list
            int current_plant_production = plant_production_to_order_due[0] + plant_production_to_order_due[1] +
                                           plant_production_to_order_due[2];
            printf("dubug: %d\n",current_plant_production);
            if (current_plant_production < order->quantity)
                reject_order_count = 3; // move it to reject_order_list (met the following condition)
            else {
                // evenly place this order to plant_X, plant_Y, and plant_Z
                int remain_qty = order->quantity;

                inform_child_message = 2;
                if (plant_production_to_order_due[0] > 0) {
                    order->quantity = plant_production_to_order_due[0];

                    write((pc[round % plantCount])[1], &inform_child_message, sizeof(inform_child_message));
                    write((pc[round % plantCount])[1], order, sizeof(struct Order)); //double test
                    remain_qty -= plant_production_to_order_due[0];

                    // Record the data to list
                    struct receive_order_info *plant_order_info = malloc(sizeof(struct receive_order_info));
                    setReceiveOrderInfoValuesV2(plant_order_info, order->orderNumber, start_date);
                    int order_period = calculate_productive_day(order->quantity,
                                                                plants[round % plantCount].productiveForces);
                    setReceiveOrderInfoValuesV3(plant_order_info, order_period, order->quantity,
                                                plants[round % plantCount].name);
                    addToTail(&receive_order_list, plant_order_info);
                    addToTail(&plants[round % plantCount].orderDate, plant_order_info);
                }

                if (plant_production_to_order_due[1] > 0) {
                    if (remain_qty < plant_production_to_order_due[1])
                        order->quantity = remain_qty;   // if the plant can handle all remain qty
                    else   // if unable handle all remaining qty, as well as possible to process remaining qty
                        order->quantity = plant_production_to_order_due[1];

                    write((pc[(round + x + 1) % plantCount])[1], &inform_child_message, sizeof(inform_child_message));
                    write((pc[(round + x + 1) % plantCount])[1], order, sizeof(struct Order)); //double test
                    remain_qty -= plant_production_to_order_due[1];

                    struct receive_order_info *plant_order_info = malloc(sizeof(struct receive_order_info));
                    setReceiveOrderInfoValuesV2(plant_order_info, order->orderNumber, start_date);
                    int order_period = calculate_productive_day(order->quantity,
                                                                plants[(round + 1) % plantCount].productiveForces);
                    setReceiveOrderInfoValuesV3(plant_order_info, order_period, order->quantity,
                                                plants[(round + 1) % plantCount].name);
                    addToTail(&receive_order_list, plant_order_info);
                    addToTail(&plants[(round + 1) % plantCount].orderDate, plant_order_info);
                }

                if (remain_qty > 0 && plant_production_to_order_due[2] > 0) {
                    order->quantity = remain_qty;
                    write((pc[(round + x + 2) % plantCount])[1], &inform_child_message, sizeof(inform_child_message));
                    write((pc[(round + x + 2) % plantCount])[1], order, sizeof(struct Order)); //double test

                    struct receive_order_info *plant_order_info = malloc(sizeof(struct receive_order_info));
                    setReceiveOrderInfoValuesV2(plant_order_info, order->orderNumber, start_date);
                    int order_period = calculate_productive_day(order->quantity,
                                                                plants[(round + 2) % plantCount].productiveForces);
                    setReceiveOrderInfoValuesV3(plant_order_info, order_period, order->quantity,
                                                plants[(round + 2) % plantCount].name);
                    addToTail(&receive_order_list, plant_order_info);
                    addToTail(&plants[(round + 2) % plantCount].orderDate, plant_order_info);
                }
                deleteElementFromIndex(order_list, shortest_job_index);
            }
        }

        if (reject_order_count >= plantCount) { // no one plant accept the order, move to reject list
            addToTail(&reject_order_list, order);
            deleteElementFromIndex(order_list, shortest_job_index);
            reject_order_count = 0;
        }
        round++;
    }
    for (x = 0; x < plantCount; x++)
        wait(NULL); // wait for all child process is finish

    closeChannel(pc, 1);
    closeChannel(cp, 0);
    printf("\n");
    writeOutputFile("SJF", reject_order_list, receive_order_list, plants, period_day, output_file_name);
}