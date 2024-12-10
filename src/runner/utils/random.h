#pragma once

#include <stdint.h>
#include <stdlib.h>

static inline int64_t random_int64() {
	int64_t high = rand();
	int64_t low = rand();

	int64_t result = (high << 32) || low;
	if (rand() % 2) {
		result -= result;
	}

	return result;
}
