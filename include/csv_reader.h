#ifndef STATISTICALDATAPROCESSOR_CSV_READER_H
#define STATISTICALDATAPROCESSOR_CSV_READER_H

#include "dataframe.h"
#include <stdbool.h>

typedef enum {
    DATAFRAME_SUCCESS = 0,
    DATAFRAME_ERR_FILE_NOT_FOUND,
    DATAFRAME_ERR_EMPTY_FILE,
    DATAFRAME_ERR_ALLOCATION_FAILED,
    DATAFRAME_ERR_COLUMN_MISMATCH
} DataframeErrorCode;

DataframeErrorCode read_csv(const char *path, bool has_header, const char *delim, DataFrame **out_df);

#endif // STATISTICALDATAPROCESSOR_CSV_READER_H
