cmake_minimum_required(VERSION 3.10)

project(AlgoritmiEStruttureDati_Elaborato)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED true)

add_executable(quick_sort
    src/runner/quicksort/main.c
)
target_link_libraries(quick_sort m)

add_executable(quick_sort_3way
    src/runner/quicksort3way/main.c
)
target_link_libraries(quick_sort_3way m)

add_executable(counting_sort
    src/runner/countingsort/main.c
)
target_link_libraries(counting_sort m)

add_executable(intro_sort
    src/runner/introsort/main.c
)
target_link_libraries(intro_sort m)

