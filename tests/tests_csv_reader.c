#include "unity/unity.h"
#include "dataframe.h"
#include "csv_reader.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define TEST_CSV_FILE "test_data.tmp.csv"

/**
 * @file tests_csv_reader.c
 * @brief Unit tests for the CSV file parsing logic.
 *
 * Validates file handling, header processing, delimiter parsing,
 * empty cell management, and automated type deduction.
 */

/**
 * @brief Helper function to quickly create a temporary CSV file.
 * @param content The string payload to be saved inside the file.
 */
static void create_temp_csv(const char *content) {
    FILE *f = fopen(TEST_CSV_FILE, "w");
    if (f) {
        fputs(content, f);
        fclose(f);
    }
}

/**
 * @brief Helper function to delete the temporary CSV file.
 */
static void clean_temp_file(void) {
    remove(TEST_CSV_FILE);
}

/**
 * @brief Tests loading a properly formatted CSV that includes a header row.
 * Expected result: Data is parsed correctly and column names match the first row.
 */
void test_LoadCsv_WithHeader(void) {
    create_temp_csv("Cena;Ilosc\n10.5;5\n20.0;2\n");

    DataFrame *df = NULL;
    const DataframeErrorCode err = read_csv(TEST_CSV_FILE, true, ";", &df);

    clean_temp_file();

    TEST_ASSERT_EQUAL(DATAFRAME_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(df);
    TEST_ASSERT_EQUAL_INT(2, df->rows);
    TEST_ASSERT_EQUAL_INT(2, df->cols);

    TEST_ASSERT_EQUAL_STRING("Cena", df->columns[0]);
    TEST_ASSERT_EQUAL_STRING("Ilosc", df->columns[1]);

    TEST_ASSERT_EQUAL_DOUBLE(10.5, df->data[0][0].v_num);
    TEST_ASSERT_EQUAL_DOUBLE(5.0,  df->data[0][1].v_num);

    TEST_ASSERT_EQUAL_DOUBLE(20.0, df->data[1][0].v_num);
    TEST_ASSERT_EQUAL_DOUBLE(2.0,  df->data[1][1].v_num);

    free_dataframe(df); // Fix: Memory leak prevented
}

/**
 * @brief Tests loading a CSV missing a header row.
 * Expected result: Data is treated purely as rows, and default column headers are auto-generated.
 */
void test_LoadCsv_NoHeader(void) {
    create_temp_csv("10.5;5\n20.0;2\n");

    DataFrame *df = NULL;
    const DataframeErrorCode err = read_csv(TEST_CSV_FILE, false, ";", &df);

    clean_temp_file();

    TEST_ASSERT_EQUAL(DATAFRAME_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(df);
    TEST_ASSERT_EQUAL_INT(2, df->rows);
    TEST_ASSERT_EQUAL_INT(2, df->cols);

    TEST_ASSERT_EQUAL_STRING("col_1", df->columns[0]);
    TEST_ASSERT_EQUAL_STRING("col_2", df->columns[1]);

    TEST_ASSERT_EQUAL_DOUBLE(10.5, df->data[0][0].v_num);
    TEST_ASSERT_EQUAL_DOUBLE(5.0,  df->data[0][1].v_num);

    TEST_ASSERT_EQUAL_DOUBLE(20.0, df->data[1][0].v_num);
    TEST_ASSERT_EQUAL_DOUBLE(2.0,  df->data[1][1].v_num);

    free_dataframe(df);
}

/**
 * @brief Tests the module's reaction to a non-existent file path.
 * Expected result: Returns DATAFRAME_ERR_FILE_NOT_FOUND instead of crashing.
 */
void test_LoadCsv_FileNotFound(void) {
    remove(TEST_CSV_FILE);

    DataFrame *df = NULL;
    const DataframeErrorCode err = read_csv(TEST_CSV_FILE, true, ";", &df);

    TEST_ASSERT_EQUAL(DATAFRAME_ERR_FILE_NOT_FOUND, err);
    TEST_ASSERT_NULL(df);
}

/**
 * @brief Tests parsing an entirely empty file.
 * Expected result: Detects lack of data and returns DATAFRAME_ERR_EMPTY_FILE.
 */
void test_LoadCsv_EmptyFile(void) {
    create_temp_csv("");

    DataFrame *df = NULL;
    const DataframeErrorCode err = read_csv(TEST_CSV_FILE, true, ";", &df);

    clean_temp_file();

    TEST_ASSERT_EQUAL(DATAFRAME_ERR_EMPTY_FILE, err);
    TEST_ASSERT_NULL(df);
}

/**
 * @brief Tests files with inconsistent column counts per row.
 * Expected result: Validation kicks in and returns DATAFRAME_ERR_COLUMN_MISMATCH.
 */
void test_LoadCsv_ColumnMismatch(void) {
    create_temp_csv("10.5;5\n20.0;2;1\n");

    DataFrame *df = NULL;
    const DataframeErrorCode err = read_csv(TEST_CSV_FILE, false, ";", &df);

    clean_temp_file();

    TEST_ASSERT_EQUAL(DATAFRAME_ERR_COLUMN_MISMATCH, err);
    TEST_ASSERT_NULL(df);
}

/**
 * @brief Tests handling of empty CSV cells (e.g., adjacent delimiters).
 * Expected result: Missing numeric values are correctly interpreted as NaN.
 */
void test_LoadCsv_MissingValues(void) {
    create_temp_csv("10.5;;2\n20.0;;\n");

    DataFrame *df = NULL;
    const DataframeErrorCode err = read_csv(TEST_CSV_FILE, false, ";", &df);

    clean_temp_file();

    TEST_ASSERT_EQUAL(DATAFRAME_SUCCESS, err);
    TEST_ASSERT_EQUAL_DOUBLE(20.0, df->data[1][0].v_num);
    TEST_ASSERT_TRUE(isnan(df->data[1][1].v_num));

    free_dataframe(df);
}

/**
 * @brief Tests automatic type deduction based on the first data row.
 * Expected result: String columns are marked as TYPE_STRING, and numbers as TYPE_NUMERIC.
 */
void test_LoadCsv_MixedTypes(void) {
    create_temp_csv("Date;Ticker;Price\n2023-01-01;AAPL;150.5\n2023-01-02;MSFT;\n");

    DataFrame *df = NULL;
    const DataframeErrorCode err = read_csv(TEST_CSV_FILE, true, ";", &df);

    clean_temp_file();

    TEST_ASSERT_EQUAL(DATAFRAME_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(df);

    TEST_ASSERT_EQUAL_INT(TYPE_STRING, df->col_types[0]);
    TEST_ASSERT_EQUAL_INT(TYPE_STRING, df->col_types[1]);
    TEST_ASSERT_EQUAL_INT(TYPE_NUMERIC, df->col_types[2]);

    TEST_ASSERT_EQUAL_STRING("2023-01-01", df->data[0][0].v_str);
    TEST_ASSERT_EQUAL_STRING("AAPL", df->data[0][1].v_str);
    TEST_ASSERT_DOUBLE_WITHIN(0.001, 150.5, df->data[0][2].v_num);
    TEST_ASSERT_TRUE(isnan(df->data[1][2].v_num));

    free_dataframe(df);
}

/**
 * @brief Test runner for the CSV reader module.
 */
void run_csv_reader_tests(void) {
    RUN_TEST(test_LoadCsv_WithHeader);
    RUN_TEST(test_LoadCsv_NoHeader);
    RUN_TEST(test_LoadCsv_FileNotFound);
    RUN_TEST(test_LoadCsv_EmptyFile);
    RUN_TEST(test_LoadCsv_ColumnMismatch);
    RUN_TEST(test_LoadCsv_MissingValues);
    RUN_TEST(test_LoadCsv_MixedTypes);
}