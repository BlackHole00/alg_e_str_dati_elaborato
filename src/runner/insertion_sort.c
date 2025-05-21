#include "algorithm_runner/algorithm_runner.h"
#include <stdint.h>
#include <stddef.h>

void insertion_sort(int64_t* array, size_t array_length, Algorithm_Stats* statistics) {
    statistics->comparison_count = 0;
    statistics->swap_count = 0;
    statistics->bytes_allocated = 0;

    for (size_t i = 1; i < array_length; i++) {
        int64_t key = array[i];
        ssize_t j = (ssize_t)i - 1;

        while (j >= 0) {
            statistics->comparison_count++;
            if (array[j] > key) {
                array[j + 1] = array[j];
                statistics->swap_count++;
                j--;
            } else {
                break;
            }
        }
        array[j + 1] = key;
    }
}

