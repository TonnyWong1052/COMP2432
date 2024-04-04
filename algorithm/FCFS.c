#include <stdio.h>
#include <stdlib.h>
#include "../data_structure/LinkedList.h"
#include "../object/order.h"
#include <unistd.h>
#include <stdbool.h>

// plant_X, plant_Y, plant_Z
#define plantCount 3

void closeChannel(int *channel[4], int index) {
    int x;
    for (x = 0; x < 4; x++)
        close((channel[x])[1]);
}

void childProcess(int pc[2], int cp[2], struct Plant *plant, int period_day) {
//    close(pc[1]);
//    close(cp[0]);
    bool list_have_order = true;
    int plantProductiveForces = plant->productiveForces;
    int remainAvabiliableDay = period_day;
    while (true) {
        read(pc[0], &list_have_order, sizeof(list_have_order));
        if (!list_have_order) {
            break;
        }

        struct Order order;
        // Read the order structure directly.
        read(pc[0], &order, sizeof(struct Order));
        int expected_day_production = (order.quantity + plantProductiveForces / 2) / plantProductiveForces;

        int is_receive_order;
        if(remainAvabiliableDay - expected_day_production != -1) {
            remainAvabiliableDay = remainAvabiliableDay - expected_day_production;
            is_receive_order = 1;   // plant have ability to receive the order
        }else{
            is_receive_order = 0;   // plant reject the order
        }
        printf("plant name %s , day production : %d, order qty: %d, ability: %d \n", plant->name,
               expected_day_production, order.quantity, plantProductiveForces);
//        printf("plant %s, qty: %d received order: %s\n", plant->name, order.quantity, order.productName);

        if (is_receive_order == 1)
            addOrder(plant, &order);
        write(cp[1], &is_receive_order, sizeof(is_receive_order));
    }
    int q = 100;
    write(cp[1], &plant, sizeof(struct Plant));
//    write(cp[1], &q, sizeof(q));
//    printFirstOrderItem(plant);

    close(pc[0]);
    close(cp[1]);
    exit(0);
}

void processComput(int pc[2], int cp[2], struct Plant *plant, int period_day) {
    int child_id = fork();
    if (child_id == 0) {
        childProcess(pc, cp, plant, period_day);
    }
}

void FCFSalgo(Node **order_list, struct Plant plants[3], int period_day) {
    int pcX[2], pcY[2], pcZ[2];
    int cpX[2], cpY[2], cpZ[2];
    int *pc[] = {pcX, pcY, pcZ};
    int *cp[] = {cpX, cpY, cpZ};

    for (int x = 0; x < plantCount; x++) {
        pipe(pc[x]);
        pipe(cp[x]);
        processComput(pc[x], cp[x], &plants[x], period_day);
    }

//    closeChannel(pc, 1);
//    closeChannel(cp, 0);

    int round = 0;
    bool list_have_order = true;
    while (true) {
        list_have_order = get_size(*order_list) > 0;
        if (list_have_order == true)
            write((pc[round % plantCount])[1], &list_have_order, sizeof(list_have_order));
        else {
            // inform all child process, shut down the while loop
            write((pc[0])[1], &list_have_order, sizeof(list_have_order));
            write((pc[1])[1], &list_have_order, sizeof(list_have_order));
            write((pc[2])[1], &list_have_order, sizeof(list_have_order));
            break;
        }

        int is_receive_order;
        struct Order *order = get_first(*order_list);
//        printf("round: %d Send order: %s", round % plantCount, order->productName);
        // Send the order to plant (child process)
        write((pc[round % plantCount])[1], order, sizeof(struct Order));

        read((cp[round % plantCount])[0], &is_receive_order, sizeof(is_receive_order));
        if (is_receive_order)
            delete_begin(order_list);

        round++;
    }
//    read((cp[0])[0], &plants[0], sizeof(struct Plant));
//    read((cp[1])[0], &plants[1], sizeof(struct Plant));
//    read((cp[2])[0], &plants[2], sizeof(struct Plant));
    for (int x = 0; x < plantCount; x++) {
//        int q;
//        read((cp[x])[0], &q, sizeof(q));
//        printf("test %d", q);
        struct Plant updatedPlant;
        read((cp[x])[0], &updatedPlant, sizeof(struct Plant));
        plants[x] = updatedPlant; // Copy the updated data back to the original array element
    }


//    struct Order order_temp = *((struct Order*)get_first(plants[0].myOrder));
//    printf("my name is %s %d %s", plants[0].name, plants[0].productiveForces, "123");

//    char* orderNumber = ((struct Order*)temp_data)->orderNumber;

//    printFirstOrderItem(&plants[0]);
//    printf("Final size = %d", get_size(plants[0].myOrder));
//    printFirstOrderItem(&plants[0]);
    printf("\n");
    int x;
    for (x = 0; x < plantCount; x++)
        wait(NULL);
}
