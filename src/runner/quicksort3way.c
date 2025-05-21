#include "algorithm_runner/algorithm_runner.h"
#include <stdlib.h>

static void qs3_rec(int64_t* a, ssize_t lo, ssize_t hi, Algorithm_Stats* stats) {
        if (lo >= hi) return;
        
        int64_t pivot = a[lo];

        ssize_t lt = lo, i = lo + 1, gt = hi;
        while (i <= gt) {

            stats->comparison_count++;
            if (a[i] < pivot) {

                int64_t tmp = a[lt];
                a[lt++] = a[i];
                a[i++] = tmp;
                stats->swap_count++;
            }
            else {
                stats->comparison_count++;
                if (a[i] > pivot) {
                    
                    int64_t tmp = a[i];
                    a[i] = a[gt];
                    a[gt--] = tmp;
                    stats->swap_count++;
                } else {
                    
                    i++;
                    stats->comparison_count++;
                }
            }
        }
        qs3_rec(a, lo, lt - 1, stats);
        qs3_rec(a, gt + 1, hi, stats);
}

void quicksort_3way(int64_t* array, size_t array_length, Algorithm_Stats* statistics) {
    
        statistics->comparison_count = 0;
        statistics->swap_count = 0;
        statistics->bytes_allocated = 0;
        if (array_length > 0) {
                qs3_rec(array, 0, (ssize_t)array_length - 1, statistics);
        }
}

