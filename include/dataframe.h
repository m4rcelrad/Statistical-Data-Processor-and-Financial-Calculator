#ifndef STATISTICALDATAPROCESSOR_DATAFRAME_H
#define STATISTICALDATAPROCESSOR_DATAFRAME_H
#include <stddef.h>

#include "typedefs.h"

/**
 * @file dataframe.h
 * @brief Core structures and functions for representing tabular data.
 *
 * This module defines the DataFrame structure, which holds a 2D grid of
 * mixed-type data (numeric and string), along with memory management and
 * visualization utilities.
 */

/**
 * @brief Error codes related to DataFrame operations.
 */
typedef enum {
    DATAFRAME_SUCCESS = 0,           /*!< Operation completed successfully. */
    DATAFRAME_ERR_FILE_NOT_FOUND,    /*!< Specified file could not be opened. */
    DATAFRAME_ERR_EMPTY_FILE,        /*!< The file is empty or contains no readable data. */
    DATAFRAME_ERR_ALLOCATION_FAILED, /*!< Memory allocation failed during creation or parsing. */
    DATAFRAME_ERR_COLUMN_MISMATCH    /*!< Inconsistent number of columns detected in rows. */
} DataframeErrorCode;

/**
 * @brief Data types supported by DataFrame columns.
 */
typedef enum {
    TYPE_NUMERIC, /*!< Column contains numeric values (represented as doubles). */
    TYPE_STRING   /*!< Column contains string values (represented as char pointers). */
} DataType;

/**
 * @brief A single cell in a DataFrame, capable of holding either a number or a string.
 */
typedef union {
    double v_num; /*!< Numeric value representation. */
    char *v_str;  /*!< Dynamically allocated string value representation. */
} DataCell;

/**
 * @brief Structure representing tabular data with named columns and typed data.
 */
typedef struct {
    char **columns;      /*!< Array of dynamically allocated column names. */
    DataType *col_types; /*!< Array of data types corresponding to each column. */
    DataCell **data;     /*!< 2D array of cells storing the actual data [row][col]. */
    int rows;            /*!< Number of rows in the DataFrame. */
    int cols;            /*!< Number of columns in the DataFrame. */
} DataFrame;

/**
 * @brief Allocates and initializes a new DataFrame with the specified dimensions.
 * @param rows The number of rows to allocate.
 * @param cols The number of columns to allocate.
 * @return A pointer to the newly allocated DataFrame, or NULL if allocation fails.
 */
DataFrame *create_dataframe(size_t rows, size_t cols);

/**
 * @brief Safely deallocates a DataFrame and all its inner dynamically allocated contents.
 * @param df Pointer to the DataFrame to be freed.
 */
void free_dataframe(DataFrame *df);

/**
 * @brief Prints a formatted tabular preview of the first few rows of the DataFrame.
 * @param df Pointer to the DataFrame to display.
 * @param limit The maximum number of rows to print.
 */
void print_head_dataframe(const DataFrame *df, int limit);

#endif //STATISTICALDATAPROCESSOR_DATAFRAME_H
