#include "algorithm_runner.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <utils/random.h>

/**
	@struct Algorithm_Extended_Stats
	@brief INTERNAL. Describes the statistics related to a single test or
	the average of multiple of them.
*/
typedef struct Algorithm_Extended_Stats {
	Algorithm_Stats stats;
	uint64_t nanoseconds_taken;
} Algorithm_Extended_Stats;

/**
	@fn algorithmrunner_check_descriptor
	@brief INTERNAL. Checks if a given `Algorithm_Runner_Descriptor` is 
	valid.
*/
static bool algorithmrunner_check_descriptor(const Algorithm_Runner_Descriptor* descriptor);

/**
	@fn algorithmrunner_test_algorithm
	@brief INTERNAL. Benchmarks and tests a single algorithm. Writes the
	results (both of single tests and averages) in the specified files.
*/
static Algorithm_Runner_Result algorithmrunner_test_algorithm(
	const Algorithm_Runner* runner,
	const Algorithm* algorithm
);

/**
	@fn algorithmrunner_test_algorithm_with_elements
	@brief INTERNAL. Benchmarks and tests a given algorithm with an array of
	the specified length. Writes the results of the single tests to the
	specified output files, but writes the average results to the specified 
	pointer.
*/
static Algorithm_Runner_Result algorithmrunner_test_algorithm_with_elements(
	const Algorithm_Runner* runner,
	const Algorithm* algorithm,
	size_t element_count,
	Algorithm_Extended_Stats* average_stats
);

/**
	@fn algorithmrunner_test_algorithm_with_array
	@brief INTERNAL. Benchmarks and tests a given algorithm with a given
	array. Writes the resulting statistics to the specified pointer.
*/
static Algorithm_Runner_Result algorithmrunner_test_algorithm_with_array(
	const Algorithm_Runner* runner,
	const Algorithm* algorithm,
	int64_t* array,
	size_t element_count,
	Algorithm_Extended_Stats* test_stats
);

/**
	@fn algorithmrunner_publish_algorithm_change
	@brief INTERNAL. Publishes a change of algorithm to the output files.
	This practically writes an empty line (if the file is not empty)
	followed by a line containing only the name of the algorithm.
*/
static void algorithmrunner_publish_algorithm_change(
	Algorithm_Runner* runner,
	const Algorithm* new_algorithm
);

/**
	@fn algorithmrunner_publish_average_stats
	@brief INTERNAL. Publishes the average stats of a series of test related
	to an algorithm to the related output file.
*/
static void algorithmrunner_publish_average_stats(
	const Algorithm_Runner* runner,
	const Algorithm* algorithm,
	size_t element_count,
	const Algorithm_Extended_Stats* stats
);

/**
	@fn algorithmrunner_publish_single_test_stats
	@brief INTERNAL. Publishes the results of a single test related to an
	algorithm to the related output file.
*/
static void algorithmrunner_publish_single_test_stats(
	const Algorithm_Runner* runner,
	const Algorithm* algorithm,
	size_t element_count,
	uint64_t test_number,
	const Algorithm_Extended_Stats* stats
);

/**
	@fn algorithmrunner_flush_outputs
	@brief INTERNAL. Flushes all the output files.
*/
static void algorithmrunner_flush_outputs(const Algorithm_Runner* runner);

/**
	@fn populate_array
	@brief INTERNAL. Populates the given array with random values.
*/
static void populate_array(int64_t* array, size_t array_size);

/**
	@fn check_if_sorted
	@brief INTERNAL. Checks if the given array is sorted
*/
static bool check_if_sorted(int64_t* array, size_t array_size);

Algorithm_Runner_Result algorithmrunner_create(
	Algorithm_Runner* runner,
	const Algorithm_Runner_Descriptor* descriptor
) {
	if (runner == NULL) {
		return ALGORITHM_RUNNER_RESULT_INVALID_PARAMETER;
	}
	if (!algorithmrunner_check_descriptor(descriptor)) {
		return ALGORITHM_RUNNER_RESULT_INVALID_DESCRIPTOR;
	}

	runner->single_test_stats_output_file = fopen(descriptor->single_test_stats_output_file, "w");
	if (runner->single_test_stats_output_file == NULL) {
		return ALGORITHM_RUNNER_RESULT_IO_ERROR;
	}

	runner->average_stats_output_file = fopen(descriptor->average_stats_output_file, "w");
	if (runner->average_stats_output_file == NULL) {
		fclose(runner->single_test_stats_output_file);
		return ALGORITHM_RUNNER_RESULT_IO_ERROR;
	}

	runner->algorithms = descriptor->algorithms;
	runner->algorithms_count = descriptor->algorithms_count;
	runner->element_count_incrementer = descriptor->element_count_incrementer;
	runner->runs_per_test = descriptor->runs_per_test;
	runner->outputs_did_write_first_line = false;

	return ALGORITHM_RUNNER_RESULT_SUCCESS;
}

