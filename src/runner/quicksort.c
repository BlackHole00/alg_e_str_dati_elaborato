#include "algorithm_runner/algorithm_runner.h"
#include <stdint.h>
#include <stddef.h>

static void swap(int64_t* a, int64_t* b, Algorithm_Stats* stats) {
    int64_t t = *a;
    *a = *b;
    *b = t;
    stats->swap_count++;
}

static int64_t partition(int64_t arr[], int64_t low, int64_t high, Algorithm_Stats* stats) {
    int64_t pivot = arr[high];
    int64_t i = low - 1;
    for (int64_t j = low; j <= high - 1; j++) {
        stats->comparison_count++;
        if (arr[j] < pivot) {
            i++;
            swap(&arr[i], &arr[j], stats);
        }
    }
    swap(&arr[i + 1], &arr[high], stats);
    return i + 1;
}

static void quickSort_rec(int64_t arr[], int64_t low, int64_t high, Algorithm_Stats* stats) {
    if (low < high) {
        int64_t pi = partition(arr, low, high, stats);
        quickSort_rec(arr, low, pi - 1, stats);
        quickSort_rec(arr, pi + 1, high, stats);
    }
}

void quicksort(int64_t* array, size_t array_length, Algorithm_Stats* statistics) {
    statistics->comparison_count = 0;
    statistics->swap_count       = 0;
    statistics->bytes_allocated  = 0;

    if (array_length > 1) {
        quickSort_rec(array, 0, (int64_t)array_length - 1, statistics);
    }
}

