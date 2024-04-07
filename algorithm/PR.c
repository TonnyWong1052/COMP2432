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

void PRchildProcess(int pc[2], int cp[2], struct Plant *plant, int period_day, char *start_date, char *end_date) {
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

    }
    printPlantSchedule(plant, period_day, start_date, end_date);
    int trash_value = -1;
    write(cp[1], &trash_value, sizeof(trash_value));
    close(pc[0]);
    close(cp[1]);
}

void PRprocessComput(int pc[2], int cp[2], struct Plant *plant, int period_day, char *start_date, char *end_date) {
    int child_id = fork();
    if (child_id == 0) {
        PRchildProcess(pc, cp, plant, period_day, start_date, end_date);
        exit(0);
    }
}

void PRalgo(Node **order_list, struct Plant plants[3], char *start_date, char *end_date, char *output_file_name) {
    int pcX[2], pcY[2], pcZ[2];
    int cpX[2], cpY[2], cpZ[2];
    int *pc[] = {pcX, pcY, pcZ};
    int *cp[] = {cpX, cpY, cpZ};
    int x;
    int period_day = calculateDaysBetweenDate(start_date, end_date) + 1;
    for (x = 0; x < plantCount; x++) {
        pipe(pc[x]);
        pipe(cp[x]);
        PRprocessComput(pc[x], cp[x], &plants[x], period_day, start_date, end_date);
    }

    closeChannel(pc, 0);    // close all pipe channel with their index
    closeChannel(cp, 1);
    int round = 0;
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

    }

    for (x = 0; x < plantCount; x++)
        wait(NULL); // wait for all child process is finish

    closeChannel(pc, 1);
    closeChannel(cp, 0);

    printf("\n");
//    writeOutputFile("FCFS", reject_order_list, receive_order_list, plants, period_day, output_file_name);
}