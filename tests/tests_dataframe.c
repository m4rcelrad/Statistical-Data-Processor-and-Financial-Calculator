#include <stddef.h>

#include "dataframe.h"
#include "unity/unity.h"

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

void test_CreateDataFrame_InvalidDimensions(void)
{
    DataFrame *df1 = create_dataframe((size_t)0, (size_t)5);
    TEST_ASSERT_NULL(df1);

    DataFrame *df2 = create_dataframe((size_t)5, (size_t)0);
    TEST_ASSERT_NULL(df2);
}

void run_dataframe_tests(void)
{
    RUN_TEST(test_CreateDataFrame_Valid);
    RUN_TEST(test_CreateDataFrame_InvalidDimensions);
}
