#include "../date.h"
#include "../object/plant.h"

void printPlantSchedule(struct Plant* plant, int period_day, char* start_date, char* end_date){
        printf("\n%s (%d per day)\n", plant->name, plant->productiveForces);
    printf("%s to %s\n\n", start_date, end_date);
    printf("%-15s| %-15s| %-15s| %-15s| %-15s|\n", "Date", "Product Name", "Order Number", "Quantity", "Due Date");
    int x;
    char* date_table = start_date;
    int order_size = get_size(plant->myOrder);
//    get_first(plant->myOrder);
//    delete_begin(&plant->myOrder);
//    printOrderList((Node *) &plant);
//    printf("End");
//    printFirstOrderItem(plant);
    printf("Order Size: %d", order_size);
    for(x=0;x<period_day;x++)
    {
        if(x < order_size){
//            void* temp_data = get_first(plant->myOrder);
//            struct Order* order = (struct Order*)get_first(plant->myOrder);
            struct Order* order = (struct Order*)get_first(plant->myOrder);
            printf("\nThe first order in plant %s is %s. index: %d, name: %s \n", plant->name, order->orderNumber, x, order->productName);

//            char* orderNumber = ((struct Order*)temp_data)->orderNumber;
//            char* dueDate = ((struct Order*)temp_data)->dueDate;
//            int quantity = ((struct Order*)temp_data)->quantity;
//            productName = ((struct Order*)temp_data)->productName;

//            order = *(struct Order*)get_first(plant->myOrder->data);
//            productName = ((struct Order*)temp_data)->productName;
//            printf("product name: %s\n", productName);
            delete_begin(&plant->myOrder);
        }
//        printf("%-15s| %-15s| %-15s| %-15d| %-15s|\n", date_table, productName, order.orderNumber, order.quantity, "1999-11-11");
        addOneDay(date_table);
    }
    printf("finish");
}