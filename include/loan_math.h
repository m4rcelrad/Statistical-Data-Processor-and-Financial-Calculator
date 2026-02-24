#ifndef STATISTICALDATAPROCESSOR_FINANCE_H
#define STATISTICALDATAPROCESSOR_FINANCE_H

#include "money.h"

/**
 * @file loan_math.h
 * @brief Core mathematical operations and data structures for loan calculations.
 *
 * This header defines the essential types and mathematical functions required
 * to calculate loan installments, interest, and baseline payments.
 */

/**
 * @brief Represents an interest rate.
 */
typedef struct {
    long double value; /*!< The rate value as a decimal (e.g., 0.05 for 5%). */
} Rate;

/**
 * @brief Specifies the type of the loan installment plan.
 */
typedef enum {
    LOAN_EQUAL_INSTALLMENTS,     /*!< Installments are equal throughout the loan term. */
    LOAN_DECREASING_INSTALLMENTS /*!< Capital part is fixed, resulting in decreasing installments. */
} LoanType;

/**
 * @brief Specifies the strategy for handling overpayments.
 */
typedef enum {
    STRATEGY_REDUCE_TERM,       /*!< Overpayment reduces the total number of months. */
    STRATEGY_REDUCE_INSTALLMENT /*!< Overpayment reduces the number of later installments. */
} OverpaymentStrategy;

/**
 * @brief Error codes returned by finance-related functions.
 */
typedef enum {
    FINANCE_SUCCESS = 0,           /*!< Operation completed successfully. */
    FINANCE_ERR_INVALID_PRINCIPAL, /*!< The provided principal amount is invalid (e.g., zero or negative). */
    FINANCE_ERR_INVALID_MONTHS, /*!< The loan term in months is invalid or exceeds the maximum allowed. */
    FINANCE_ERR_INVALID_ARGUMENT, /*!< A provided argument (e.g., pointer) is invalid or NULL. */
    FINANCE_ERR_NULL_RATES,       /*!< The array containing market interest rates is NULL. */
    FINANCE_ERR_INVALID_RATE, /*!< An interest rate value is invalid (e.g., negative or non-finite). */
    FINANCE_ERR_ALLOCATION_FAILED,     /*!< Memory allocation failed during the operation. */
    FINANCE_ERR_NEGATIVE_AMORTIZATION, /*!< The payment is too small to cover the accrued interest. */
    FINANCE_ERR_PAYMENT_TOO_LARGE, /*!< A custom payment significantly exceeds the remaining balance and interest. */
    FINANCE_ERR_NUMERIC_OVERFLOW /*!< A numeric overflow occurred during a mathematical calculation. */
} FinanceErrorCode;

/**
 * @brief Details of a single loan installment.
 */
typedef struct {
    Money capital;  /*!< The portion of the payment applied to the principal. */
    Money interest; /*!< The portion of the payment applied to interest. */
    Money payment;  /*!< The total payment amount (capital and interest). */
    Money balance;  /*!< The remaining principal balance after this payment. */
} Installment;

/**
 * @brief Core parameters defining a loan.
 */
typedef struct {
    Money principal; /*!< The initial amount borrowed. */
    int term_months; /*!< The duration of the loan in months. */
    LoanType type;   /*!< The installment plan type. */
} LoanDefinition;

/**
 * @brief Represents market conditions, specifically interest rates over time.
 */
typedef struct {
    const Rate *annual_rates; /*!< Array of annual interest rates for each month. */
} MarketScenario;

/**
 * @brief Configuration for the loan simulation behavior.
 */
typedef struct {
    OverpaymentStrategy strategy; /*!< How to handle custom overpayments. */
    const Money *custom_payments; /*!< Optional array of custom payment amounts per month. */
} SimulationConfig;

/**
 * @brief The complete schedule of all loan installments.
 */
typedef struct {
    Installment *items;   /*!< Dynamically allocated array of installments. */
    int count;            /*!< The actual number of recorded installments. */
    Money total_interest; /*!< The sum of all interest paid. */
    Money total_paid;     /*!< The sum of all payments made. */
} LoanSchedule;

/**
 * @brief The current state of an ongoing loan simulation.
 */
typedef struct {
    Money current_balance;    /*!< The remaining principal balance. */
    Money last_total_payment; /*!< The amount paid in the previous month. */
    int current_month;        /*!< The index of the current simulation month. */
} SimulationState;

/**
 * @brief Frees the memory allocated for a loan schedule and resets its state.
 * @param schedule Pointer to the LoanSchedule to clear.
 */
void free_schedule(LoanSchedule *schedule);

/**
 * @brief Creates a Rate structure from a floating-point value.
 * @param value The interest rate as a decimal.
 * @return The initialized Rate structure.
 */
Rate create_rate(long double value);

/**
 * @brief Converts a FinanceErrorCode into a string.
 * @param code The error code.
 * @return A constant string describing the error.
 */
const char *finance_error_string(FinanceErrorCode code);

/**
 * @brief Calculates the interest for a single month.
 * @param balance The current principal balance.
 * @param current_rate The annual interest rate applicable for this month.
 * @return The calculated interest amount for the month.
 */
Money calculate_monthly_interest(Money balance, Rate current_rate);

/**
 * @brief Calculates the baseline required payment for the current month.
 * * This function determines the minimum required payment (capital and interest)
 * based on the loan type (equal or decreasing installments).
 * * @param loan Pointer to the loan definition.
 * @param market Pointer to the market scenario.
 * @param state Pointer to the current simulation state.
 * @param interest The accrued interest for the current month.
 * @param out_payment Pointer to store the calculated baseline payment.
 * @return FINANCE_SUCCESS on success, or an appropriate error code.
 */
FinanceErrorCode calculate_baseline_payment(const LoanDefinition *loan,
                                            const MarketScenario *market,
                                            const SimulationState *state,
                                            Money interest,
                                            Money *out_payment);

#endif // STATISTICALDATAPROCESSOR_FINANCE_H
