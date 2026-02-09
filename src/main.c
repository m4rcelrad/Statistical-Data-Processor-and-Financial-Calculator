#include "finance.h"

int main(void) {
    calculate_and_print_schedule(100000, 0.05, 12, LOAN_EQUAL_INSTALLMENTS);

    long double payments[12] = {0};
    payments[2] = 10000.00;

    calculate_and_print_dynamic_schedule(100000, 0.05, 12, LOAN_EQUAL_INSTALLMENTS, payments);
}
