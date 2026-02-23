#ifndef STATISTICALDATAPROCESSOR_DATAFRAME_H
#define STATISTICALDATAPROCESSOR_DATAFRAME_H

typedef enum {
    DATAFRAME_SUCCESS = 0,
    DATAFRAME_ERR_FILE_NOT_FOUND,
    DATAFRAME_ERR_EMPTY_FILE,
    DATAFRAME_ERR_ALLOCATION_FAILED,
    DATAFRAME_ERR_COLUMN_MISMATCH
} DataframeErrorCode;

typedef enum {
    TYPE_NUMERIC,
    TYPE_STRING
} DataType;

typedef union {
    double v_num;
    char *v_str;
} DataCell;

typedef struct {
    char **columns;
    DataType *col_types;
    DataCell **data;
    int rows;
    int cols;
} DataFrame;

DataFrame *create_dataframe(int rows, int cols);

void free_dataframe(DataFrame *df);

void print_head_dataframe(const DataFrame *df, int limit);

#endif //STATISTICALDATAPROCESSOR_DATAFRAME_H