cmake_minimum_required(VERSION 3.10)

project(AlgoritmiEStruttureDati_Elaborato)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED true)

add_executable(insertion_sort
    src/runner/insertion_sort/main.c
)
add_executable(quick_sort
    src/runner/quicksort/main.c
)
add_executable(quick_sort_3way
    src/runner/quicksort3way/main.c
)


