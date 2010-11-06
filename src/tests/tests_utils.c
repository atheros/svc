#include <stdlib.h>
#include <stdio.h>
#include "tests_utils.h"

static const char* current_test = NULL;
static int assert_count = 0;

void test_start(const char* name) {
	printf("Testing %s... ", name);
	fflush(stdout);
	current_test = name;
	assert_count = 0;
}

static void test_failed(const char* condition, const char* file, int line) {
	fprintf(stderr, "%s failed! %s in %s:%i\n", current_test, condition, file, line);
	exit(1);
}

void __test_assert(int result, const char* condition, const char* file, int line) {
	assert_count++;
	if (!result) {
		test_failed(condition, file, line);
		exit(1);
	}
}

void test_end() {
	printf("done (%i assert(s) evaluated)\n", assert_count);
}