void algorithmrunner_destroy(const Algorithm_Runner* runner) {
	fclose(runner->single_test_stats_output_file);
	fclose(runner->average_stats_output_file);
}

Algorithm_Runner_Result algorithmrunner_run(Algorithm_Runner* runner) {
	for (size_t algorithm_index = 0; algorithm_index < runner->algorithms_count; algorithm_index++) {
		const Algorithm* algorithm = &runner->algorithms[algorithm_index];
		algorithmrunner_publish_algorithm_change(runner, algorithm);

		Algorithm_Runner_Result run_result = algorithmrunner_test_algorithm(runner, algorithm);
		if (run_result != ALGORITHM_RUNNER_RESULT_SUCCESS) {
			return run_result;
		}
	}

	return ALGORITHM_RUNNER_RESULT_SUCCESS;
}

static bool algorithmrunner_check_descriptor(const Algorithm_Runner_Descriptor* descriptor) {
	if (descriptor == NULL) {
		return false;
	}
	if (descriptor->runs_per_test == 0) {
		return false;
	}
	if (descriptor->single_test_stats_output_file == NULL || descriptor->average_stats_output_file == NULL) {
		return false;
	}
	if (strcmp(descriptor->single_test_stats_output_file, descriptor->average_stats_output_file) == 0) {
		return false;
	}
	if (descriptor->algorithms_count == 0 || descriptor->algorithms == NULL) {
		return false;
	}
	if (descriptor->element_count_incrementer == NULL) {
		return false;
	}

	for (size_t i = 0; i < descriptor->algorithms_count; i++) {
		if (descriptor->algorithms[i].name == NULL || descriptor->algorithms[i].algorithm_callback == NULL) {
			return false;
		}
	}

	return true;
}

static Algorithm_Runner_Result algorithmrunner_test_algorithm(
	const Algorithm_Runner* runner,
	const Algorithm* algorithm
) {
	// NOTE: By resetting the random number generator we are providing a way
	// to test the various algorithms with the same arrays every time.
	srand(0);

	size_t element_count = 1;
	while (runner->element_count_incrementer(&element_count)) {
		Algorithm_Extended_Stats average_stats;

		Algorithm_Runner_Result test_result = algorithmrunner_test_algorithm_with_elements(
			runner,
			algorithm,
			element_count,
			&average_stats
		);
		if (test_result != ALGORITHM_RUNNER_RESULT_SUCCESS) {
			return test_result;
		}

		algorithmrunner_publish_average_stats(runner, algorithm, element_count, &average_stats);
	}

	algorithmrunner_flush_outputs(runner);
	return ALGORITHM_RUNNER_RESULT_SUCCESS;
}

static Algorithm_Runner_Result algorithmrunner_test_algorithm_with_elements(
	const Algorithm_Runner* runner,
	const Algorithm* algorithm,
	size_t element_count,
	Algorithm_Extended_Stats* average_stats
) {
	*average_stats = (Algorithm_Extended_Stats){};

	int64_t* array_to_sort = malloc(sizeof(int64_t) * element_count);
	if (array_to_sort == NULL) {
		return ALGORITHM_RUNNER_RESULT_MEMORY_ERROR;
	}

	for (uint64_t run_count = 0; run_count < runner->runs_per_test; run_count++) {
		populate_array(array_to_sort, element_count);

		Algorithm_Extended_Stats statistics;
		Algorithm_Runner_Result test_result = algorithmrunner_test_algorithm_with_array(
			runner,
			algorithm,
			array_to_sort,
			element_count,
			&statistics
		);
		if (test_result != ALGORITHM_RUNNER_RESULT_SUCCESS) {
			free(array_to_sort);
			return ALGORITHM_RUNNER_RESULT_SORTING_ERROR;
		}

		algorithmrunner_publish_single_test_stats(runner, algorithm, element_count, run_count, &statistics);

		average_stats->nanoseconds_taken      += statistics.nanoseconds_taken;
		average_stats->stats.bytes_allocated  += statistics.stats.bytes_allocated;
		average_stats->stats.comparison_count += statistics.stats.comparison_count;
		average_stats->stats.swap_count       += statistics.stats.swap_count;
	}

	average_stats->nanoseconds_taken      /= runner->runs_per_test;
	average_stats->stats.comparison_count /= runner->runs_per_test;
	average_stats->stats.swap_count       /= runner->runs_per_test;
	average_stats->stats.bytes_allocated  /= runner->runs_per_test;

	free(array_to_sort);

	return ALGORITHM_RUNNER_RESULT_SUCCESS;
}

