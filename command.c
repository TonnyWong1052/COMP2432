#include "date.h"
#include "algorithm/FCFS.c"
#include "algorithm/PR.c"
#include "algorithm/SJF.c"
#include "algorithm/MTS.c"

void addPERIOD(char input_command[100], char (*start_date)[11], char (*end_date)[11]) {
    // 0123456789012345678012345678901 --> char_index
    // addPERIOD 2024-06-01 2024-06-30 --> use case
    // addPERIOD 2024-06-01a 2024-06-30b --> date format error
    // addPERIOD 2024-06-01 2024-06-01 --> error 003
    // addPERIOD 2024-06-01 2024-05-30 --> error 004

    char temp_start_date[11], temp_send_date[11];
    strncpy(temp_start_date, input_command + 10, 10);
    strncpy(temp_send_date, input_command + 21, 10);
    temp_start_date[10] = '\0', temp_send_date[10] = '\0';

    int isContainAdditionalFormat = strcmp(input_command + 32, "") == 0;

    if (!date_is_valid(temp_start_date) || !date_is_valid(temp_send_date)) {
        printf("ERROR 001: Your date format is invalid: Correct date 'YYYY-MM-DD'\n");
        return;
    } else if(isContainAdditionalFormat == 0) {
        printf("ERROR 002: Your command format is invalid: Correct format 'addPERIOD YYYY-MM-DD YYYY-MM-DD'\n");
        return;
    }

    int diff_day = calculateDaysBetweenDate(temp_start_date, temp_send_date);
    if (diff_day == 0) {
        printf("ERROR 003: Your date format is invalid: start date and end date can not be same\n");
        return;
    }else if (diff_day < 0) {
        printf("ERROR 004: Your date format is invalid: end date can not be later than the start date.\n");
        return;
    }

        strncpy(*start_date, input_command + 10, 10);
    strncpy(*end_date, input_command + 21, 10);
}

bool addORDER(char input_command[100], Node **order_list) {
    char *command = strtok(input_command, " ");
    char *orderNumber = NULL, *dueDate = NULL, *quantity = NULL, *productName = NULL;
    orderNumber = strtok(NULL, " ");
    dueDate = strtok(NULL, " ");
    quantity = strtok(NULL, " ");
    productName = strtok(NULL, " ");
    char *additionalFormat = strtok(NULL, " ");

    // input exception handle
    // test case: addORDER P0001 2024-06-10 2000 Product_A --> correct
    // addORDER P0001 2024-06-10 2000a Product_A --> quantity format error
    // addORDER P0001 2024-06-10 2000 --> ERROR 101
    // addORDER P0001 2024-06-10 d Product_A --> ERROR 102
    // addORDER P0001 2024-06-10ERR 2000 Product_A --> ERROR 103
    // addORDER P0001 2024-06-10 2000 Product_A ERROR --> ERROR 104
    if (strcmp(command, "addORDER") != 0) {
        printf("Error 100 : command not found\n");
        return false;
    }
    if (orderNumber == NULL || dueDate == NULL || quantity == NULL || productName == NULL ||
             strcmp(productName, "\n") == 0) {
        printf("Error 101 : Your command format is invalid: Correct format is 'addORDER [Order Number] [Due Date] [Quantity] [Product Name]' with example like 'addORDER P0001 2024-06-10 2000 Product_A'\n");
        return false;
    }
    char *endptr;
    strtol(quantity, &endptr, 10);
    if (*endptr != '\0') {  // check quantity is an integer
        printf("Error 102 : Quantity is not integer\n");
        return false;
    } else if (!date_is_valid(dueDate)) {   // date format
        printf("Error 103 : Your Date format is invalid: Example format '2024-06-30'\n");
        return false;
    } else if (additionalFormat != 0) {  // command format
        printf("Error 104 : Your command format is invalid: Correct format 'addORDER P0001 2024-06-10 2000 Product_A'\n");
        return false;
    }

    int quantity_int = atoi(quantity);
    struct Order *order = malloc(sizeof(struct Order));
    setOrderValues(order, orderNumber, dueDate, quantity_int, productName);
    addToTail(order_list, order);
    return true;
}

