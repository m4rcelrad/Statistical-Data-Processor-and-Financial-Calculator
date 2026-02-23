#include "dataframe.h"
#include "unity/unity.h"

void test_CreateDataFrame(void) {
    DataFrame *df = create_dataframe(3, 2);

    TEST_ASSERT_NOT_NULL(df);
    TEST_ASSERT_EQUAL_INT(3, df->rows);
    TEST_ASSERT_EQUAL_INT(2, df->cols);
    TEST_ASSERT_NOT_NULL(df->data);
    TEST_ASSERT_NOT_NULL(df->columns);
    TEST_ASSERT_NOT_NULL(df->col_types);

    TEST_ASSERT_EQUAL_DOUBLE(0.0, df->data[0][0].v_num);

    free_dataframe(df);
}

void run_dataframe_tests(void) {
    RUN_TEST(test_CreateDataFrame);
}
