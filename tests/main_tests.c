#include "unity.h"

extern void run_money_tests(void);
extern void run_loan_math_tests(void);

void setUp(void) {}
void tearDown(void) {}

int main(void) {
    UNITY_BEGIN();

    run_money_tests();
    run_loan_math_tests();

    return UNITY_END();
}
