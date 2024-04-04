#include "object/plant.h"
#include "object/order.h"
#include "algorithm/FCFS.c"
#include "date.c"
#include "data_structure/LinkedList.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

void addPERIOD(char input_command[50], char (*start_date)[11], char (*end_date)[11]) {
    // 0123456789012345678012345678901 --> char_index
    // addPERIOD 2024-06-01 2024-06-30 --> use case
    // addPERIOD 2024-06-01a 2024-06-30b --> date format error
    char temp_start_date[11], temp_send_date[11];
    strncpy(temp_start_date, input_command + 10, 10);
    strncpy(temp_send_date, input_command + 21, 10);
    temp_start_date[10] = '\0', temp_send_date[10] = '\0';

    int isContainAdditionalFormat = strcmp(input_command + 32, "") == 0;
//    printf("%d", isContainAdditionalFormat);
    if (date_is_valid(temp_start_date) && date_is_valid(temp_send_date) && isContainAdditionalFormat != 0) {
        strncpy(*start_date, input_command + 10, 10);
        strncpy(*end_date, input_command + 21, 10);
    } else {
        printf("Your command format is invalid: Correct format 'addPeriod YYYY-MM-DD YYYY-MM-DD'\n");
    }
}

void addORDER(char input_command[50], Node **order_list) {
    strtok(input_command, " ");
    char *orderNumber = NULL, *dueDate = NULL, *quantity = NULL, *productName = NULL;
    orderNumber = strtok(NULL, " ");
    dueDate = strtok(NULL, " ");
    quantity = strtok(NULL, " ");
    productName = strtok(NULL, " ");
    char *additionalFormat = strtok(NULL, " ");

    // input exception handle
    // test case: addORDER P0001 2024-06-10 2000 Product_A --> correct
    // addORDER P0001 2024-06-10 2000a Product_A --> quantity format error
    // addORDER P0001 2024-06-10a 2000 Product_A --> date format error
    char *endptr;
    strtol(quantity, &endptr, 10);
    if (*endptr != '\0') {  // check quantity is an integer
        printf("Error 200 : quantity is not integer\n");
        return;
    } else if (orderNumber == NULL || dueDate == NULL || quantity == NULL || productName == NULL ||
               strcmp(productName, "\n") == 0) {
        printf("Error 201 : Your command format is invalid: Correct format 'addORDER P0001 2024-06-10 2000 Product_A'\n");
        return;
    } else if (!date_is_valid(dueDate)) {   // date format
        printf("Error 202 : Your Date format is invalid: Correct format '2024-06-30'\n");
        return;
    } else if (additionalFormat != 0) {  // command format
        printf("Error 203 : Your command format is invalid: Correct format 'addORDER P0001 2024-06-10 2000 Product_A'\n");
        return;
    }
    int quantity_int = atoi(quantity);

    struct Order *order = malloc(sizeof(struct Order));
    setOrderValues(order, orderNumber, dueDate, quantity_int, productName);

//    if (order_list == NULL)
//        createNode(order);
//    else
    appendNode(order_list, order);
}

void removeNewline(char *str) {
    char *newlinePos;
    while ((newlinePos = strchr(str, '\n')) != NULL)
        *newlinePos = '\0';
}


void addBATCH(char input_command[50], Node **order_list) {
    strtok(input_command, " ");
    char *fileName = strtok(NULL, " ");
    char *additionalFormat = strtok(NULL, " ");
    // addBATCH orderBATCH01.dat
    if (additionalFormat != 0) {  // command format
        printf("Your command format is invalid: Correct format 'addBATCH [Orders in a batch file]'\n");
        return;
    }
    removeNewline(fileName);

    FILE *file = fopen(fileName, "r");
    if (file == NULL) {
        printf("File not found!\n");
        return;
    }

    char line[100];
    while (fgets(line, sizeof(line), file) != NULL) {
        addORDER(line, order_list);
    }
    fclose(file);
}

