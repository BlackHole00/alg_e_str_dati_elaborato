cmake_minimum_required(VERSION 3.10)

project(AlgoritmiEStruttureDati_Elaborato)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED true)

add_executable(AlgoritmiEStruttureDati_Elaborato
    src/main.c
    src/algorithm_runner/algorithm_runner.c
)
target_include_directories(AlgoritmiEStruttureDati_Elaborato PUBLIC
    src/algorithm_runner
)

