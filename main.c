#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include "tagged_storage.h"

#define TS_BUFFER_SIZE (6*1024)

#define ARRAY_SIZE(x) ( sizeof(x) / sizeof((x)[0]) )

char pattern(void *data, size_t buffer_id, size_t index) {
	data = data;
	return (char)(index*index + buffer_id);
}

void pattern_fill(void *data, size_t size, size_t buffer_id) {
	char *c = data;
	size_t i;
	for (i = 0; i < size; i++) {
		c[i] = pattern(data, buffer_id, i);
	}
}

bool pattern_check(void *data, size_t size, size_t buffer_id) {
	char *c = data;
	size_t i;
	for (i = 0; i < size; i++) {
		if (c[i] != pattern(data, buffer_id, i)) {
			return false;
		}
	}
	return true;
}

void allocate_lots(struct ts *s, size_t block_size) {
	size_t i;
	void *bufs[24];

	for (i = 0; i < ARRAY_SIZE(bufs); i++) {
		printf("Appending %zu\n", i);
		bufs[i] = ts_append(s, 2, 0, block_size);
		if (bufs[i] == NULL) {
			printf("# Failed to append %zu\n", i);
			continue;
		}
		pattern_fill(bufs[i], block_size, i);
	}

	printf("\n");

	for (i = 0; i < ARRAY_SIZE(bufs); i++) {
		if (bufs[i] == NULL) {
			printf("Skipping %zu\n", i);
			continue;
		}
		printf("Checking %zu\n", i);
		if(!pattern_check(bufs[i], block_size, i)) {
			printf("# Error - Buffer %zu failed pattern check\n", i);
		}
	}

	printf("-----------------------------------------------\n");
	printf("\n");
}

void allocate_lots_with_find(struct ts *s, size_t block_size) {
	size_t i;
	void *bufs[24];

	for (i = 0; i < ARRAY_SIZE(bufs); i++) {
		ts_tag_t tag = (ts_tag_t)(i + 1);
		printf("Appending %zu\n", i);
		assert((size_t)(tag) == (i+1)); // make sure we don't overflow
		bufs[i] = ts_append(s, tag, 0, block_size);
		if (bufs[i] == NULL) {
			printf("# Failed to append %zu\n", i);
			continue;
		}
		pattern_fill(bufs[i], block_size, tag);
	}

	printf("\n");

	for (i = 0; i < ARRAY_SIZE(bufs); i++) {
		ts_tag_t tag = (ts_tag_t)(i + 1);
		void *data;
		if (bufs[i] == NULL) {
			printf("Skipping %zu\n", i);
			continue;
		}
		
		printf("Checking %zu\n", i);
		data = ts_find_next(s, tag, NULL,NULL, NULL);
		if (data == NULL) {
			printf("# Failed to find %zu\n", i);
			continue;
		}
		if(!pattern_check(data, block_size, tag)) {
			printf("# Error - Buffer %zu failed pattern check\n", i);
		}
	}

	printf("-----------------------------------------------\n");
	printf("\n");
}

void test_allocations() {
	struct ts *s;
	s = malloc(TS_BUFFER_SIZE);
	if (!s) {
		perror("Failed to allocate buffer");
	}

	ts_reset(s, TS_BUFFER_SIZE);
	allocate_lots(s, 128);
	allocate_lots(s, 128);

	free(s);
}

void test_find() {
	struct ts *s;
	s = malloc(TS_BUFFER_SIZE);
	if (!s) {
		perror("Failed to allocate buffer");
	}

	ts_reset(s, TS_BUFFER_SIZE);
	allocate_lots_with_find(s, 128);
	allocate_lots_with_find(s, 128);

	free(s);
}

int main() {

	printf("# Test Allocations #\n\n");
	test_allocations();

	printf("# Test Finds #\n\n");
	test_find();

	printf("Hello\n");
	return 0;
}