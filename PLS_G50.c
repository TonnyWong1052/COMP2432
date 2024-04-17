#include "object/plant.h"
#include "object/order.h"
#include "date.h"
#include "command.c"
#include "LinkedList.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
    struct Plant plants[3];
    setPlantAttributes(&plants[0], 0, "Plant_X", 300);
    setPlantAttributes(&plants[1], 1, "Plant_Y", 400);
    setPlantAttributes(&plants[2], 2, "Plant_Z", 500);

    char start_date[11] = "", end_date[11] = "";
    start_date[10] = '\0', end_date[10] = '\0';
    Node *order_list = NULL;

    printf("\n   ~~WELCOME TO PLS~~\n\n");
    while (true) {
        char input_command[100] = "";
        printf("Please enter:\n> ");
        fgets(input_command, sizeof(input_command), stdin);

        if (strncmp(input_command, "test", 4) == 0) {   // for testing only !!!
//            FCFS algo
//            strcpy(input_command, "addPERIOD 2024-06-01 2024-06-11");
//            addPERIOD(input_command, &start_date, &end_date);
//            strcpy(input_command, "addBATCH test_data_G50_FCFS.dat");
//            addBATCH(input_command, &order_list);
//            strcpy(input_command, "runPLS FCFS | printREPORT > report_01_FCFS.txt");
//            runPLS(input_command, &order_list, start_date, end_date, plants);
//            PR algo
//            strcpy(input_command, "addPERIOD 2024-05-01 2024-05-20");
//            addPERIOD(input_command, &start_date, &end_date);
//            strcpy(input_command, "addBATCH test_data_G50_PR.dat");
//            addBATCH(input_command, &order_list);
//            strcpy(input_command, "runPLS PR | printREPORT > report_02_PR.txt");
//            runPLS(input_command, &order_list, start_date, end_date, plants);
//            SJF algo
//            strcpy(input_command, "addPERIOD 2024-04-01 2024-04-11");
//            addPERIOD(input_command, &start_date, &end_date);
//            strcpy(input_command, "addBATCH test_data_G50_SJF.dat");
//            addBATCH(input_command, &order_list);
//            strcpy(input_command, "runPLS SJF | printREPORT > report_03_SJF.txt");
//            runPLS(input_command, &order_list, start_date, end_date, plants);
//            MTS algo
            strcpy(input_command, "addPERIOD 2024-03-01 2024-03-30");
            addPERIOD(input_command, &start_date, &end_date);
            strcpy(input_command, "addBATCH test_data_G50_MTS.dat");
            addBATCH(input_command, &order_list);
            strcpy(input_command, "runPLS MTS | printREPORT > PLS_Report_G50_MTS.txt");
            runPLS(input_command, &order_list, start_date, end_date, plants);
        } else if (strncmp(input_command, "addPERIOD", 9) == 0) {
            addPERIOD(input_command, &start_date, &end_date); // addPERIOD 2024-06-01 2024-06-30
        } else if (strncmp(input_command, "addORDER", 8) == 0) {
            addORDER(input_command, &order_list); // addORDER P0001 2024-06-10 2000 Product_A
        } else if (strncmp(input_command, "addBATCH", 8) == 0) {
            addBATCH(input_command, &order_list); // addBATCH test_data_G50_FCFS.dat
        } else if (strncmp(input_command, "runPLS", 6) == 0) {
            runPLS(input_command, &order_list, start_date, end_date,
                   plants); // runPLS FCFS | printREPORT > report_01_FCFS.txt
        } else if (strncmp(input_command, "exitPLS", 7) == 0) {
            printf("Bye-bye!\n"); // exitPLS
            exit(0);
        } else if (strncmp(input_command, "printDATA", 9) == 0) {
            printDATA(start_date, end_date, order_list); // printDATA
        } else if(strcmp(input_command, " ") == 0)
            continue;
        else{
            removeNewline(input_command);
            printf("Input command '%s' not found\n", input_command);
        }
    }
}