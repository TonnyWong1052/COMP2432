#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>

#ifndef DATE_H
#define DATE_H

void closeChannel(int *channel[3], int index) {
    int x;
    for (x = 0; x < 3; x++)
        close((channel[x])[index]);
}

bool isDateLater(const char *date1, const char *date2) {
    struct tm tm1, tm2;
    time_t t1, t2;

    memset(&tm1, 0, sizeof(struct tm));
    memset(&tm2, 0, sizeof(struct tm));

    strptime(date1, "%Y-%m-%d", &tm1);
    strptime(date2, "%Y-%m-%d", &tm2);

    t1 = mktime(&tm1);
    t2 = mktime(&tm2);

    return difftime(t1, t2) > 0;
}


void removeNewline(char *str) {
    char *newlinePos;
    while ((newlinePos = strchr(str, '\n')) != NULL)
        *newlinePos = '\0';
}

int isLeapYear(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

int getDaysInMonth(int month, int year) {
    int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month == 2 && isLeapYear(year)) {
        return 29;
    }
    return daysInMonth[month - 1];
}

void addOneDay(char *date) {    // input 2024-02-29 --> output 2024-03-01
    int year, month, day;
    sscanf(date, "%d-%d-%d", &year, &month, &day);

    day++;
    if (day > getDaysInMonth(month, year)) {
        day = 1;
        month++;
        if (month > 12) {
            month = 1;
            year++;
        }
    }

    sprintf(date, "%d-%02d-%02d", year, month, day);
}

void addDays(char *date, int daysToAdd, char *outputDate) {
    int year, month, day;
    sscanf(date, "%d-%d-%d", &year, &month, &day);
    int i;
    for (i = 0; i < daysToAdd; i++) {
        day++;
        if (day > getDaysInMonth(month, year)) {
            day = 1;
            month++;
            if (month > 12) {
                month = 1;
                year++;
            }
        }
    }

    sprintf(outputDate, "%d-%02d-%02d", year, month, day);
}


int calculateDaysBetweenDate(const char* startDateStr, const char* endDateStr) {
    struct tm startDate = {0};
    struct tm endDate = {0};

    strptime(startDateStr, "%Y-%m-%d", &startDate);
    time_t start = mktime(&startDate);

    strptime(endDateStr, "%Y-%m-%d", &endDate);
    time_t end = mktime(&endDate);
    double diffInSeconds = difftime(end, start);
    int diffInDays = diffInSeconds / (60 * 60 * 24);
    return diffInDays;
}

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
            if (!isdigit((unsigned char) buffer[i])) {
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

#endif