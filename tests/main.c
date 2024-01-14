#include <munit/munit.h>
#define EECS_IMPLEMENTATION
#include <eecs.h>

extern MunitSuite basic;

int main (int argc, char* argv[]) {
	MunitSuite suites = {
		.suites = (MunitSuite[]) {
			basic,
			{ 0 },
		},
	};

	return munit_suite_main(&suites, NULL, argc, argv);
}
