#include "object/plant.c"
#include "object/order.c"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define size 3
#define string_length 10
bool date_is_valid(const char *buffer) {
  if (buffer == NULL) {
    return false;
  }

  const int len = strlen(buffer);

  if (len != 10) {
    return false;
  }

  int i;
  for (i = 0; i < len; i++) {
    if (i == 4 || i == 7) {
      if (buffer[i] != '-') {
        return false;
      }
    } else {
      if (!isdigit((unsigned char)buffer[i])) {
        return false;
      }
    }
  }

  int yy, mm, dd;
  yy = strtol(buffer, NULL, 10);
  mm = strtol(buffer + 5, NULL, 10);
  dd = strtol(buffer + 8, NULL, 10);

  if (yy >= 1900 && yy <= 9999) {
    if (mm >= 1 && mm <= 12) {
      if ((dd >= 1 && dd <= 31) && (mm == 1 || mm == 3 || mm == 5 || mm == 7 ||
                                    mm == 8 || mm == 10 || mm == 12))
        return true;
      else if ((dd >= 1 && dd <= 30) &&
               (mm == 4 || mm == 6 || mm == 9 || mm == 11))
        return true;
      else if ((dd >= 1 && dd <= 28) && (mm == 2))
        return true;
      else if (dd == 29 && mm == 2 &&
               (yy % 400 == 0 || (yy % 4 == 0 && yy % 100 != 0)))
        return true;
      else
        return false;
    } else {
      return false;
    }
  } else {
    return false;
  }
}

void addPERIOD(char input_command[50], char period_date[size*2][string_length+1], int* period_date_arr_len) {
  printf("addPERIOD command \n");
  // 0123456789012345678012345678901
  // addPERIOD 2024-06-01 2024-06-30
  char start_date[11] = "", end_date[11] = "";
  start_date[10] = '\0', end_date[10] = '\0';

  strncpy(start_date, input_command + 10, 10);
  strncpy(end_date, input_command + 21, 10);

  if (date_is_valid(start_date) && date_is_valid(end_date)) {
      strcpy(period_date[(*period_date_arr_len)++], start_date);
      strcpy(period_date[(*period_date_arr_len)++], end_date);
  } else {
    printf("Your Date Format is invalid, please refer correct format is YYYY-MM-DD ");
  }
    for (int i = 0; i < 10; i++) {
        printf("string %s ", period_date[i] );
    }

}

int main() {
  struct Plant plant[3];
  setPlantAttributes(&plant[0], "Plant_X", 300);
  setPlantAttributes(&plant[1], "Plant_Y", 400);
  setPlantAttributes(&plant[2], "Plant_Z", 500);

  printf("\n   ~~WELCOME TO PLS~~\n\n");

  char period_date[size*2][string_length+1];
  int period_date_arr_len = 0;
  memset(period_date, 0, sizeof(period_date));

  while (1) {
    char input_command[50] = "";
    printf("Please enter:\n> ");
    fgets(input_command, sizeof(input_command), stdin);

    if (strncmp(input_command, "addPERIOD", 9) == 0) {
      // addPEIOD 2024-06-01 2024-06-30
      addPERIOD(input_command, period_date, &period_date_arr_len);
    } else if (strncmp(input_command, "addORDER", 8) == 0) {
      printf("addORDER command \n");
    } else if (strncmp(input_command, "addBATCH", 8) == 0) {
      printf("addBATCH command \n");
    } else if (strncmp(input_command, "runPLS", 6) == 0) {
      printf("runPLS command \n");
    } else if (strncmp(input_command, "exitPLS", 7) == 0) {
      printf("Bye-bye!\n");
      exit(0);
    } else if (strncmp(input_command, " ", 1) == 0) {
      printf("Command not found\n");
    }
  }
  printf("End");
}