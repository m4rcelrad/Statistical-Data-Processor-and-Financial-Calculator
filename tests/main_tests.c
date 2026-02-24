#include "unity.h"

/**
 * @file main_tests.c
 * @brief Central entry point for executing all unit test suites.
 *
 * This file gathers all modular test runners and executes them inside
 * the Unity testing framework boundaries.
 */

extern void run_money_tests(void);
extern void run_loan_math_tests(void);
extern void run_loan_simulation_tests(void);
extern void run_dataframe_tests(void);
extern void run_statistics_tests(void);
extern void run_memory_utils_tests(void);

/**
 * @brief Unity required function executed before each test.
 */
void setUp(void) {}

/**
 * @brief Unity required function executed after each test.
 */
void tearDown(void) {}

/**
 * @brief The main runner initializing and terminating the test suites.
 * @return 0 on test success, or an error code on failure.
 */
int main(void) {
  UNITY_BEGIN();

  run_money_tests();
  run_loan_math_tests();
  run_loan_simulation_tests();
  run_dataframe_tests();
  run_statistics_tests();

  return UNITY_END();
}