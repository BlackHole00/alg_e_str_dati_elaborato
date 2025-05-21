#include "algorithm_runner/algorithm_runner.h"
#include <stddef.h>
#include <stdint.h>

static ssize_t partition(int64_t* a, ssize_t lo, ssize_t hi, Algorithm_Stats* stats) {
    int64_t pivot = a[hi];
    ssize_t i = lo - 1;
    for (ssize_t j = lo; j < hi; j++) {
        stats->comparison_count++;
        if (a[j] <= pivot) {
            i++;
            int64_t tmp = a[i];
            a[i]     = a[j];
            a[j]     = tmp;
            stats->swap_count++;
        }
    }
    int64_t tmp = a[i + 1];
    a[i + 1]   = a[hi];
    a[hi]      = tmp;
    stats->swap_count++;
    return i + 1;
}

static void qs_rec(int64_t* a, ssize_t lo, ssize_t hi, Algorithm_Stats* stats) {
    if (lo < hi) {
        ssize_t p = partition(a, lo, hi, stats);
        qs_rec(a, lo, p - 1, stats);
        qs_rec(a, p + 1, hi, stats);
    }
}

void quicksort(int64_t* array, size_t array_length, Algorithm_Stats* statistics) {
    statistics->comparison_count = 0;
    statistics->swap_count       = 0;
    statistics->bytes_allocated  = 0;

    if (array_length > 1) {
        qs_rec(array, 0, (ssize_t)array_length - 1, statistics);
    }
}

