#include "object/plant.h"
#include "object/order.h"
#include "date.h"
#include "command.c"
#include "LinkedList.h"

int pc[2], cp[2];
int response_message = 1;

void run_command(char input_command[100], char (*start_date)[11], char (*end_date)[11], Node **order_list, struct Plant plants[]) {
    int child_id = fork();
    if (child_id == 0) {
        if (strncmp(input_command, "test", 4) == 0) {
            strcpy(input_command, "addPERIOD 2024-06-01 2024-06-11");
            addPERIOD(input_command, start_date, end_date);
            strcpy(input_command, "addBATCH test_data_G50_FCFS.dat");
            addBATCH(input_command, order_list);
            strcpy(input_command, "runPLS FCFS | printREPORT > report_01_FCFS.txt");
            runPLS(input_command, order_list, start_date, end_date, plants);
        } else if (strncmp(input_command, "addPERIOD", 9) == 0) {
            addPERIOD(input_command, start_date, end_date);
        } else if (strncmp(input_command, "addORDER", 8) == 0) {
            addORDER(input_command, order_list);
        } else if (strncmp(input_command, "addBATCH", 8) == 0) {
            addBATCH(input_command, order_list);
        } else if (strncmp(input_command, "runPLS", 6) == 0) {
            runPLS(input_command, order_list, *start_date, *end_date, plants);
        } else if (strncmp(input_command, "exitPLS", 7) == 0) {
            printf("Bye-bye!\n");
            response_message = -1;
            write(cp[1], &response_message, sizeof(response_message));
            exit(0);
        } else if (strncmp(input_command, "printDATA", 9) == 0) {
            printDATA(*start_date, *end_date, *order_list);
        } else {
            removeNewline(input_command); // remove the newline character if any
            printf("Input command '%s' not found\n", input_command);
        }
        write(cp[1], &response_message, sizeof(response_message));
    }
    // if read
    int exit_message;
    read(cp[0], &exit_message, sizeof(exit_message));
    if(exit_message == -1)
        exit(0);
}

int main() {
    struct Plant plants[3];
    setPlantAttributes(&plants[0], 0, "Plant_X", 300);
    setPlantAttributes(&plants[1], 1, "Plant_Y", 400);
    setPlantAttributes(&plants[2], 2, "Plant_Z", 500);

    char start_date[11] = "", end_date[11] = "";
    start_date[10] = '\0', end_date[10] = '\0';
    Node *order_list = NULL;
    pipe(pc);
    pipe(cp);
    printf("\n   ~~WELCOME TO PLS~~\n\n");
    while (true) {
        char input_command[100] = "";
        printf("Please enter:\n> ");
        fgets(input_command, sizeof(input_command), stdin);
        run_command(input_command, &start_date, &end_date, &order_list, plants);
        wait(NULL);
    }
}