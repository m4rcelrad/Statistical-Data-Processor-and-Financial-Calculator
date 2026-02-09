#include <stdio.h>

#include "finance.h"

int main(void) {
    printf("Hello, World!\n");
    calculate_and_print_schedule(100000, 0.05, 12, LOAN_EQUAL_INSTALLMENTS);
}
