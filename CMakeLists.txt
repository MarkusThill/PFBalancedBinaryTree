# cmake_minimum_required(VERSION <specify CMake version here>)
project(PFTreeMap C)

set(CMAKE_C_STANDARD 99)

add_executable(PFTreeMap main.c TreeMap.c)