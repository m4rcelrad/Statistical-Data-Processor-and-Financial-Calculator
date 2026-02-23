cmake_minimum_required(VERSION 3.20)
project(StatisticalDataProcessor C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_C_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(CORE_SOURCES
        src/money.c
        src/loan_math.c
        src/loan_simulation.c
        src/report.c
        src/dataframe.c
        src/csv_reader.c
        src/statistics.c
        src/memory_utils.c
)

add_library(CoreLib STATIC ${CORE_SOURCES})

target_include_directories(CoreLib PUBLIC include)

if(NOT MSVC)
    target_link_libraries(CoreLib PUBLIC m)
endif()

add_executable(StatisticalDataProcessor src/main.c)
target_link_libraries(StatisticalDataProcessor PRIVATE CoreLib)


enable_testing()
set(UNITY_DIR tests/unity)

add_executable(run_tests
        tests/main_tests.c
        tests/tests_money.c
        tests/tests_loan_math.c
        tests/tests_loan_simulation.c
        tests/tests_dataframe.c
        tests/tests_csv_reader.c
        tests/tests_statistics.c
        ${UNITY_DIR}/unity.c
)

target_include_directories(run_tests PRIVATE ${UNITY_DIR})
target_compile_definitions(run_tests PRIVATE UNITY_INCLUDE_DOUBLE)
target_link_libraries(run_tests PRIVATE CoreLib)