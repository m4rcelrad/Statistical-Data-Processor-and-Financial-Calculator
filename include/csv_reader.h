#ifndef STATISTICALDATAPROCESSOR_CSV_READER_H
#define STATISTICALDATAPROCESSOR_CSV_READER_H

#include "dataframe.h"
#include <stdbool.h>

DataframeErrorCode read_csv(const char *path, bool has_header, const char *delim, DataFrame **out_df);

#endif // STATISTICALDATAPROCESSOR_CSV_READER_H
