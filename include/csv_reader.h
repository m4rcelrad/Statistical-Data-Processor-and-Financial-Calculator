#ifndef STATISTICALDATAPROCESSOR_CSV_READER_H
#define STATISTICALDATAPROCESSOR_CSV_READER_H

#include "dataframe.h"
#include <stdbool.h>

/**
 * @file csv_reader.h
 * @brief Module for reading and parsing CSV files into DataFrame structures.
 *
 * This header exposes the main functionality required to load tabular data
 * from CSV files, automatically detecting column counts and basic data types.
 */

/**
 * @brief Reads a CSV file and populates a dynamically allocated DataFrame.
 * * This function handles dynamic row and column counting, extracts headers
 * (or generates default ones), and attempts to infer whether a column is
 * numeric or string-based by inspecting the first row of data.
 *
 * @param path The file path to the CSV file to be read.
 * @param has_header A boolean flag indicating whether the first line contains column names.
 * @param delim The delimiter string used in the CSV (e.g., ",", ";").
 * @param out_df Pointer to a DataFrame pointer where the newly created DataFrame will be stored.
 * @return DATAFRAME_SUCCESS on success, or an appropriate DataframeErrorCode on failure.
 */
DataframeErrorCode read_csv(const char *path, bool has_header, const char *delim, DataFrame **out_df);

#endif // STATISTICALDATAPROCESSOR_CSV_READER_H