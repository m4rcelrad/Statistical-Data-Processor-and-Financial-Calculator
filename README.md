# Statistical Data Processor and Financial Calculator

A high-performance command-line application written in C that combines a financial loan simulator with a statistical time-series data processor. 
## 🚀 Features

**Financial Calculator**
* Simulates complex loan amortization schedules (equal and decreasing installments).
* Calculates dynamic overpayment strategies (reduce term or reduce installment).
* Uses **fixed-point arithmetic** to prevent standard floating-point precision loss.
* Imports irregular payment schedules directly from CSV files.

**Statistical & Time-Series Analyzer**
* Calculates descriptive statistics (mean, variance, standard deviation) using **Welford's online algorithm** for numerical stability.
* Computes SMA, EMA, Bollinger Bands, covariance, and Pearson correlation.
* Generates basic algorithmic trading signals based on moving average crossovers.

**Under the Hood**
* **Custom DataFrame:** A dynamic 2D grid structure for parsing and storing mixed-type CSV data.
* **Optimized Memory:** Uses cache-line aligned allocations to improve memory access speeds.
* **Robust Testing:** Comprehensive unit test suite covering math and memory modules.

## 🛠️ Tech Stack
* **Language:** C (C11)
* **Build System:** CMake
* **Testing:** Unity Framework

## ⚙️ Getting Started

### Prerequisites
* C compiler (GCC, Clang, MSVC)
* CMake (v3.10+)

### Build and Run

```bash
git clone [https://github.com/yourusername/statistical-data-processor.git](https://github.com/yourusername/statistical-data-processor.git)
cd statistical-data-processor

# Create build directory and compile
mkdir build && cd build
cmake ..
cmake --build .

# Run the application
./StatisticalDataProcessor

# Run unit tests
ctest --output-on-failure
