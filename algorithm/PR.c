#include <stdlib.h>
#include "../LinkedList.h"
#include "../object/order.h"
#include "../object/receive_order_info.h"
#include "parent_child_communication.h"
#include "runPLS_report.h"
#include <unistd.h>
#include <stdbool.h>

// plant_X, plant_Y, plant_Z
#define plantCount 3

int productNameToPRvalue(char productName[50]) {
    if (strstr(productName, "Product_A") != NULL)
        return 1;
    if (strstr(productName, "Product_B") != NULL)
        return 1;
    if (strstr(productName, "Product_C") != NULL)
        return 1;
    if (strstr(productName, "Product_D") != NULL)
        return 2;
    if (strstr(productName, "Product_E") != NULL)
        return 2;
    if (strstr(productName, "Product_F") != NULL)
        return 2;
    if (strstr(productName, "Product_G") != NULL)
        return 3;
    if (strstr(productName, "Product_H") != NULL)
        return 3;
    if (strstr(productName, "Product_I") != NULL)
        return 3;
    else
        return 4;
}


void PR(Node **order_list, struct Plant plants[3], char *start_date, char *end_date, char *output_file_name) {
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
    int inform_child_message;
    Node *reject_order_list = NULL, *receive_order_list = NULL;

    while (true) {
        // seeking the plant which have maximum remaining day
        plant_index = determineFastestPlant(pc, cp, plantCount);

        if (get_size(*order_list) == 0){    // seek does order list have any order
            shutdownChildProcesses(pc, cp, plantCount);
            break;
        }

        // find the order with the highest PR value
        int hight_PR_index = 0, previous_product_PRvalue = productNameToPRvalue(((struct Order *) getElementFromIndex(*order_list, 0))->productName);
        for (x = 0; x < get_size(*order_list); x++){
            struct Order *temp = (struct Order *) getElementFromIndex(*order_list, x);
            int current_pr = productNameToPRvalue(temp->productName);
            if(current_pr == 1){    // 1 is highest PR
                hight_PR_index = x;
                break;
            }
            if(current_pr <= previous_product_PRvalue)    // 1 is minimum value(Product_A), 4 is maximum value
                hight_PR_index = x, previous_product_PRvalue = current_pr;
        }

        // Send the order to plant (child process)
        write((pc[plant_index % plantCount])[1], &inform_child_message, sizeof(inform_child_message));
        struct Order *order = getElementFromIndex(*order_list, hight_PR_index);
        // Send the order to plant (child process)
        write((pc[plant_index % plantCount])[1], order, sizeof(struct Order));

        int is_receive_order;
        read((cp[plant_index % plantCount])[0], &is_receive_order, sizeof(is_receive_order));
        // receive message from child:
        // is_receive_order = 1 : receive the order . if equal 0 : not enough Productive Forces . if equal -1 : order due date not match date
        if (is_receive_order == 1) {
            processAcceptedOrder(pc, cp, order_list, plants, plant_index, &receive_order_list, plantCount, hight_PR_index);
            reject_order_count = 0;
        }else if (is_receive_order == -1) { // if order due date not match date, let's find other plant
            reject_order_count++;   //  the order has been rejected by the plant
        } else if (is_receive_order == 0) {
            // Due to plant do not have enough productive forces
            // the parent would ask for all child status and evenly place this order to plants (like send product_a to plant_X and plant_Y)
            reject_order_count = evaluateAndDistributeOrders(pc, cp, plants, order, order_list, &reject_order_list, &receive_order_list,
                                        start_date, plant_index, reject_order_count, plantCount, hight_PR_index);

        }

        if (reject_order_count >= plantCount) { // no one plant accept the order, move to reject list
            addToTail(&reject_order_list, order);
            deleteElementFromIndex(order_list, hight_PR_index);
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