cmake_minimum_required(VERSION 3.20)
project(StatisticalDataProcessor C)

set(CMAKE_C_STANDARD 17)
include_directories(include)

set(CORE_SOURCES
        src/money.c
        src/loan_math.c
        src/loan_simulation.c
        src/report.c
)

add_executable(StatisticalDataProcessor
        src/main.c
        ${CORE_SOURCES}
)

enable_testing()

set(UNITY_DIR tests/unity)
include_directories(${UNITY_DIR})

add_executable(run_tests
        tests/tests_money.c
        ${UNITY_DIR}/unity.c
        ${CORE_SOURCES}
)

target_compile_definitions(run_tests PRIVATE UNITY_INCLUDE_DOUBLE)