void runPLS(char input_command[50], Node **order_list, char start_date[11], char end_date[11], struct Plant plants[3]) {
    // runPLS FCFS | printREPORT > report_01_FCFS.txt

    char *currentAlgorithm[] = {"FCFS", "PR", "SJF"};

    strtok(input_command, " ");
    char *algorithm = NULL, *pipeOperator = NULL, *printREPORT = NULL, *greaterSign = NULL, *outputReport = NULL, *additionalFormat;
    algorithm = strtok(NULL, " ");
    pipeOperator = strtok(NULL, " ");
    printREPORT = strtok(NULL, " ");
    greaterSign = strtok(NULL, " ");
    outputReport = strtok(NULL, " ");
    additionalFormat = strtok(NULL, " ");

    // runPLS FCFS | printREPORT > report_01_FCFS.txt
    // Input exception handle
    // error handle test case: 'runPLS FCFS | printREPORT > ', 'runPLS FCFS Z printREPORT > report_01_FCFS.txt',
    // , 'runPLS FCFS | printREPORT ? report_01_FCFS.txt', 'runPLS FCFS Z printEXcel > report_01_FCFS.txt'
    //  printf("\ntest: %s %s %s %s outputReport:|%s|\n", algorithm, pipeOperator, printREPORT, greaterSign, outputReport);
    if (algorithm == NULL || pipeOperator == NULL || printREPORT == NULL || greaterSign == NULL ||
        outputReport == NULL || strcmp(outputReport, "\n") == 0) {
        printf("Error 400 : Your command format is invalid : Correct format 'runPLS [Algorithm] | printREPORT > [Report file name]'\n");
        return;
    } else if (strcmp(pipeOperator, "|") != 0) { //  if pipeOperator != "|"
        printf("Error 401 : Your command format is invalid : Correct format 'runPLS [Algorithm] | printREPORT > [Report file name]'\n");
        return;
    } else if (strcmp(printREPORT, "printREPORT") != 0) { //  if printREPORT != "printREPORT"
        printf("Error 402 : Your command format is invalid : Correct format 'runPLS [Algorithm] | printREPORT > [Report file name]'\n");
        return;
    } else if (strcmp(greaterSign, ">") != 0) { //  if greaterSign != ">"
        printf("Error 403 : Your command format is invalid : Correct format 'runPLS [Algorithm] | printREPORT > [Report file name]'\n");
        return;
    } else if (additionalFormat != 0) {  // command format
        printf("Error 404 : Your command format is invalid : Correct format 'runPLS [Algorithm] | printREPORT > [Report file name]'\n");
        return;
    }

    int period_day = calculateDaysBetweenDate(start_date, end_date) + 1;
//    printf("current period day: %d\n", period_day);
    // create channel for parent to child and child to parent

    FCFSalgo(order_list, plants, period_day);

//    if (strcmp(algorithm, "FCFS") == 0) // if user input 'FCFS' algorithm
//        FCFSalgo(order_list, &plants, period_day);
//    else if (strcmp(algorithm, "PR") == 0)
//        printf("PR algo TBC");
//    else if (strcmp(algorithm, "SJF") == 0)
//        printf("SJF algo TBC");
//    else
//        printf("input algorithm not exist");
//    print report

//    printf("\nPlant_X (%d per day)\n", plants[0].productiveForces);
//    printf("%s to %s\n\n", start_date, end_date);
//    printf("%-15s| %-15s| %-15s| %-15s| %-15s|\n", "Date", "Product Name", "Order Number", "Quantity", "Due Date");
//    int x;
//    char* date_table = start_date;
//    for(x=0;x<period_day;x++)
//    {
//        printf("%-15s| %-15s| %-15s| %-15d| %-15s|\n", date_table, "Product_A", "P1001", 300, "1999-11-11");
//        addOneDay(date_table);
//    }
//    printFirstOrderItem(&plants[1]);
}

void printDATA(char start_date[11], char end_date[11], Node *order_list) {
    if (start_date[0] == '\0')
        printf("Period: NULL\n");
    else
        printf("Period: %s to %s\n", start_date, end_date);
    printOrderList(order_list);
    printf("End\n");
}

int main() {
    struct Plant plants[3];
    setPlantAttributes(&plants[0], "Plant_X", 300);
    setPlantAttributes(&plants[1], "Plant_Y", 400);
    setPlantAttributes(&plants[2], "Plant_Z", 500);

    char start_date[11] = "", end_date[11] = "";
    start_date[10] = '\0', end_date[10] = '\0';
    Node *order_list = NULL;

    printf("\n   ~~WELCOME TO PLS~~\n\n");
    while (1) {
        char input_command[50] = "";
        printf("Please enter:\n> ");
        fgets(input_command, sizeof(input_command), stdin);

        if (strncmp(input_command, "test", 4) == 0) {   // for testing only !!!
            strcpy(input_command, "addPERIOD 2024-06-01 2024-06-30");
            addPERIOD(input_command, &start_date, &end_date);
            strcpy(input_command, "addBATCH orderBATCH01.dat");
            addBATCH(input_command, &order_list);
//            printDATA(start_date, end_date, order_list);
            strcpy(input_command, "runPLS FCFS | printREPORT > report_01_FCFS.txt");
            runPLS(input_command, &order_list, start_date, end_date, plants);
        } else if (strncmp(input_command, "addPERIOD", 9) == 0) {
            addPERIOD(input_command, &start_date, &end_date); // addPERIOD 2024-06-01 2024-06-30
        } else if (strncmp(input_command, "addORDER", 8) == 0) {
            addORDER(input_command, &order_list); // addORDER P0001 2024-06-10 2000 Product_A
        } else if (strncmp(input_command, "addBATCH", 8) == 0) {
            addBATCH(input_command, &order_list); // addBATCH orderBATCH01.dat
        } else if (strncmp(input_command, "runPLS", 6) == 0) {
            runPLS(input_command, &order_list, start_date, end_date,
                   plants); // runPLS FCFS | printREPORT > report_01_FCFS.txt
        } else if (strncmp(input_command, "exitPLS", 7) == 0) {
            printf("Bye-bye!\n"); // exitPLS
            exit(0);
        } else if (strncmp(input_command, "printDATA", 9) == 0) {
            printDATA(start_date, end_date, order_list); // printDATA
        } else {
            printf("Command not found\n");
        }
    }
}