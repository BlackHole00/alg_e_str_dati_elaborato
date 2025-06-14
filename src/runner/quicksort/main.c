#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include <time.h>
#include <math.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <string.h>


////////////////////////////////////////////////////////////////////////////////
// CONFIG
////////////////////////////////////////////////////////////////////////////////

#define RUNNER_ALGORITHM_NAME "Quick Sort"
#define RUNNER_ALGORITHM_FUNCTION quicksort

enum Runner_Mode { RUNNERMODE_BENCHMARK, RUNNERMODE_ELEARNING };
#define RUNNER_MODE RUNNERMODE_BENCHMARK

#define RUNNER_MAX_RELATIVE_ERROR 0.00001
#define RUNNER_TEST_COUNT 250

#define RUNNER_STARTING_ARRAY_LENGTH 100
#define RUNNER_ENDING_ARRAY_LENGTH 100000
#define RUNNER_MIN_ARRAY_ELEMENT 10
#define RUNNER_MAX_ARRAY_ELEMENT 100000 + RUNNER_MIN_ARRAY_ELEMENT

#define RUNNER_ARRAY_LENGTH 10000
#define RUNNER_STARTING_ELEMENT_RANGE 10
#define RUNNER_ENDING_ELEMENT_RANGE 1000000

#define RUNNER_ARRAY_LENGTH_OUTPUT_FILE "./results/quicksort.array_length.csv"
#define RUNNER_INPUT_RANGE_OUTPUT_FILE "./results/quicksort.input_range.csv"


////////////////////////////////////////////////////////////////////////////////
// SORTING FUNCTION
////////////////////////////////////////////////////////////////////////////////

void swap(int64_t* a, int64_t* b) {
	int64_t t = *a;
	*a = *b;
	*b = t;
}

int64_t partition(int64_t arr[], int64_t low, int64_t high) {
	int64_t pivot = arr[high];
	int64_t i = low - 1;
	for (int64_t j = low; j <= high - 1; j++) {
		if (arr[j] < pivot) {
			i++;
			swap(&arr[i], &arr[j]);
		}
	}
	swap(&arr[i + 1], &arr[high]);
	return i + 1;
}

void quicksort_rec(int64_t arr[], int64_t low, int64_t high) {
	if (low < high) {
		int64_t pi = partition(arr, low, high);
		quicksort_rec(arr, low, pi - 1);
		quicksort_rec(arr, pi + 1, high);
	}
}

void quicksort(int64_t* array, size_t array_length) {
	if (array_length > 1) {
		quicksort_rec(array, 0, (int64_t)array_length - 1);
	}
}


////////////////////////////////////////////////////////////////////////////////
// BENCHMARK MODE
////////////////////////////////////////////////////////////////////////////////

struct {
	double clock_precision;
	double min_execution_time;
	double array_average_init_time; // for 1 element

	double length_constant_a;
	double length_constant_b;
	double input_range_constant_a;
	double input_range_constant_b;

	int64_t* array_buffer;
	size_t array_buffer_size;

	FILE* output_array_length_file;
	FILE* output_input_range_file;
} g_runner;

size_t calculate_array_length(size_t iteration) {
	double b_power = pow(g_runner.length_constant_b, (double)iteration);
	return (size_t)(g_runner.length_constant_a * b_power);
}

int64_t calculate_input_range(size_t iteration) {
	double b_power = pow(g_runner.input_range_constant_b, (double)iteration);
	return (int64_t)(g_runner.input_range_constant_a * b_power);
}

int64_t calculate_random_array_element(int64_t minimum_element, int64_t maximum_element) {
	return minimum_element + rand() % (maximum_element - minimum_element + 1);
}

void randomize_array(int64_t* array, size_t array_length, int64_t minimum_element, int64_t maximum_element) {
	for (size_t i = 0; i < array_length; i++) {
		g_runner.array_buffer[i] = calculate_random_array_element(minimum_element, maximum_element);
	}

	size_t max_element_index = rand() % array_length;
	g_runner.array_buffer[max_element_index] = maximum_element;

	size_t min_element_index;
	do {
		min_element_index = rand() % array_length;
	} while(array_length != 1 && min_element_index == max_element_index);
	g_runner.array_buffer[min_element_index] = minimum_element;
}

bool is_array_sorted(int64_t* array, size_t array_length) {
	for (size_t i = 1; i < array_length; i++) {
		if (array[i] < array[i - 1]) {
			return false;
		}
	}

	return true;
}

double timespec_duration(struct timespec start, struct timespec end) {
	return end.tv_sec - start.tv_sec
		+ ((end.tv_nsec - start.tv_nsec ) / (double) 1000000000.0);
}

