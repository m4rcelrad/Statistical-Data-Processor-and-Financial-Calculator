cmake_minimum_required(VERSION 4.1)
project(StatisticalDataProcessor C)

set(CMAKE_C_STANDARD 17)

include_directories(include)
file(GLOB SOURCES "src/*.c")

add_executable(StatisticalDataProcessor ${SOURCES}
        include/loan_math.h
        src/loan_math.c
        include/report.h
        src/report.c
        src/money.c
        include/money.h
        src/loan_simulation.c
        include/loan_simulation.h
        tests/unity/unity.c
        tests/unity/unity.h
        tests/unity/unity_internals.h)