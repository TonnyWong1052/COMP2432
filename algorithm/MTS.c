#include <stdio.h>
#include <stdlib.h>
#include "../LinkedList.h"
#include "../object/order.h"
#include "../object/receive_order_info.h"
#include "../date.h"
#include "parent_child_communication.h"
#include "runPLS_report.h"
#include <unistd.h>
#include <stdbool.h>

// plant_X, plant_Y, plant_Z
#define plantCount 3

int findMaxRemainingDayPlant(int plant_remaining_productiveForces[plantCount][3]){
    int y, max_remain_day_plant_index = 0, max_remain_day = plant_remaining_productiveForces[0][1];
    for(y=0;y<plantCount;y++){
        if(plant_remaining_productiveForces[y][1] > max_remain_day)
            max_remain_day = plant_remaining_productiveForces[y][1], max_remain_day_plant_index =  y;
    }
    return max_remain_day_plant_index;
}

bool checkPlantPriorityValue(int x, int y, int z, int max_remain_day_index, int send_order_to_plant[plantCount]){
    if(max_remain_day_index==0) {
        if (x >= send_order_to_plant[0])
            return true;
    }else if(max_remain_day_index==1) {
        if (y >= send_order_to_plant[1])
            return true;
    } else if(max_remain_day_index==2) {
        if (y >= send_order_to_plant[2])
            return true;
    }
    return false;
}

void MTS(Node **order_list, struct Plant plants[3], char *start_date, char *end_date, char *output_file_name) {
    int pcX[2], pcY[2], pcZ[2];
    int cpX[2], cpY[2], cpZ[2];
    int *pc[] = {pcX, pcY, pcZ};
    int *cp[] = {cpX, cpY, cpZ};
    int x;
    int period_day = calculateDaysBetweenDate(start_date, end_date) + 1;
    for (x = 0; x < plantCount; x++) {
        pipe(pc[x]);
        pipe(cp[x]);
        arrangeChildProcess(pc[x], cp[x], &plants[x], period_day, start_date, end_date);
    }

    closeChannel(pc, 0);    // close all pipe channel with their index
    closeChannel(cp, 1);
    int reject_order_count = 0, round = 0;
    int inform_child_message, order_index;
    Node *reject_order_list = NULL, *receive_order_list = NULL;

    while (true) {
        if (get_size(*order_list) == 0) {    // seek does order list have any order
            shutdownChildProcesses(pc, cp, plantCount);
            break;
        }

        // find the order which close to due time
        char *min_date = ((struct Order *) getElementFromIndex(*order_list, 0))->dueDate;
        order_index = 0;
        for (x = 0; x < get_size(*order_list); x++) {
            struct Order *temp = (struct Order *) getElementFromIndex(*order_list, x);
            if (isDateLater(min_date, temp->dueDate)) {
                min_date = temp->dueDate, order_index = x;
            }
        }

        int plant_remaining_productiveForces[plantCount][3];
        for (x = 0; x < plantCount; x++) {
            inform_child_message = 4;
            write((pc[x])[1], &inform_child_message, sizeof(inform_child_message));
            read((cp[x])[0], &plant_remaining_productiveForces[x][0],
                 sizeof(plant_remaining_productiveForces[x][0]));  // get plant productive forces
            read((cp[x])[0], &plant_remaining_productiveForces[x][1],
                 sizeof(plant_remaining_productiveForces[x][1]));  //  set remaining available day of plant
            plant_remaining_productiveForces[x][2] =
                    plant_remaining_productiveForces[x][1] * plant_remaining_productiveForces[x][0];
        }
        struct Order *order = getElementFromIndex(*order_list, order_index);

//        printf("Current order count: %d %d", get_size(*order_list), order->quantity);
//        printf(" print data %d %d %d \n", plant_remaining_productiveForces[0][1], plant_remaining_productiveForces[1][1], plant_remaining_productiveForces[2][1]);
        int send_order_to_plant[] = {0, 0, 0};
//        printf("test %d %d %d", plant_remaining_productiveForces[0][2], plant_remaining_productiveForces[1][2], plant_remaining_productiveForces[2][2]);
        int total = order->quantity;
        int y, z;
        for (x = 0; x <= total / 300 && x <= plant_remaining_productiveForces[0][1]; x++) {
            for (y = 0; y <= total / 400 && y <= plant_remaining_productiveForces[1][1]; y++) {
                for (z = 0; z <= total / 500 && z <= plant_remaining_productiveForces[2][1]; z++) {
                    int l = total - (300 * x + 400 * y + 500 * z);
                    // plant priority: detect which have maximum remaining day
                    if (l == 0 && checkPlantPriorityValue(x,y,z, findMaxRemainingDayPlant(plant_remaining_productiveForces),send_order_to_plant)) {
//                        printf("Spec (%d, %d, %d, %d)\n", x, y, z, l);
                        send_order_to_plant[0] = x;
                        send_order_to_plant[1] = y;
                        send_order_to_plant[2] = z;
                    }
                }
            }
        }
//        printf("data here %d %d %d \n", send_order_to_plant[0], send_order_to_plant[1], send_order_to_plant[2]);

        if (send_order_to_plant[0] + send_order_to_plant[1] + send_order_to_plant[2] != 0) {
            // insert order to plant
            inform_child_message = 2;
            for (x = 0; x < plantCount; x++) {
                int order_qty = send_order_to_plant[x] * plant_remaining_productiveForces[x][0];
                if(order_qty!=0) {
//                    printf("Insert order %s with %d to %s\n", order->orderNumber, order_qty, plants[x].name);
                    order->quantity = order_qty;
                    write((pc[x])[1], &inform_child_message, sizeof(inform_child_message));
                    write((pc[x])[1], order, sizeof(struct Order));
                }
            }
//            printf("\n");
            deleteElementFromIndex(order_list, order_index);
        } else {
//            printf("Auto-arrange order %s\n", order->orderNumber);
            int plant_index = 0;
            for (x = 0; x < plantCount; x++) {
                if (plant_remaining_productiveForces[x][1] > plant_remaining_productiveForces[plant_index][1])
                    plant_index = x;
            }
            evaluateAndDistributeOrders(pc, cp, plants, order, order_list, &reject_order_list, &receive_order_list,
                                        start_date, plant_index, reject_order_count, plantCount, order_index);
        }
        round++;
    }

    for (x = 0; x < plantCount; x++)
        wait(NULL); // wait for all child process is finish

    closeChannel(pc, 1);
    closeChannel(cp, 0);
    printf("\n");
    writeOutputFile("PR", reject_order_list, receive_order_list, plants, period_day, output_file_name);
}