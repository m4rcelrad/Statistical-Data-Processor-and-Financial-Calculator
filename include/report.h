#ifndef STATISTICALDATAPROCESSOR_REPORT_H
#define STATISTICALDATAPROCESSOR_REPORT_H

#include "loan_math.h"

void print_schedule_to_console(const LoanSchedule *schedule);

int save_schedule_to_csv(const LoanSchedule *schedule, const char *filename);

#endif // STATISTICALDATAPROCESSOR_REPORT_H