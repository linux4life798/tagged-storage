#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include "tagged_storage.h"

#define TS_BUFFER_SIZE (6*1024)

#define ARRAY_SIZE(x) ( sizeof(x) / sizeof((x)[0]) )

void pattern_fill(void *data, size_t size) {
	char *c = data;
	size_t i;
	for (i = 0; i < size; i++) {
		*c++ = (char)i;
	}
}

bool pattern_check(void *data, size_t size) {
	char *c = data;
	size_t i;
	for (i = 0; i < size; i++) {
		if (*c++ != (char)i) {
			return false;
		}
	}
	return true;
}

void allocate_lots(struct ts *s) {
	size_t i;
	void *bufs[24];

	for (i = 0; i < ARRAY_SIZE(bufs); i++) {
		bufs[i] = ts_append(s, 2, 0, 128);
		pattern_fill(bufs[i], 128);
	}

	for (i = 0; i < ARRAY_SIZE(bufs); i++) {
		if(!pattern_check(bufs[i], 128)) {
			printf("# Error - Buffer %zu failed pattern check\n", i);
		}
	}
}

int main() {
	struct ts *s;
	s = malloc(TS_BUFFER_SIZE);
	if (!s) {
		perror("Failed to allocate buffer");
	}

	ts_reset(s, TS_BUFFER_SIZE);

	allocate_lots(s);

	printf("Hello\n");
	return 0;
}