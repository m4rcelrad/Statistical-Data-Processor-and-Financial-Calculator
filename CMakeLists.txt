cmake_minimum_required(VERSION 4.1)
project(StatisticalDataProcessor C)

set(CMAKE_C_STANDARD 17)

include_directories(include)
file(GLOB SOURCES "src/*.c")

add_executable(StatisticalDataProcessor ${SOURCES}
        include/finance.h
        src/finance.c
        include/report.h
        src/report.c)