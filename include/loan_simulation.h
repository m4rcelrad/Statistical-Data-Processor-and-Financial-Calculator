#ifndef STATISTICALDATAPROCESSOR_LOAN_SIMULATION_H
#define STATISTICALDATAPROCESSOR_LOAN_SIMULATION_H

#include "loan_math.h"

FinanceErrorCode run_loan_simulation(const LoanDefinition *loan,
                                     const MarketScenario *market,
                                     const SimulationConfig *config,
                                     LoanSchedule *out_result);

#endif // STATISTICALDATAPROCESSOR_LOAN_SIMULATION_H