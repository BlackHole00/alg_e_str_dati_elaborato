#pragma once

#include <stdint.h>
#include <stdlib.h>

/**
	@fn random_int64
	@brief Generates a random int64_t number
	@returns The generated number
*/
static inline int64_t random_int64() {
	int64_t high = rand();
	int64_t low = rand();

	int64_t result = (high << 32) | low;
	if (rand() % 2) {
		result -= result;
	}

	return result;
}
