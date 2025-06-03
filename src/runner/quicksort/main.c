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


////////////////////////////////////////////////////////////////////////////////
// CONFIG
////////////////////////////////////////////////////////////////////////////////

#define RUNNER_ALGORITHM_NAME "Quick Sort"
#define RUNNER_ALGORITHM_FUNCTION quicksort

enum Runner_Mode { RUNNERMODE_BENCHMARK, RUNNERMODE_ELEARNING };
#define RUNNER_MODE RUNNERMODE_BENCHMARK

#define RUNNER_MAX_RELATIVE_ERROR 0.0001
#define RUNNER_STARTING_ARRAY_LENGTH 100
#define RUNNER_ENDING_ARRAY_LENGTH 100000
#define RUNNER_MIN_ARRAY_ELEMENT 10
#define RUNNER_MAX_ARRAY_ELEMENT 1000000
#define RUNNER_TEST_COUNT 1000

#define RUNNER_OUTPUT_FILE "./results/quicksort.csv"


////////////////////////////////////////////////////////////////////////////////
// SORTING FUNCTION
////////////////////////////////////////////////////////////////////////////////

#include <stdint.h>
#include <stddef.h>

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

	double constant_a;
	double constant_b;

	int64_t* array_buffer;
	size_t array_buffer_size;

	FILE* output_file;
} g_runner;

size_t calculate_array_length(size_t iteration) {
	double b_power = pow(g_runner.constant_b, (double)iteration);
	return (size_t)(g_runner.constant_a * b_power);
}

int64_t calculate_random_array_element(void) {
	return RUNNER_MIN_ARRAY_ELEMENT + rand() % (RUNNER_MAX_ARRAY_ELEMENT - RUNNER_MIN_ARRAY_ELEMENT + 1);
}

bool is_array_sorted(int64_t* array, size_t array_count) {
	for (size_t i = 1; i < array_count; i++) {
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

void init_runner(void) {
	// Srand with seed 0 so it is deterministic
	srand(0);

	calculate_clock_precision();

	g_runner.constant_a = (double)RUNNER_STARTING_ARRAY_LENGTH;
	g_runner.constant_b = pow(
		(double)RUNNER_ENDING_ARRAY_LENGTH / (double)RUNNER_STARTING_ARRAY_LENGTH,
		1.0 / (double)(RUNNER_TEST_COUNT - 1)
	);
	printf("%f\n", g_runner.constant_a);
	printf("%f\n", g_runner.constant_b);

	g_runner.array_buffer = malloc(sizeof(int64_t) * RUNNER_ENDING_ARRAY_LENGTH);
	assert(g_runner.array_buffer != NULL);
	g_runner.array_buffer_size = RUNNER_ENDING_ARRAY_LENGTH;

	g_runner.output_file = fopen(RUNNER_OUTPUT_FILE, "w");
	assert(g_runner.output_file != NULL);
}

void run_benchmark_iteration(size_t iteration) {
	size_t array_length = calculate_array_length(iteration);

	printf("Benchmarking iteration %llu (%llu elements)...\n",
		(unsigned long long)iteration + 1,
		(unsigned long long)array_length
	);

	double total_duration = 0.0;
	size_t sorted_arrays = 0;
	do {
		for (size_t i = 0; i < array_length; i++) {
			g_runner.array_buffer[i] = calculate_random_array_element();
		}

		size_t max_element_index = rand() % array_length;
		g_runner.array_buffer[max_element_index] = RUNNER_MAX_ARRAY_ELEMENT;

		size_t min_element_index;
		do {
			min_element_index = rand() % array_length;
		} while(min_element_index == max_element_index);
		g_runner.array_buffer[min_element_index] = RUNNER_MIN_ARRAY_ELEMENT;

		struct timespec start;
		struct timespec end;

		clock_gettime(CLOCK_MONOTONIC, &start);
		RUNNER_ALGORITHM_FUNCTION(g_runner.array_buffer, array_length);
		clock_gettime(CLOCK_MONOTONIC, &end);

		assert(is_array_sorted(g_runner.array_buffer, array_length));

		total_duration += timespec_duration(start, end);
		sorted_arrays += 1;
	} while(total_duration < g_runner.min_execution_time);

	double average_time = total_duration / (double)sorted_arrays;

	printf("Benchmarked iteration %llu (%llu elements):\n"
		"\t-total time: %.17fs\n"
		"\t-sorted arrays: %llu\n"
		"\t-averate time: %.17fs\n\n",
		(unsigned long long)iteration + 1,
		(unsigned long long)array_length,
		total_duration,
		(unsigned long long)sorted_arrays,
		average_time
	);

	fprintf(g_runner.output_file, "%llu, %.17f\n",
		(unsigned long long)array_length,
		average_time
	);
	fflush(g_runner.output_file);
}

void run_benchmarks(void) {
	for (size_t iteration = 0; iteration < RUNNER_TEST_COUNT; iteration += 1) {
		run_benchmark_iteration(iteration);
	}
}

void terminate_runner(void) {
	free(g_runner.array_buffer);
	fclose(g_runner.output_file);
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