void calculate_clock_precision(void) {
	struct timespec start;
	struct timespec end;

	clock_gettime(CLOCK_MONOTONIC, &start);
	do {
		clock_gettime(CLOCK_MONOTONIC, &end);
	} while (timespec_duration(start, end) == 0.0);

	g_runner.clock_precision =  timespec_duration(start, end);
	g_runner.min_execution_time = g_runner.clock_precision * ((1.0 / RUNNER_MAX_RELATIVE_ERROR) + 1.0);
}

void calculate_array_init_time(void) {
	double total_duration = 0.0;
	size_t initialization_count = 0;
	struct timespec start;
	struct timespec end;

	clock_gettime(CLOCK_MONOTONIC, &start);
	do {
		randomize_array(g_runner.array_buffer,
			g_runner.array_buffer_size,
			RUNNER_MIN_ARRAY_ELEMENT,
			RUNNER_MAX_ARRAY_ELEMENT
		);

		initialization_count += 1;

		clock_gettime(CLOCK_MONOTONIC, &end);
		total_duration = timespec_duration(start, end);
	} while(total_duration < g_runner.min_execution_time);

	g_runner.array_average_init_time =
		total_duration / (double)initialization_count / (double)g_runner.array_buffer_size * 0.8;
}

void init_runner(void) {
	// Srand with seed 0 so it is deterministic
	srand(0);

	calculate_clock_precision();

	g_runner.length_constant_a = (double)RUNNER_STARTING_ARRAY_LENGTH;
	g_runner.length_constant_b = pow(
		(double)RUNNER_ENDING_ARRAY_LENGTH / (double)RUNNER_STARTING_ARRAY_LENGTH,
		1.0 / (double)(RUNNER_TEST_COUNT - 1)
	);

	g_runner.input_range_constant_a = (double)RUNNER_STARTING_ELEMENT_RANGE;
	g_runner.input_range_constant_b = pow(
		(double)RUNNER_ENDING_ELEMENT_RANGE / (double)RUNNER_STARTING_ELEMENT_RANGE,
		1.0 / (double)(RUNNER_TEST_COUNT - 1)
	);

	g_runner.array_buffer = malloc(sizeof(int64_t) * RUNNER_ENDING_ARRAY_LENGTH);
	assert(g_runner.array_buffer != NULL);
	g_runner.array_buffer_size = RUNNER_ENDING_ARRAY_LENGTH;

	calculate_array_init_time();

	g_runner.output_array_length_file = fopen(RUNNER_ARRAY_LENGTH_OUTPUT_FILE, "w");
	assert(g_runner.output_array_length_file != NULL);
	g_runner.output_input_range_file = fopen(RUNNER_INPUT_RANGE_OUTPUT_FILE, "w");
	assert(g_runner.output_input_range_file != NULL);
}

void run_array_length_benchmark_iteration(size_t iteration) {
	size_t array_length = calculate_array_length(iteration);

	printf("Benchmarking array length iteration %llu (%llu elements)...\n",
		(unsigned long long)iteration + 1,
		(unsigned long long)array_length
	);

	double total_duration = 0.0;
	size_t sorted_arrays = 0;
	struct timespec start;
	struct timespec end;

	clock_gettime(CLOCK_MONOTONIC, &start);
	do {
		randomize_array(
			g_runner.array_buffer,
			array_length,
			RUNNER_MIN_ARRAY_ELEMENT,
			RUNNER_MAX_ARRAY_ELEMENT
		);
		RUNNER_ALGORITHM_FUNCTION(g_runner.array_buffer, array_length);

		sorted_arrays += 1;

		clock_gettime(CLOCK_MONOTONIC, &end);
		total_duration = timespec_duration(start, end);
	} while(total_duration < g_runner.min_execution_time);

	double init_duration = g_runner.array_average_init_time * (double)array_length * (double)sorted_arrays;
	double duration_without_init = total_duration - init_duration;
	double average_time = duration_without_init / (double)sorted_arrays;

	printf("Benchmarked array length iteration %llu (%llu elements):\n"
		"\t-total time: %.17fs (%.17fs without init)\n"
		"\t-sorted arrays: %llu\n"
		"\t-averate time: %.17fs\n\n",
		(unsigned long long)iteration + 1,
		(unsigned long long)array_length,
		total_duration,
		duration_without_init,
		(unsigned long long)sorted_arrays,
		average_time
	);

	fprintf(g_runner.output_array_length_file, "%llu, %.17f\n",
		(unsigned long long)array_length,
		average_time
	);
	fflush(g_runner.output_array_length_file);
}