void addBATCH(char input_command[100], Node **order_list) {
    char *command = strtok(input_command, " ");
    char *fileName = strtok(NULL, " ");
    char *additionalFormat = strtok(NULL, " ");
    // addBATCH test_data_G50_FCFS.dat --> correct
    // addBATCH NOT_EXIST_FOUND.dat --> ERROR 202
    // addBATCH test_data_G50_FCFS.dat ERROR --> ERROR 201
    // addBATCH incorrect_batch.dat --> ERROR 203


    if (additionalFormat != 0) {  // command format
        printf("Error 201 : Your command format is invalid: Correct format 'addBATCH [Orders in a batch file]'\n");
        return;
    }
    removeNewline(fileName);
    FILE *file = fopen(fileName, "r");
    if (file == NULL) {
        printf("Error 202 : File not found!\n");
        return;
    }

    // addBATCH incorrect_batch.dat
    char line[100];
    while (fgets(line, sizeof(line), file) != NULL) {
        removeNewline(line);
        char temp[100];
        strcpy(temp, line);
        if(!addORDER(line, order_list))
            printf("Error 203 : batch File inside command format is not correct: '%s'\n", temp);
    }
    fclose(file);
}

void runPLS(char input_command[100], Node **order_list, char start_date[11], char end_date[11], struct Plant plants[3]) {
    // runPLS FCFS | printREPORT > report_01_FCFS.txt
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
    // test case: 'runPLS FCFS | printREPORT > ', 'runPLS FCFS Z printREPORT > report_01_FCFS.txt',
    // 'runPLS FCFS | printREPORT ? report_01_FCFS.txt', 'runPLS FCFS Z printEXcel > report_01_FCFS.txt'
    // 'runPLS FCFS | printREPORT ERROR report_01_FCFS.txt'  --> Error 401
    // PERIOD time is null  --> Error 402 // order list is null  --> Error 403
    // input algorithm is not exist --> Error 404

    if (algorithm == NULL || pipeOperator == NULL || printREPORT == NULL || greaterSign == NULL ||
        outputReport == NULL || strcmp(outputReport, "\n") == 0) {
        printf("Error 401 : Your command format is invalid : Correct format 'runPLS [Algorithm] | printREPORT > [Report file name]'\n");
        return;
    } else if (strcmp(pipeOperator, "|") != 0) { //  if pipeOperator != "|"
        printf("Error 401 : Your command format is invalid : Correct format 'runPLS [Algorithm] | printREPORT > [Report file name]'\n");
        return;
    } else if (strcmp(printREPORT, "printREPORT") != 0) { //  if printREPORT != "printREPORT"
        printf("Error 401 : Your command format is invalid : Correct format 'runPLS [Algorithm] | printREPORT > [Report file name]'\n");
        return;
    } else if (strcmp(greaterSign, ">") != 0) { //  if greaterSign != ">"
        printf("Error 401 : Your command format is invalid : Correct format 'runPLS [Algorithm] | printREPORT > [Report file name]'\n");
        return;
    } else if (additionalFormat != 0) {  // command format
        printf("Error 401 : Your command format is invalid : Correct format 'runPLS [Algorithm] | printREPORT > [Report file name]'\n");
        return;
    } else if (strcmp(start_date, "") == 0 || strcmp(end_date, "") == 0) {   // check period date is exist
        printf("Error 402: Please set period date : Recommend command 'addPERIOD [start date] [end date]'\n");
        return;
    } else if (get_size(*order_list) == 0) {    // check enough order
        printf("Error 403: order list is null, please add order to the list : Recommend command 'addORDER [Order Number] [Due Date] [Quantity] [Product Name]'\n");
        return;
    }

    if (strcmp(algorithm, "FCFS") == 0) // if user input 'FCFS' algorithm
        FCFS(order_list, plants, start_date, end_date, outputReport);
    else if (strcmp(algorithm, "PR") == 0)
        PR(order_list, plants, start_date, end_date, outputReport);
    else if (strcmp(algorithm, "SJF") == 0)
        SJF(order_list, plants, start_date, end_date, outputReport);
    else if (strcmp(algorithm, "MTS") == 0)
        MTS(order_list, plants, start_date, end_date, outputReport);
    else
        printf("Error 404: Input '%s' algorithm not existing\n", algorithm);
//    order_list = malloc(sizeof(struct Order));
}

void printDATA(char start_date[11], char end_date[11], Node *order_list) {
    if (start_date[0] == '\0')
        printf("Date: NULL\n");
    else
        printf("Date: %s to %s", start_date, end_date);

    if (get_size(order_list) == 0)
        printf("Order List: NULL\n");
    else
        printOrderList(order_list);
}