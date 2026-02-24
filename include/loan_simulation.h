#ifndef STATISTICALDATAPROCESSOR_LOAN_SIMULATION_H
#define STATISTICALDATAPROCESSOR_LOAN_SIMULATION_H

#include "loan_math.h"

/**
 * @file loan_simulation.h
 * @brief Interfaces for simulating loan repayment schedules.
 *
 * This module orchestrates the step-by-step calculation of loan installments,
 * supporting various configurations such as variable interest rates and overpayments.
 */

/**
 * @brief Initializes the simulation state with the starting principal.
 * @param state Pointer to the simulation state to initialize.
 * @param principal The starting amount of the loan.
 */
void loan_simulation_init(SimulationState *state, Money principal);

/**
 * @brief Performs a single monthly step in the loan simulation.
 * * Computes the interest, required baseline payment, and actual payment (considering
 * any custom overpayments or strategies). It updates the simulation state and populates
 * the output installment structure.
 *
 * @param loan Pointer to the core loan definition.
 * @param market Pointer to the market scenario (interest rates).
 * @param config Pointer to the simulation configuration (overpayment strategy).
 * @param state Pointer to the active simulation state (updated during the call).
 * @param out_installment Pointer to the structure where the result of this month will be saved.
 * @return FINANCE_SUCCESS on successful calculation, or an appropriate error code.
 */
FinanceErrorCode loan_simulation_step(const LoanDefinition *loan,
                                      const MarketScenario *market,
                                      const SimulationConfig *config,
                                      SimulationState *state,
                                      Installment *out_installment);

/**
 * @brief Checks if the loan simulation has reached its conclusion.
 * * The simulation is complete either when the remaining balance drops to zero or
 * below, or when the maximum number of planned months has been reached.
 *
 * @param loan Pointer to the core loan definition.
 * @param state Pointer to the active simulation state.
 * @return true if the simulation is complete, false otherwise.
 */
bool loan_simulation_is_complete(const LoanDefinition *loan, const SimulationState *state);

/**
 * @brief Runs the complete loan simulation from start to finish.
 * * This is the main driver function. It validates inputs, allocates memory for the
 * schedule, and iterates month by month using loan_simulation_step until the loan
 * is fully paid off or the term is reached.
 *
 * @param loan Pointer to the core loan definition.
 * @param market Pointer to the market scenario (interest rates).
 * @param config Pointer to the simulation configuration (overpayments).
 * @param out_result Pointer to the LoanSchedule where the complete history will be stored.
 * @return FINANCE_SUCCESS if the entire simulation completes successfully, or an error code.
 */
FinanceErrorCode run_loan_simulation(const LoanDefinition *loan,
                                     const MarketScenario *market,
                                     const SimulationConfig *config,
                                     LoanSchedule *out_result);

#endif // STATISTICALDATAPROCESSOR_LOAN_SIMULATION_H
