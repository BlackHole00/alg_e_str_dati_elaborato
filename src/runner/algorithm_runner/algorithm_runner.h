#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/**
	@enum Algorithm_Runner_Result
	@brief Specifies the resulting status of an operation related to the 
	`Algorithm_Runner`.
*/
typedef enum Algorithm_Runner_Result {
	/** @brief The related operation was successfull */
	ALGORITHM_RUNNER_RESULT_SUCCESS = 0,
	/**
		@brief The related operation failed because a provided function 
		parameter was invalid
	**/
	ALGORITHM_RUNNER_RESULT_INVALID_PARAMETER,
	/**
		@brief The related operation failed because the provided
		`Algorithm_Runner_Descriptor` is invalid
	*/
	ALGORITHM_RUNNER_RESULT_INVALID_DESCRIPTOR,
	/**
		@brief The related operation failed because an user provided
		`Algorithm` failed to sort an array
	*/
	ALGORITHM_RUNNER_RESULT_SORTING_ERROR,
	/**
		@brief The related operation failed because an IO operation
		(such as `fopen`) failed
	*/
	ALGORITHM_RUNNER_RESULT_IO_ERROR,
	/**
		@brief The related operation failed because a memory related
		operation (such as `malloc`) failed
	*/
	ALGORITHM_RUNNER_RESULT_MEMORY_ERROR,
} Algorithm_Runner_Result;

/**
	@struct Algorithm_Stats
	@brief Describes the statistics of a single test of an algorithm. It is 
	expected to be filled by the user provided callbacks of type
	`Algorithm_Callback`.
*/
typedef struct Algorithm_Stats {
	/** @brief The number of comparisions that happened in a given test */
	int64_t comparison_count;
	/** @brief The number of variable swaps that happened in a given test */
	int64_t swap_count;
	/** @brief The number of bytes allocated in a given test */
	int64_t bytes_allocated;
} Algorithm_Stats;

/**
	@typedef Algorithm_Callback
	@brief An user provided callback that, in combination with an
	`Algorithm`, defines an algorithm. Given an array and its size this
	callback must sort it, whilst writing the appropriate values into the
	provided statistics pointer.

	@param array The array to sort
	@param array_length The length of the array to sort
	@param statistics A pointer to the statistics to be written
*/
typedef void (*Algorithm_Callback)(int64_t* array, size_t array_length, Algorithm_Stats* statistics);

/**
	@typedef Element_Count_Incrementer
	@brief An user provided callback that defines how the number of element
	in the array to sort should be increased. It also determines when to
	stop running a series of tests.

	@param element_count A pointer to the current number of elements. Must
	be modified with the next element count.
	@return true if the tests should continue, false if the test should not
	continue
*/
typedef bool (*Element_Count_Incrementer)(size_t* element_count);

/**
	@struct Algorithm
	@brief Describes an algorithm to be tested and benchmarked
*/
typedef struct Algorithm {
	/**
		@brief The function to be benchmarked
		@note Must not be NULL.
	*/
	Algorithm_Callback algorithm_callback;

	/**
		@brief The name of the algorithm
		@note Must not be NULL.
	*/
	const char* name;
} Algorithm;

/**
	@struct Algorithm_Runner_Descriptor
	@brief Describes the configuration to use while initializing a
	determined `Algorithm_Runner`. All the fields in this structure are user
	defined and are expected to be valid for the entire `Algorithm_Runner`
	lifetime.
*/
typedef struct Algorithm_Runner_Descriptor {
	/**
		@brief The list of algorithm to test and benchmark. Its length
		is specified in `algorithms_count`.
		@note The list and its contents are expected to be valid for the
		entire lifetime of the related `Algorithm_Runner`.
		@note Must not be NULL.
	*/
	Algorithm* algorithms;

	/**
		@brief The number of algorithms present in `algorithms`
		@note Must be >= 1.
	*/
	size_t algorithms_count;

	/**
		@brief The function used to increment the element count between
		tests
		@note Must not be NULL.
	*/
	Element_Count_Incrementer element_count_incrementer;

	/**
		@brief The path to the average statistics output file. This is
		where the results of the tests with the same number of elements
		are put after they are averaged.
		@note Expected to be valid for the entire lifetime of the
		related `Algorithm_Runner`.
		@note Must not be NULL.
	*/
	const char* average_stats_output_file;

	/**
		@brief The path to the not averaged statistics output file
		@note Expected to be valid for the entire lifetime of the
		related	`Algorithm_Runner`.
		@note Must not be NULL.
	*/
	const char* single_test_stats_output_file;

	/**
		@brief The number of tests that must be executed per number of
		elements to sort
		@note Must be >= 1.
	*/
	uint64_t runs_per_test;
} Algorithm_Runner_Descriptor;

