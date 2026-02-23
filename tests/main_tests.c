#include "unity.h"

extern void run_money_tests(void);
extern void run_loan_math_tests(void);
extern void run_loan_simulation_tests(void);
extern void run_dataframe_tests(void);
extern void run_csv_reader_tests(void);

void setUp(void) {}
void tearDown(void) {}

int main(void) {
    UNITY_BEGIN();

    run_money_tests();
    run_loan_math_tests();
    run_loan_simulation_tests();
    run_dataframe_tests();
    run_csv_reader_tests();

    return UNITY_END();
}
