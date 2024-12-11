#include <stdio.h>

#include <algorithm_runner/algorithm_runner.h>

#define CMP(...) (statistics->comparison_count++, __VA_ARGS__)

bool linear_scale(size_t* prev) {
	(*prev)++;
	return *prev <= 3000;
}

void insertion_sort(int64_t* array, size_t array_length, Algorithm_Stats* statistics) {
	for (size_t i = 1; i < array_length; i++) {
		uint64_t key = array[i];
		size_t j = i - 1;

		while (CMP((j + 1) > 0) && CMP(array[j] > key)) {
			array[j + 1] = array[j];
			j--;
		}

		array[j + 1] = key;
		statistics->swap_count++;
	}
}

const Algorithm_Runner_Descriptor runner_configuration = (Algorithm_Runner_Descriptor) {
	.algorithms_count = 1,
	.algorithms = &(Algorithm){
		.algorithm_callback = insertion_sort,
		.name = "Insertion sort",
	},
	.element_count_incrementer = linear_scale,
	.single_test_stats_output_file = "results/single_test_results.csv",
	.average_stats_output_file = "results/average_results.csv",
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