/**
	@struct Algorithm_Runner
	@brief Accordingly with the values of the related
	`Algorithm_Runner_Descriptor`, it tests and benchmarks the given
	algorithms.
	The results of the tests are written in the following files as csv:
		- single test data: Written to the file specified in the
		descriptor as `single_test_stats_output_file`. These statistics
		are not processed in any way.
		- averaged data: Written to the file specified in the descriptor
		as `average_stats_output_file`. These results are obtaining by
		averaging the tests that have been run with the same number of
		elements.
	In both files the results are written in sections, one for every
	algorithm. Each section is specified by a new empty line (or the start
	of the file) and another line containing the name of the algorithm
	related to the section.
	Inside a section, the data is written in the following formats:
		- single test data: 
			element count, run number, exection time in ns,
			comparisons, swaps, bytes allocated
		- averaged data
			element count, exection time in ns, comparisons, swaps,
			bytes allocated
*/
typedef struct Algorithm_Runner {
	/** @brief The list of algorithms to test and benchmark */
	Algorithm* algorithms;

	/** @brief The number of algorithms in `algorithm` */
	size_t algorithms_count;

	/**
		@brief The function used to increment the element count between 
		tests
	*/
	Element_Count_Incrementer element_count_incrementer;

	/** @brief Contains if anything has been written in the output files */
	bool outputs_did_write_first_line;

	/** @brief The handle to the single test data output file */
	FILE* single_test_stats_output_file;

	/** @brief The handle to the averaged test data output file */
	FILE* average_stats_output_file;

	/** 
		@brief The number of tests that must be executed per number of
		elements to sort
	*/
	uint64_t runs_per_test;
} Algorithm_Runner;

/**
	@fn algorithmrunner_create
	@brief Initializes an `Algorithm_Runner` with the provided descriptor

	@param runner A pointer to the runner to initialize
	@param descriptor The descriptor.
	@return One of the following:
	- `ALGORITHM_RUNNER_RESULT_SUCCESS` if no errors occurred,
	- `ALGORITHM_RUNNER_RESULT_INVALID_PARAMETER` if `runner` or 
		`descriptor` are NULL,
	- `ALGORITHM_RUNNER_RESULT_INVALID_DESCRIPTOR` if the requirements
		defined in the documentation of the `descriptor` are not
		fulfilled,
	- `ALGORITHM_RUNNER_RESULT_IO_ERROR` if it could not be possible to open
		the specified output files.

	@note `descriptor` must fullfill the requiremenets defined in its 
	documentation and must be valid for the entire lifetime of `runner`
*/
Algorithm_Runner_Result algorithmrunner_create(Algorithm_Runner* runner, const Algorithm_Runner_Descriptor* descriptor);

/**
	@fn algorithmrunner_destroy
	@brief Deinitializes an `Algorithm_Runner`

	@param runner The runner to deinitialize
*/
void algorithmrunner_destroy(const Algorithm_Runner* runner);

/**
	@fn algorithmrunner_run
	@brief Runs and benchmarks all the algorithms of a specified
	`Algorithm_Runner`

	@param runner The runner of which algorithms should be test
	@return One of the following:
	- `ALGORITHM_RUNNER_RESULT_SORTING_ERROR` if an algotithm fails to sort
		an array,
	- `ALGORITHM_RUNNER_RESULT_MEMORY_ERROR` if the runner could not
		allocate the memory necessary to run the tests.
*/

void quicksort(int64_t* array, size_t array_length, Algorithm_Stats* statistics);
void quicksort_3way(int64_t* array, size_t array_length, Algorithm_Stats* statistics);
void insertion_sort(int64_t* array, size_t array_length, Algorithm_Stats* statistics);

Algorithm_Runner_Result algorithmrunner_run(Algorithm_Runner* runner);