static Algorithm_Runner_Result algorithmrunner_test_algorithm_with_array(
	const Algorithm_Runner* runner,
	const Algorithm* algorithm,
	int64_t* array,
	size_t element_count,
	Algorithm_Extended_Stats* test_stats
) {
	*test_stats = (Algorithm_Extended_Stats){};

	struct timespec start_time;
	struct timespec end_time;

	clock_gettime(CLOCK_MONOTONIC, &start_time);
	algorithm->algorithm_callback(array, element_count, &test_stats->stats);
	clock_gettime(CLOCK_MONOTONIC, &end_time);

	if (!check_if_sorted(array, element_count)) {
		return ALGORITHM_RUNNER_RESULT_SORTING_ERROR;
	}

	uint64_t time_taken =
		(end_time.tv_sec - start_time.tv_sec) * 1000000000 + (end_time.tv_nsec - start_time.tv_nsec);
	test_stats->nanoseconds_taken = time_taken;

	return ALGORITHM_RUNNER_RESULT_SUCCESS;
}

static void algorithmrunner_publish_algorithm_change(
	Algorithm_Runner* runner,
	const Algorithm* new_algorithm
) {
	printf("Now testing algorithm `%s`", new_algorithm->name);

	if (runner->outputs_did_write_first_line) {
		fprintf(runner->single_test_stats_output_file, "\n");
		fprintf(runner->average_stats_output_file, "\n");
	} else {
		runner->outputs_did_write_first_line = true;
	}

	fprintf(runner->single_test_stats_output_file, "%s\n", new_algorithm->name);
	fprintf(runner->average_stats_output_file, "%s\n", new_algorithm->name);
}

static void algorithmrunner_publish_average_stats(
	const Algorithm_Runner* runner,
	const Algorithm* algorithm,
	size_t element_count,
	const Algorithm_Extended_Stats* stats
) {
	printf(
		"Finished accumulative test with algorithm `%s` (run %llu times with %llu elements). The "
		"test finished with the following averaged statistics:\n"
		"\t- time taken: %llu ns\n"
		"\t- comparisons: %llu\n"
		"\t- swaps: %llu\n"
		"\t- bytes allocated: %llu bytes\n",
		algorithm->name,
		(unsigned long long)(runner->runs_per_test),
		(unsigned long long)(element_count),
		(unsigned long long)(stats->nanoseconds_taken),
		(unsigned long long)(stats->stats.comparison_count),
		(unsigned long long)(stats->stats.swap_count),
		(unsigned long long)(stats->stats.bytes_allocated)
	);

	fprintf(
		runner->average_stats_output_file,
		"%llu,%lld,%llu,%llu,%llu\n",
		(unsigned long long)(element_count),
		(unsigned long long)(stats->nanoseconds_taken),
		(unsigned long long)(stats->stats.comparison_count),
		(unsigned long long)(stats->stats.swap_count),
		(unsigned long long)(stats->stats.bytes_allocated)
	);
}

static void algorithmrunner_publish_single_test_stats(
	const Algorithm_Runner* runner,
	const Algorithm* algorithm,
	size_t element_count,
	uint64_t test_number,
	const Algorithm_Extended_Stats* stats
) {
	printf(
		"Finished single test with algorithm `%s` (run %llu with %llu elements). The test "
		"finished with the following statistics:\n"
		"\t- time taken: %llu ns\n"
		"\t- comparisons: %llu\n"
		"\t- swaps: %llu\n"
		"\t- memory allocated: %llu bytes\n",
		algorithm->name,
		(unsigned long long)(test_number),
		(unsigned long long)(element_count),
		(unsigned long long)(stats->nanoseconds_taken),
		(unsigned long long)(stats->stats.comparison_count),
		(unsigned long long)(stats->stats.swap_count),
		(unsigned long long)(stats->stats.bytes_allocated)
	);

	fprintf(
		runner->single_test_stats_output_file,
		"%llu,%llu,%llu,%llu,%llu,%llu\n",
		(unsigned long long)(element_count),
		(unsigned long long)(test_number),
		(unsigned long long)(stats->nanoseconds_taken),
		(unsigned long long)(stats->stats.comparison_count),
		(unsigned long long)(stats->stats.swap_count),
		(unsigned long long)(stats->stats.bytes_allocated)
	);
}

static void algorithmrunner_flush_outputs(const Algorithm_Runner* runner) {
	fflush(runner->single_test_stats_output_file);
	fflush(runner->average_stats_output_file);
}

static void populate_array(int64_t* array, size_t array_size) {
	assert(array != NULL);

	for (size_t i = 0; i < array_size; i++) {
		array[i] = random_int64();
	}
}

static bool check_if_sorted(int64_t* array, size_t array_size) {
	assert(array != NULL);

	for (size_t i = 1; i < array_size; i++) {
		if (array[i - 1] > array[i]) {
			return false;
		}
	}

	return true;
}