void run_input_range_benchmark_iteration(size_t iteration) {
	int64_t input_range = calculate_input_range(iteration);
	int64_t minimum_element = RUNNER_MIN_ARRAY_ELEMENT;
	int64_t maximum_element = RUNNER_MIN_ARRAY_ELEMENT + input_range;

	printf("Benchmarking input range iteration %llu (%llu input range: %llu-%llu)...\n",
		(unsigned long long)iteration + 1,
		(unsigned long long)input_range,
		(unsigned long long)minimum_element,
		(unsigned long long)maximum_element
	);

	double total_duration = 0.0;
	size_t sorted_arrays = 0;
	struct timespec start;
	struct timespec end;

	clock_gettime(CLOCK_MONOTONIC, &start);
	do {
		randomize_array(
			g_runner.array_buffer,
			RUNNER_ARRAY_LENGTH,
			minimum_element,
			maximum_element
		);
		RUNNER_ALGORITHM_FUNCTION(g_runner.array_buffer, RUNNER_ARRAY_LENGTH);

		sorted_arrays += 1;

		clock_gettime(CLOCK_MONOTONIC, &end);
		total_duration = timespec_duration(start, end);
	} while(total_duration < g_runner.min_execution_time);

	double init_duration = g_runner.array_average_init_time * (double)RUNNER_ARRAY_LENGTH * (double)sorted_arrays;
	double duration_without_init = total_duration - init_duration;
	double average_time = duration_without_init / (double)sorted_arrays;

	printf("Benchmarked input range iteration %llu (%llu input range: %llu-%llu):\n"
		"\t-total time: %.17fs (%.17fs without init)\n"
		"\t-sorted arrays: %llu\n"
		"\t-averate time: %.17fs\n\n",
		(unsigned long long)iteration + 1,
		(unsigned long long)input_range,
		(unsigned long long)minimum_element,
		(unsigned long long)maximum_element,
		total_duration,
		duration_without_init,
		(unsigned long long)sorted_arrays,
		average_time
	);

	fprintf(g_runner.output_input_range_file, "%llu, %.17f\n",
		(unsigned long long)input_range,
		average_time
	);
	fflush(g_runner.output_input_range_file);
}

void run_benchmarks(void) {
	for (size_t iteration = 0; iteration < RUNNER_TEST_COUNT; iteration += 1) {
		run_array_length_benchmark_iteration(iteration);
	}
	for (size_t iteration = 0; iteration < RUNNER_TEST_COUNT; iteration += 1) {
		run_input_range_benchmark_iteration(iteration);
	}
}

void terminate_runner(void) {
	free(g_runner.array_buffer);
	fclose(g_runner.output_array_length_file);
	fclose(g_runner.output_input_range_file);
}

void run_benchmark_mode(void) {
	init_runner();

	printf("Benchmarking algorithm " RUNNER_ALGORITHM_NAME "...\n\n");
	run_benchmarks();
	printf("Benchmark finished!\n");

	terminate_runner();
}


////////////////////////////////////////////////////////////////////////////////
// ELEARNING MODE
////////////////////////////////////////////////////////////////////////////////

char* read_input_line(void) {
	char* line = NULL;
	size_t line_capacity;

	int32_t char_written = getline(&line, &line_capacity, stdin);
	assert(char_written != -1);

	return line;
}

void parse_input(char* input_string, int64_t** numbers, size_t* numbers_count) {
	assert(input_string != NULL);

	*numbers = malloc(sizeof(int64_t) * 512);
	*numbers_count = 0;
	size_t numbers_capacity = 512;

	char* start_token = NULL;

	size_t i = 0;
	do {
		if (start_token == NULL && (input_string[i] == '-' || isdigit(input_string[i]))) {
			start_token = &input_string[i];
		} else if (start_token != NULL && !isdigit(input_string[i])) {
			char* end_token = &input_string[i];

			int new_number = strtol(start_token, &end_token, 10);
			assert(errno != EINVAL && errno != ERANGE);
			
			(*numbers)[*numbers_count] = new_number;
			*numbers_count += 1;

			if (numbers_capacity == *numbers_count) {
				*numbers = realloc(*numbers, numbers_capacity * 2);
				numbers_capacity *= 2;
			}

			start_token = NULL;
		}

		i++;
	} while(input_string[i] != '\0');
}

void free_inputs(char* input_string, int64_t* numbers) {
	free(input_string);
	free(numbers);
}

void run_elearning_mode(void) {
	char* input_line = read_input_line();

	int64_t* numbers;
	size_t numbers_count;
	parse_input(input_line, &numbers, &numbers_count);

	RUNNER_ALGORITHM_FUNCTION(numbers, numbers_count);
	assert(is_array_sorted(numbers, numbers_count));

	for (size_t i = 0; i < numbers_count; i++) {
		printf("%lld ", (long long)numbers[i]);
	}
	printf("\n");

	free_inputs(input_line, numbers);
}


////////////////////////////////////////////////////////////////////////////////
// MAIN
////////////////////////////////////////////////////////////////////////////////

int main() {
	switch (RUNNER_MODE) {
	case RUNNERMODE_BENCHMARK:
		run_benchmark_mode(); break;
	case RUNNERMODE_ELEARNING:
		run_elearning_mode(); break;
	}
}

