#include <stdio.h>

#include <algorithm_runner/algorithm_runner.h>

#define CMP(...) (statistics->comparison_count++, __VA_ARGS__)

bool linear_scale(size_t* prev) {
	(*prev)++;
	return *prev <= 3000;
}

static Algorithm algorithms[] = {
        {
            .algorithm_callback = insertion_sort,
            .name = "Insertion sort",
        },
        {
            .algorithm_callback = quicksort,
            .name = "Quicksort",
        },
        {
            .algorithm_callback = quicksort_3way,
            .name = "Quicksort 3-way",
        }
};

const Algorithm_Runner_Descriptor runner_configuration = {
        .algorithms = algorithms,
        .algorithms_count = 3,
        .element_count_incrementer = linear_scale,
        .single_test_stats_output_file = "../results/single_test_results.csv",
        .average_stats_output_file = "../results/average_results.csv",
        .runs_per_test = 16
};

int main() {
	Algorithm_Runner runner;
	Algorithm_Runner_Result runner_creation_result = algorithmrunner_create(&runner, &runner_configuration);
	if (runner_creation_result != ALGORITHM_RUNNER_RESULT_SUCCESS) {
		fprintf(stderr, "Could not create an algorithm runner. Got error code %d.\n", runner_creation_result);
		return -1;
	}

	Algorithm_Runner_Result run_result = algorithmrunner_run(&runner);
	if (run_result != ALGORITHM_RUNNER_RESULT_SUCCESS) {
		fprintf(stderr, "Could not run the algorithm runner. Got error code %d.\n", run_result);
		return -1;
	}

	algorithmrunner_destroy(&runner);
	return 0;
}
