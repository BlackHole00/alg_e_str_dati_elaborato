cmake_minimum_required(VERSION 3.10)

project(AlgoritmiEStruttureDati_Elaborato)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED true)

add_executable(AlgoritmiEStruttureDati_Elaborato
    src/runner/main.c
    src/runner/algorithm_runner/algorithm_runner.c
	src/runner/quicksort.c
	src/runner/quicksort3way.c
	src/runner/insertion_sort.c
)
target_include_directories(AlgoritmiEStruttureDati_Elaborato PUBLIC
    src/runner
)

