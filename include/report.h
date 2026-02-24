#ifndef STATISTICALDATAPROCESSOR_REPORT_H
#define STATISTICALDATAPROCESSOR_REPORT_H

#include "loan_math.h"

/**
 * @file report.h
 * @brief Interfaces for generating and formatting loan simulation reports.
 *
 * This module provides functionalities to display the loan schedule in a
 * table format in the console and to export it to a CSV file.
 */

/**
 * @brief Prints the structured header for the console loan schedule table.
 */
void print_report_header(void);

/**
 * @brief Formats and prints a single row of the loan schedule.
 * @param month_idx The zero-based index of the simulated month.
 * @param inst Pointer to the installment data to print.
 */
void print_report_row(int month_idx, const Installment *inst);

/**
 * @brief Prints the final summary of the loan simulation.
 * @param total_interest The total accumulated interest paid over the term.
 * @param total_paid The absolute total amount paid (principal and interest).
 */
void print_report_summary(Money total_interest, Money total_paid);

/**
 * @brief Prints the entire loan schedule to the standard output.
 * @param schedule Pointer to the generated loan schedule.
 */
void print_schedule_to_console(const LoanSchedule *schedule);

/**
 * @brief Exports the complete loan schedule and summary to a CSV file.
 * @param schedule Pointer to the generated loan schedule.
 * @param filename The path and name of the destination CSV file.
 * @return 0 on success, or -1 if the file could not be opened or data is invalid.
 */
int save_schedule_to_csv(const LoanSchedule *schedule, const char *filename);

#endif // STATISTICALDATAPROCESSOR_REPORT_H
