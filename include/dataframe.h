#ifndef STATISTICALDATAPROCESSOR_DATAFRAME_H
#define STATISTICALDATAPROCESSOR_DATAFRAME_H

#include <stdbool.h>

typedef struct {
  char **columns;
  double **data;
  int rows;
  int cols;
} DataFrame;

DataFrame* create_dataframe(int rows, int cols);

void free_dataframe(DataFrame *df);

DataFrame* read_csv(const char *path, bool has_header);

void print_head_dataframe(const DataFrame *df, int limit);

#endif //STATISTICALDATAPROCESSOR_DATAFRAME_H