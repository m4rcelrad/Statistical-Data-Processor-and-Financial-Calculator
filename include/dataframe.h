#ifndef STATISTICALDATAPROCESSOR_DATAFRAME_H
#define STATISTICALDATAPROCESSOR_DATAFRAME_H

#include <stdbool.h>

typedef enum {
    DATAFRAME_SUCCESS = 0,
    DATAFRAME_ERR_FILE_NOT_FOUND,
    DATAFRAME_ERR_EMPTY_FILE,
    DATAFRAME_ERR_ALLOCATION_FAILED,
    DATAFRAME_ERR_COLUMN_MISMATCH
} DataframeErrorCode;

typedef struct {
  char **columns;
  double **data;
  int rows;
  int cols;
} DataFrame;

DataFrame* create_dataframe(int rows, int cols);

void free_dataframe(DataFrame *df);

DataframeErrorCode read_csv(const char *path, bool has_header, const char *delim, DataFrame **out_df);

void print_head_dataframe(const DataFrame *df, int limit);

#endif //STATISTICALDATAPROCESSOR_DATAFRAME_H