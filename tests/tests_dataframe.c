#include "unity/unity.h"
#include "dataframe.h"
#include <stdio.h>
#include <stdlib.h>

#define TEST_CSV_FILE "test_data.tmp.csv"

static void create_temp_csv(const char *content) {
    FILE *f = fopen(TEST_CSV_FILE, "w");
    if (f) {
        fputs(content, f);
        fclose(f);
    }
}

static void clean_temp_file(void) {
    remove(TEST_CSV_FILE);
}

void test_CreateDataFrame(void) {
    DataFrame *df = create_dataframe(3, 2);

    TEST_ASSERT_NOT_NULL(df);
    TEST_ASSERT_EQUAL_INT(3, df->rows);
    TEST_ASSERT_EQUAL_INT(2, df->cols);
    TEST_ASSERT_NOT_NULL(df->data);
    TEST_ASSERT_NOT_NULL(df->columns);

    TEST_ASSERT_EQUAL_DOUBLE(0.0, df->data[0][0]);

    free_dataframe(df);
}

void test_LoadCsv_WithHeader(void) {
    create_temp_csv("Cena;Ilosc\n10.5;5\n20.0;2\n");

    DataFrame *df = NULL;
    const DataframeErrorCode err = read_csv(TEST_CSV_FILE, true, ";", &df);

    TEST_ASSERT_EQUAL(DATAFRAME_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(df);
    TEST_ASSERT_EQUAL_INT(2, df->rows);
    TEST_ASSERT_EQUAL_INT(2, df->cols);

    TEST_ASSERT_EQUAL_STRING("Cena", df->columns[0]);
    TEST_ASSERT_EQUAL_STRING("Ilosc", df->columns[1]);

    TEST_ASSERT_EQUAL_DOUBLE(10.5, df->data[0][0]);
    TEST_ASSERT_EQUAL_DOUBLE(5.0,  df->data[0][1]);

    TEST_ASSERT_EQUAL_DOUBLE(20.0, df->data[1][0]);
    TEST_ASSERT_EQUAL_DOUBLE(2.0,  df->data[1][1]);

    free_dataframe(df);
    clean_temp_file();
}

void test_LoadCsv_NoHeader(void) {
    create_temp_csv("10.5;5\n20.0;2\n");

    DataFrame *df = NULL;
    const DataframeErrorCode err = read_csv(TEST_CSV_FILE, false, ";", &df);

    TEST_ASSERT_EQUAL(DATAFRAME_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(df);
    TEST_ASSERT_EQUAL_INT(2, df->rows);
    TEST_ASSERT_EQUAL_INT(2, df->cols);

    TEST_ASSERT_EQUAL_STRING("col_1", df->columns[0]);
    TEST_ASSERT_EQUAL_STRING("col_2", df->columns[1]);

    TEST_ASSERT_EQUAL_DOUBLE(10.5, df->data[0][0]);
    TEST_ASSERT_EQUAL_DOUBLE(5.0,  df->data[0][1]);

    TEST_ASSERT_EQUAL_DOUBLE(20.0, df->data[1][0]);
    TEST_ASSERT_EQUAL_DOUBLE(2.0,  df->data[1][1]);

    free_dataframe(df);
    clean_temp_file();
}

void test_LoadCsv_FileNotFound(void) {
    remove(TEST_CSV_FILE);

    DataFrame *df = NULL;
    const DataframeErrorCode err = read_csv(TEST_CSV_FILE, true, ";", &df);

    TEST_ASSERT_EQUAL(DATAFRAME_ERR_FILE_NOT_FOUND, err);
    TEST_ASSERT_NULL(df);

    free_dataframe(df);
}

void test_LoadCsv_EmptyFile(void) {
    create_temp_csv("");

    DataFrame *df = NULL;
    const DataframeErrorCode err = read_csv(TEST_CSV_FILE, true, ";", &df);

    TEST_ASSERT_EQUAL(DATAFRAME_ERR_EMPTY_FILE, err);
    TEST_ASSERT_NULL(df);

    free_dataframe(df);
    clean_temp_file();
}

void test_LoadCsv_ColumnMismatch(void) {
    create_temp_csv("10.5;5\n20.0;2;1\n");

    DataFrame *df = NULL;
    const DataframeErrorCode err = read_csv(TEST_CSV_FILE, false, ";", &df);

    TEST_ASSERT_EQUAL(DATAFRAME_ERR_COLUMN_MISMATCH, err);
    TEST_ASSERT_NULL(df);

    free_dataframe(df);
    clean_temp_file();
}

void test_LoadCsv_MissingValues(void) {
    create_temp_csv("10.5;;2\n20.0;;\n");

    DataFrame *df = NULL;
    const DataframeErrorCode err = read_csv(TEST_CSV_FILE, false, ";", &df);

    TEST_ASSERT_EQUAL(DATAFRAME_SUCCESS, err);
    TEST_ASSERT_EQUAL_DOUBLE(20.0, df->data[1][0]);
    TEST_ASSERT_TRUE(isnan(df->data[1][1]));

    free_dataframe(df);
    clean_temp_file();
}

void run_dataframe_tests(void) {
    RUN_TEST(test_CreateDataFrame);
    RUN_TEST(test_LoadCsv_WithHeader);
    RUN_TEST(test_LoadCsv_NoHeader);
    RUN_TEST(test_LoadCsv_FileNotFound);
    RUN_TEST(test_LoadCsv_EmptyFile);
}