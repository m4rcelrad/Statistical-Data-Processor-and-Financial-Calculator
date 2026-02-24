#include <stddef.h>

#include "dataframe.h"
#include "unity/unity.h"

/**
 * @file tests_dataframe.c
 * @brief Unit tests for the DataFrame core structure.
 *
 * Verifies the allocation, initialization, and safe dimension handling
 * of the custom 2D data representation.
 */

/**
 * @brief Tests successful creation of a valid DataFrame.
 * Expected result: Dimensions are correctly set, all inner arrays are
 * allocated, and memory is zero-initialized.
 */
void test_CreateDataFrame_Valid(void)
{
    DataFrame *df = create_dataframe((size_t)3, (size_t)2);

    TEST_ASSERT_NOT_NULL(df);
    TEST_ASSERT_EQUAL_INT(3, df->rows);
    TEST_ASSERT_EQUAL_INT(2, df->cols);
    TEST_ASSERT_NOT_NULL(df->data);
    TEST_ASSERT_NOT_NULL(df->columns);
    TEST_ASSERT_NOT_NULL(df->col_types);

    TEST_ASSERT_EQUAL_DOUBLE(0.0, df->data[0][0].v_num);

    free_dataframe(df);
}

/**
 * @brief Tests the creation function with invalid dimensions (zero rows or columns).
 * Expected result: The allocator fails safely and returns a NULL pointer.
 */
void test_CreateDataFrame_InvalidDimensions(void)
{
    DataFrame *df1 = create_dataframe((size_t)0, (size_t)5);
    TEST_ASSERT_NULL(df1);

    DataFrame *df2 = create_dataframe((size_t)5, (size_t)0);
    TEST_ASSERT_NULL(df2);
}

/**
 * @brief Test runner for the dataframe module.
 */
void run_dataframe_tests(void)
{
    RUN_TEST(test_CreateDataFrame_Valid);
    RUN_TEST(test_CreateDataFrame_InvalidDimensions);
}
