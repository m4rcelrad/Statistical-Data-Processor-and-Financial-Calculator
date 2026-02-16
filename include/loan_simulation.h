#ifndef STATISTICALDATAPROCESSOR_LOAN_SIMULATION_H
#define STATISTICALDATAPROCESSOR_LOAN_SIMULATION_H

#include "loan_math.h"

void loan_simulation_init(SimulationState *state, Money principal);

FinanceErrorCode loan_simulation_step(const LoanDefinition *loan,
                                      const MarketScenario *market,
                                      const SimulationConfig *config,
                                      SimulationState *state,
                                      Installment *out_installment);

bool loan_simulation_is_complete(const LoanDefinition *loan, const SimulationState *state);

FinanceErrorCode run_loan_simulation(const LoanDefinition *loan,
                                     const MarketScenario *market,
                                     const SimulationConfig *config,
                                     LoanSchedule *out_result);

#endif // STATISTICALDATAPROCESSOR_LOAN_SIMULATION_H