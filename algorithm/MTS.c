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
    int plant_index, reject_order_count = 0, round = 0;
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
            read((cp[x])[0], &plant_remaining_productiveForces[x][0], sizeof(plant_remaining_productiveForces[x][0]));
            read((cp[x])[0], &plant_remaining_productiveForces[x][1], sizeof(plant_remaining_productiveForces[x][1]));
        }
        struct Order *order = getElementFromIndex(*order_list, order_index);

        int total = order->quantity;
        int y, z;
        for (x = 0; x <= total / 300 && x <= plant_remaining_productiveForces[0][0]; x++) {
            for (y = 0; y <= total / 400 && y <= plant_remaining_productiveForces[0][1]; y++) {
                for (z = 0; z <= total / 500 && z <= plant_remaining_productiveForces[0][2]; z++) {
                    int l = total - (300 * x + 400 * y + 500 * z);
                    if (l >= 0) {
                        printf("(%d, %d, %d, %d)\n", x, y, z, l);
                    }
                }
            }
        }

        plant_index = determineFastestPlant(pc, cp, plantCount);

        inform_child_message = 1;
        // Send the order to plant (child process)
        write((pc[plant_index % plantCount])[1], &inform_child_message, sizeof(inform_child_message));
        // Send the order to plant (child process)
        write((pc[plant_index % plantCount])[1], order, sizeof(struct Order));

        int is_receive_order;
        read((cp[plant_index % plantCount])[0], &is_receive_order, sizeof(is_receive_order));
        // receive message from child:
        // is_receive_order = 1 : receive the order . if equal 0 : not enough Productive Forces . if equal -1 : order due date not match date
        if (is_receive_order == 1) {
            processAcceptedOrder(pc, cp, order_list, plants, plant_index, &receive_order_list, plantCount, order_index);
        } else if (is_receive_order == -1) { // if order due date not match date, let's find other plant
            reject_order_count++;   //  the order has been rejected by the plant
        } else if (is_receive_order == 0) {
            // Due to plant do not have enough productive forces
            // the parent would ask for all child status and evenly place this order to plants (like send product_a to plant_X and plant_Y)
            evaluateAndDistributeOrders(pc, cp, plants, order, order_list, &reject_order_list, &receive_order_list,
                                        start_date, plant_index, reject_order_count, plantCount, order_index);

        }

        if (reject_order_count >= plantCount) { // no one plant accept the order, move to reject list
            addToTail(&reject_order_list, order);
            deleteElementFromIndex(order_list, order_index);
            reject_order_count = 0;
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