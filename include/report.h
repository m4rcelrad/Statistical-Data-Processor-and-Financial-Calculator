#ifndef STATISTICALDATAPROCESSOR_REPORT_H
#define STATISTICALDATAPROCESSOR_REPORT_H

#include "loan_math.h"

void print_report_header(void);
void print_report_row(int month_idx, const Installment *inst);
void print_report_summary(Money total_interest, Money total_paid);

void print_schedule_to_console(const LoanSchedule *schedule);
int save_schedule_to_csv(const LoanSchedule *schedule, const char *filename);

#endif // STATISTICALDATAPROCESSOR_REPORT_H