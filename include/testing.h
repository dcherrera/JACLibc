/* (c) 2025 FRINKnet & Friends – MIT licence */
#ifndef TESTING_H
#define TESTING_H
#pragma once

/**
 * INSTRUCTIONS:
 *
 *   TEST_TYPE(name)              an identifier displayed in the front end
 *   TEST_UNIT(filename.h)        filename of the unit you are testing
 *   TEST_SUITE(function)         name of the specific function you are testing
 *   TEST(function_specific_test) {  name of specific thing you are checking
 *     TEST_SKIP("reason");       skip tests that segfault until you can fix
 *     TEST_INFO("essay");        add as much explanation as you want
 *     code
 *     ASSERT...                  run whatever assertions you need
 *   }
 *
 *   TEST_MAIN()                  use this instead of defining main yourself
 *
 * ASSERTIONS:
 *
 *   Boolean:
 *     ASSERT_TRUE(x)                                      - value is true
 *     ASSERT_FALSE(x)                                     - value is false
 *
 *   Comparisons:
 *     ASSERT_EQ(a, b)                                     - equal (alias for ASSERT_INT_EQ)
 *     ASSERT_NE(a, b)                                     - not equal
 *     ASSERT_GT(a, b)                                     - greater than
 *     ASSERT_LT(a, b)                                     - less than
 *     ASSERT_GE(a, b)                                     - greater or equal
 *     ASSERT_LE(a, b)                                     - less or equal
 *
 *   Strings Comparisons:
 *     ASSERT_STR_EQ(expected, actual)                     - strings equal
 *     ASSERT_STR_NE(expected, actual)                     - strings not equal
 *     ASSERT_STR_PRE(prefix, actual)                      - string starts with prefix
 *     ASSERT_STR_SUF(suffix, actual)                      - string ends with suffix
 *     ASSERT_STR_HAS(needle, actual)                      - string contains substring
 *
 *   Numeric Comparisons:
 *     ASSERT_INT_EQ/NE/GT/LT/GE/LE(a, b)                  - explicit integer comparison
 *     ASSERT_FLT_EQ/NE/GT/LT/GE/LE(a, b)                  - float comparisons
 *     ASSERT_DBL_EQ/NE/GT/LT/GE/LE(a, b)                  - double comparisons
 *     ASSERT_LDBL_EQ/NE/GT/LT/GE/LE(a, b)                 - long double comparisons
 *
 *   Tolerant Comparisons:
 *     ASSERT_FLT_NEAR(expected, actual, tolerance)        - float within tolerance
 *     ASSERT_DBL_NEAR(expected, actual, tolerance)        - double within tolerance
 *     ASSERT_LDBL_NEAR(expected, actual, tolerance)       - long double within tolerance
 *
 *   Complex Comparisons:
 *     ASSERT_FLT_COMPLEX(real, imag, actual, tolerance    - float complex numbers
 *     ASSERT_DBL_COMPLEX(real, imag, actual, tolerance)   - double complex numbers
 *     ASSERT_LDBL_COMPLEX(real, imag, actual, tolerance)  - long complex numbers
 *
 *   Memory Comparisons:
 *     ASSERT_MEM_EQ(expected, actual, size)               - memory regions equal
 *
 *   Pointer Comparisons:
 *     ASSERT_NULL(ptr)                                    - pointer is NULL
 *     ASSERT_NOT_NULL(ptr)                                - pointer is not NULL
 *
 *   Special Values:
 *     ASSERT_NAN(x)                                       - value is NaN
 *     ASSERT_INF(x)                                       - value is infinity
 *     ASSERT_FINITE(x)                                    - value is finite
 *
 *   Errno Checks:
 *     ASSERT_ERRNO_EQ(err)                                - errno equals expected value
 */

#include <config.h>
#include <stdio.h>   // fprintf(), printf(), puts()
#include <stdlib.h>  // exit()
#include <string.h>  // strcmp(), memset()
#include <stdarg.h>  // va_arg()
#include <inttypes.h>  // PRIdMAX, PRIuMAX
#include <math.h>

/* ============================================================= */
/* Configuration                                                 */
/* ============================================================= */
#ifndef JACL_MAX_TESTS
#define JACL_MAX_TESTS 1000
#endif
#ifndef JACL_MAX_SUITES
#define JACL_MAX_SUITES 100
#endif

/* ============================================================= */
/* Data Structures                                               */
/* ============================================================= */
typedef void (*test_func_t)(void);

typedef struct {
  int total;
  int passed;
  int failed;
  int skipped;
} test_stats_t;

typedef struct {
  const char* name;
  test_func_t func;
} test_t;

typedef struct {
	const char* type;
	const char* unit;
	const char* suite;
	test_t tests[JACL_MAX_TESTS];
	int test_count;
	test_stats_t stats;
} test_suite_t;

static test_stats_t __jacl_test_stats         = {0,0,0,0};
static int          __jacl_test_failed        = 0;
static int          __jacl_test_skipped       = 0;
static int          __jacl_test_errored       = 0;
static char         __jacl_test_error[2048]   = {0};
static int          __jacl_show_only_failed   = 0;
static int          __jacl_show_only_skipped  = 0;
static int          __jacl_test_suite_count   = 0;
static test_suite_t __jacl_test_suites[JACL_MAX_SUITES];

/* ============================================================= */
/* ANSI Colour Helper                                            */
/* ============================================================= */
#define COLOR_HEAD   "\x1b[39m"
#define COLOR_TEST   "\x1b[93m"
#define COLOR_PASS   "\x1b[92m"
#define COLOR_FAIL   "\x1b[31m"
#define COLOR_SKIP   "\x1b[36m"
#define COLOR_NORM   "\x1b[0m"

static inline int __jacl_test_color(void) {
  static int checked = 0, has = 0;

  if (!checked) {
    const char *t = getenv("TERM");

    has = t && strcmp(t, "dumb");
    checked = 1;
  }

  return has;
}

/* ============================================================= */
/* Registration Internals                                        */
/* ============================================================= */
static char *__jacl_test_type = "unit";
static char *__jacl_test_unit = "test";
static char *__jacl_test_suite = "default";

static inline void __jacl_test_register(const char *test_name, test_func_t func) {
	test_suite_t *suite = NULL;

	for (int i = 0; i < __jacl_test_suite_count; i++) {
		if (!strcmp(__jacl_test_suites[i].type, __jacl_test_type) &&
		    !strcmp(__jacl_test_suites[i].unit, __jacl_test_unit) &&
		    !strcmp(__jacl_test_suites[i].suite, __jacl_test_suite)) {
			suite = &__jacl_test_suites[i];

			break;
		}
	}

	if (!suite) {
		if (__jacl_test_suite_count >= JACL_MAX_SUITES) {
			fprintf(stderr, "Error: too many suites (>%d)\n", JACL_MAX_SUITES);

			exit(1);
		}

		suite = &__jacl_test_suites[__jacl_test_suite_count++];
		suite->type = __jacl_test_type;
		suite->unit = __jacl_test_unit;
		suite->suite = __jacl_test_suite;
		suite->test_count = 0;
		suite->stats = (test_stats_t){0, 0, 0, 0};
	}

	if (suite->test_count >= JACL_MAX_TESTS) {
		fprintf(stderr, "Error: too many tests in suite '%s: %s' (>%d)\n", suite->unit, suite->suite, JACL_MAX_TESTS);
		exit(1);
	}

	suite->tests[suite->test_count].name = test_name;
	suite->tests[suite->test_count].func = func;
	suite->test_count++;
}

/* ============================================================= */
/* Runner Internals                                              */
/* ============================================================= */
static inline void __jacl_test_run(test_suite_t *suite, test_t *t) {
	int print_this = 1;

	__jacl_test_failed  = 0;
	__jacl_test_errored = 0;
	__jacl_test_skipped = 0;
	__jacl_test_stats.total++;
	suite->stats.total++;
	errno = 0;

	memset(__jacl_test_error, 0, sizeof(__jacl_test_error));

	// Filter output: only print matching tests if filter is set
	if (__jacl_show_only_failed || __jacl_show_only_skipped) print_this = 0;

	t->func();

	// Evaluate result for filtered output
	if (__jacl_show_only_failed && __jacl_test_failed) print_this = 1;
	if (__jacl_show_only_skipped && __jacl_test_skipped) print_this = 1;
	if (print_this) printf("  %s%-60s%s", __jacl_test_color() ? COLOR_TEST : "", t->name, __jacl_test_color() ? COLOR_NORM : "");

	// Output and stats
	if (__jacl_test_skipped) {
		if (print_this) printf("%s[SKIP]%s\n", __jacl_test_color() ? COLOR_SKIP : "", __jacl_test_color() ? COLOR_NORM : "");
		if (print_this) printf("\n%s%s%s\n\n", __jacl_test_color() ? COLOR_SKIP : "", __jacl_test_error, __jacl_test_color() ? COLOR_NORM : "");

		__jacl_test_stats.skipped++;
		suite->stats.skipped++;
	} else if (__jacl_test_failed) {
		if (print_this) printf("%s[FAIL]%s\n", __jacl_test_color() ? COLOR_FAIL : "", __jacl_test_color() ? COLOR_NORM : "");
		if (__jacl_test_errored && print_this) printf("\n%s%s%s\n\n", __jacl_test_color() ? COLOR_FAIL : "", __jacl_test_error, __jacl_test_color() ? COLOR_NORM : "");

		__jacl_test_stats.failed++;
		suite->stats.failed++;
	} else {
		if (print_this) printf("%s[PASS]%s\n", __jacl_test_color() ? COLOR_PASS : "", __jacl_test_color() ? COLOR_NORM : "");

		__jacl_test_stats.passed++;
		suite->stats.passed++;
	}
}
static inline int __jacl_test_only(const char *test_name) {
	for (int i = 0; i < __jacl_test_suite_count; i++) {
		test_suite_t *suite = &__jacl_test_suites[i];

		for (int j = 0; j < suite->test_count; j++) {
			if (!strcmp(suite->tests[j].name, test_name)) {
  			printf("\n  %sRunning only '%s' test…%s\n",
				       __jacl_test_color() ? COLOR_TEST : "",
							 test_name,
				       __jacl_test_color() ? COLOR_NORM : "");
				printf("  %s%s - %s: %s test%s\n\n",
				       __jacl_test_color() ? COLOR_HEAD : "",
				       suite->suite, suite->unit, suite->type,
				       __jacl_test_color() ? COLOR_NORM : "");
				__jacl_test_run(suite, &suite->tests[j]);

				return __jacl_test_stats.failed > 0;
			}
		}
	}

  printf("Test '%s' not found\n", test_name);

  return 1;
}
static inline int __jacl_test_each(const char *suite_name) {
	test_suite_t *suite = NULL;

	for (int i = 0; i < __jacl_test_suite_count; i++) {
		if (!strcmp(__jacl_test_suites[i].suite, suite_name)) { suite = &__jacl_test_suites[i]; break; }
	}

	if (!suite) { printf("  Suite '%s' not found\n", suite_name); return 1; }

  printf("\n  %sRunning '%s' suite…%s\n",
	       __jacl_test_color() ? COLOR_TEST : "",
				 suite_name,
	       __jacl_test_color() ? COLOR_NORM : "");

	printf("  %s%s - %s: %s tests%s\n\n",
	       __jacl_test_color() ? COLOR_HEAD : "",
	       suite->suite, suite->unit, suite->type,
	       __jacl_test_color() ? COLOR_NORM : "");

	for (int i = 0; i < suite->test_count; i++) __jacl_test_run(suite, &suite->tests[i]);

	return __jacl_test_stats.failed > 0;
}
static inline int __jacl_test_all(void) {
  if (__jacl_test_suite_count == 0) { puts("No tests registered."); return 0; }

  int total = 0;

  for (int i = 0; i < __jacl_test_suite_count; i++) total += __jacl_test_suites[i].test_count;

  // Add blank line before, color the "Running" message like test names (blue)
  printf("\n  %sRunning all %d test%s…%s\n",
         __jacl_test_color() ? COLOR_TEST : "",
         total,
         total == 1 ? "" : "s",
         __jacl_test_color() ? COLOR_NORM : "");

	if (__jacl_show_only_failed || __jacl_show_only_skipped) printf("\n");

  for (int i = 0; i < __jacl_test_suite_count; i++) {
    test_suite_t *suite = &__jacl_test_suites[i];

    // Add blank line before header, keep header on its own line
    if (!__jacl_show_only_failed && !__jacl_show_only_skipped) printf("\n  %s%s - %s: %s tests%s\n\n",
           __jacl_test_color() ? COLOR_HEAD : "",
           suite->suite, suite->unit, suite->type,
           __jacl_test_color() ? COLOR_NORM : "");

    for (int j = 0; j < suite->test_count; j++) __jacl_test_run(suite, &suite->tests[j]);
  }

  return __jacl_test_stats.failed > 0;
}

/* ============================================================= */
/* Print Summary                                                 */
/* ============================================================= */
static inline void __jacl_test_report(void) {
  if (!__jacl_test_stats.total) { puts("No tests run."); return; }

  printf("\n%s\n",
    __jacl_test_stats.failed
    ? (__jacl_test_color() ? COLOR_FAIL
      "  ❌ SOME CHECKS FAILED ❌" COLOR_NORM
      : "  ❌ SOME CHECKS FAILED ❌")
    : (__jacl_test_color() ? COLOR_PASS
      "  🎉 ALL CHECKS PASSED 🎉" COLOR_NORM
      : "  🎉 ALL CHECKS PASSED 🎉")
  );

  printf("\n  %s%d total   %s%d passed   %s%d skipped   %s%d failed%s\n\n",
    __jacl_test_color() ? COLOR_TEST : "",
    __jacl_test_stats.total,
    __jacl_test_color() ? COLOR_PASS : "",
		__jacl_test_stats.passed,
    __jacl_test_color() ? COLOR_SKIP : "",
		__jacl_test_stats.skipped,
    __jacl_test_color() ? COLOR_FAIL : "",
    __jacl_test_stats.failed,
    __jacl_test_color() ? COLOR_NORM : ""
  );
}

/* ============================================================= */
/* Test Assertions                                               */
/* ============================================================= */
#define ASSERT_TRUE(x) do { \
	if (!(x)) { \
		TEST_FAIL("\tFAIL: %s:%d - Value should be true\n" \
		          "\tTEST: %s", __FILE__, __LINE__, #x); \
	} \
} while(0)

#define ASSERT_FALSE(x) do { \
	if (x) { \
		TEST_FAIL("\tFAIL: %s:%d\n - Value should be false\n" \
		          "\tTEST: %s", __FILE__, __LINE__, #x); \
	} \
} while(0)

#define ASSERT_EQ(a, b)  ASSERT_INT_EQ(a, b)
#define ASSERT_NE(a, b)  ASSERT_INT_NE(a, b)
#define ASSERT_GT(a, b)  ASSERT_INT_GT(a, b)
#define ASSERT_LT(a, b)  ASSERT_INT_LT(a, b)
#define ASSERT_GE(a, b)  ASSERT_INT_GE(a, b)
#define ASSERT_LE(a, b)  ASSERT_INT_LE(a, b)

#define ASSERT_STR_EQ(expected, actual) do { \
	if (!expected || !actual || strcmp((expected), (actual)) != 0) { \
		TEST_FAIL("\tFAIL: %s:%d - Strings not equal\n" \
		          "\tTEST: \"%s\" is \"%s\"", \
		          __FILE__, __LINE__, \
		          (actual) ? (actual) : "(null)", \
		          (expected) ? (expected) : "(null)"); \
	} \
} while(0)

#define ASSERT_STR_NE(expected, actual) do { \
	if (expected && actual && strcmp((expected), (actual)) == 0) { \
		TEST_FAIL("\tFAIL: %s:%d - Strings should not match\n" \
		          "\tTEST: \"%s\" is not \"%s\"", \
		          __FILE__, __LINE__, \
		          (actual) ? (actual) : "(null)", \
		          (expected) ? (expected) : "(null)"); \
	} \
} while(0)

#define ASSERT_STR_PRE(prefix, actual) do { \
	if (!prefix || !actual || strncmp((actual), (prefix), strlen(prefix)) != 0) { \
		TEST_FAIL("\tFAIL: %s:%d - String should have prefix\n" \
		          "\tTEST: \"%s\" starts with \"%s\"", \
		          __FILE__, __LINE__, \
		          (actual) ? (actual) : "(null)", \
		          (prefix) ? (prefix) : "(null)"); \
	} \
} while(0)

#define ASSERT_STR_SUF(suffix, actual) do { \
	size_t __test_suf_len = suffix ? strlen(suffix) : 0; \
	size_t __test_act_len = actual ? strlen(actual) : 0; \
	if (!suffix || !actual || __test_act_len < __test_suf_len || \
	    strcmp((actual) + __test_act_len - __test_suf_len, (suffix)) != 0) { \
		TEST_FAIL("\tFAIL: %s:%d - String should have suffix\n" \
		          "\tTEST: \"%s\" ends with \"%s\"", \
		          __FILE__, __LINE__, \
		          (actual) ? (actual) : "(null)", \
		          (suffix) ? (suffix) : "(null)"); \
	} \
} while(0)


#define ASSERT_STR_HAS(needle, actual) do { \
	if (!needle || !actual || strstr((actual), (needle)) == NULL) { \
		TEST_FAIL("\tFAIL: %s:%d - String should contain substring\n" \
		          "\tTEST: \"%s\" includes \"%s\"", \
		          __FILE__, __LINE__, \
		          (actual) ? (actual) : "(null)", \
		          (needle) ? (needle) : "(null)"); \
	} \
} while(0)

#define ASSERT_ERRNO_EQ(err) do { \
	if (!((errno) == (err))) { \
		TEST_FAIL("\tFAIL: %s:%d - Error should be set\n" \
		          "\tTEST: error %d is %d", __FILE__, __LINE__, (errno), (err)); \
	} \
} while(0)

#define ASSERT_INT_EQ(a, b) do { \
	if (!((long)(a) == (long)(b))) { \
		TEST_FAIL("\tFAIL: %s:%d - Values should be equal\n" \
		          "\tTEST: %ld == %ld", __FILE__, __LINE__, (long)(a), (long)(b)); \
	} \
} while(0)

#define ASSERT_INT_GT(a, b) do { \
	if (!((long)(a) > (long)(b))) { \
		TEST_FAIL("\tFAIL: %s:%d - Values should be more\n" \
		          "\tTEST: %ld > %ld", __FILE__, __LINE__, (long)(a), (long)(b)); \
	} \
} while(0)

#define ASSERT_INT_LT(a, b) do { \
	if (!((long)(a) < (long)(b))) { \
		TEST_FAIL("\tFAIL: %s:%d - Values should be less\n" \
		          "\tTEST: %ld < %ld", __FILE__, __LINE__, (long)(a), (long)(b)); \
	} \
} while(0)

#define ASSERT_INT_GE(a, b) do { \
	if (!((long)(a) >= (long)(b))) { \
		TEST_FAIL("\tFAIL: %s:%d - Values shouldn't be less\n" \
		          "\tTEST: %ld >= %ld", __FILE__, __LINE__, (long)(a), (long)(b)); \
	} \
} while(0)

#define ASSERT_INT_LE(a, b) do { \
	if (!((long)(a) <= (long)(b))) { \
		TEST_FAIL("\tFAIL: %s:%d - Values shouldn't be more\n" \
		          "\tTEST: %ld <= %ld", __FILE__, __LINE__, (long)(a), (long)(b)); \
	} \
} while(0)

#define ASSERT_INT_NE(a, b) do { \
	if (!((long)(a) != (long)(b))) { \
		TEST_FAIL("\tFAIL: %s:%d - Values should be different\n" \
		          "\tTEST: %ld != %ld", __FILE__, __LINE__, (long)(a), (long)(b)); \
	} \
} while(0)

#define ASSERT_NAN(x) do { \
	if (!isnan(x)) { \
		TEST_FAIL("\tFAIL: %s:%d - Expected NaN\n" \
		          "\tTEST: isnan(%Lf)", __FILE__, __LINE__, (long double)(x)); \
	} \
} while(0)

#define ASSERT_INF(x) do { \
	if (!isinf(x)) { \
		TEST_FAIL("\tFAIL: %s:%d - Expected Infinity\n" \
		          "\tTEST: isinf(%Lg)", __FILE__, __LINE__, (long double)(x)); \
	} \
} while(0)

#define ASSERT_FINITE(x) do { \
	if (!isfinite(x)) { \
		TEST_FAIL("\tFAIL: %s:%d - Expected finite value\n" \
		          "\tTEST: isfinite(%Lg)", __FILE__, __LINE__, (long double)(x)); \
	} \
} while(0)

#define ASSERT_NULL(ptr) do { \
	if ((ptr) != NULL) { \
		TEST_FAIL("\tFAIL: %s:%d - Expected NULL pointer\n" \
		          "\tTEST: %p == NULL", __FILE__, __LINE__, (ptr)); \
	} \
} while(0)

#define ASSERT_NOT_NULL(ptr) do { \
	if ((ptr) == NULL) { \
		TEST_FAIL("\tFAIL: %s:%d - Expected non-NULL pointer\n" \
		          "\tTEST: NULL != NULL", __FILE__, __LINE__); \
	} \
} while(0)

#define ASSERT_FLT_EQ(a, b) do { \
	if (!((float)(a) == (float)(b))) { \
		TEST_FAIL("\tFAIL: %s:%d - Values should be equal\n" \
		          "\tTEST: %g == %g", __FILE__, __LINE__, (float)(a), (float)(b)); \
	} \
} while(0)

#define ASSERT_FLT_GT(a, b) do { \
	if (!((float)(a) > (float)(b))) { \
		TEST_FAIL("\tFAIL: %s:%d - Values should be more\n" \
		          "\tTEST: %g > %g", __FILE__, __LINE__, (float)(a), (float)(b)); \
	} \
} while(0)

#define ASSERT_FLT_LT(a, b) do { \
	if (!((float)(a) < (float)(b))) { \
		TEST_FAIL("\tFAIL: %s:%d - Values should be less\n" \
		          "\tTEST: %g < %g", __FILE__, __LINE__, (float)(a), (float)(b)); \
	} \
} while(0)

#define ASSERT_FLT_GE(a, b) do { \
	if (!((float)(a) >= (float)(b))) { \
		TEST_FAIL("\tFAIL: %s:%d - Values should be more\n" \
		          "\tTEST: %g >= %g", __FILE__, __LINE__, (float)(a), (float)(b)); \
	} \
} while(0)

#define ASSERT_FLT_LE(a, b) do { \
	if (!((float)(a) <= (float)(b))) { \
		TEST_FAIL("\tFAIL: %s:%d - Values should be less\n" \
		          "\tTEST: %g <= %g", __FILE__, __LINE__, (float)(a), (float)(b)); \
	} \
} while(0)

#define ASSERT_FLT_NE(a, b) do { \
	if (!((float)(a) != (float)(b))) { \
		TEST_FAIL("\tFAIL: %s:%d - Values should be different\n" \
		          "\tTEST: %g != %g", __FILE__, __LINE__, (float)(a), (float)(b)); \
	} \
} while(0)

#define ASSERT_FLT_NEAR(expected, actual, tolerance) do { \
	float __test_diff = (float)(expected) - (float)(actual); \
	if (__test_diff < 0) __test_diff = -__test_diff; \
	if (__test_diff > (tolerance)) { \
		TEST_FAIL("\tFAIL: %s:%d - not within tolerance\n" \
		          "\tTEST: %e ≈ %e\n" \
		          "\tWITH: ϵ = %e, Δ = %e", \
		          __FILE__, __LINE__, (actual), (expected), (double)(tolerance), __test_diff); \
	} \
} while(0)

#define ASSERT_FLT_COMPLEX(expt_real, expt_imgn, actual, tolerance) do { \
	float __test_r = creal(actual); \
	float __test_i = cimag(actual); \
	float __test_diff_r = (float)(expt_real) - __test_r; \
	float __test_diff_i = (float)(expt_imgn) - __test_i; \
	if (__test_diff_r < 0) __test_diff_r = -__test_diff_r; \
	if (__test_diff_i < 0) __test_diff_i = -__test_diff_i; \
	if (__test_diff_r > (tolerance) || __test_diff_i > (tolerance)) { \
		TEST_FAIL("\tFAIL: %s:%d - complex not in tolerance\n" \
		          "\tTEST: %.6fr%+.6fi ≈ %.6fr%+.6fi\n" \
		          "\tWITH: ϵ = %e, Δr = %e, Δi = %e", \
		          __FILE__, __LINE__, __test_r, __test_i, \
		          (float)(expt_real), (float)(expt_imgn), \
		          (float)(tolerance), __test_diff_r, __test_diff_i); \
	} \
} while(0)

#define ASSERT_DBL_EQ(a, b) do { \
	if (!((double)(a) == (double)(b))) { \
		TEST_FAIL("\tFAIL: %s:%d - Values should be equal\n" \
		          "\tTEST: %lg == %lg", __FILE__, __LINE__, (double)(a), (double)(b)); \
	} \
} while(0)

#define ASSERT_DBL_GT(a, b) do { \
	if (!((double)(a) > (double)(b))) { \
		TEST_FAIL("\tFAIL: %s:%d - Values should be more\n" \
		          "\tTEST: %lg > %lg", __FILE__, __LINE__, (double)(a), (double)(b)); \
	} \
} while(0)

#define ASSERT_DBL_LT(a, b) do { \
	if (!((double)(a) < (double)(b))) { \
		TEST_FAIL("\tFAIL: %s:%d - Values should be less\n" \
		          "\tTEST: %lg < %lg", __FILE__, __LINE__, (double)(a), (double)(b)); \
	} \
} while(0)

#define ASSERT_DBL_GE(a, b) do { \
	if (!((double)(a) >= (double)(b))) { \
		TEST_FAIL("\tFAIL: %s:%d - Values should be more\n" \
		          "\tTEST: %lg >= %lg", __FILE__, __LINE__, (double)(a), (double)(b)); \
	} \
} while(0)

#define ASSERT_DBL_LE(a, b) do { \
	if (!((double)(a) <= (double)(b))) { \
		TEST_FAIL("\tFAIL: %s:%d - Values should be less\n" \
		          "\tTEST: %lg <= %lg", __FILE__, __LINE__, (double)(a), (double)(b)); \
	} \
} while(0)

#define ASSERT_DBL_NE(a, b) do { \
	if (!((double)(a) != (double)(b))) { \
		TEST_FAIL("\tFAIL: %s:%d - Values should be different\n" \
		          "\tTEST: %lg != %lg", __FILE__, __LINE__, (double)(a), (double)(b)); \
	} \
} while(0)

#define ASSERT_DBL_NEAR(expected, actual, tolerance) do { \
	double __test_diff = (double)(expected) - (double)(actual); \
	if (__test_diff < 0) __test_diff = -__test_diff; \
	if (__test_diff > (tolerance)) { \
		TEST_FAIL("\tFAIL: %s:%d - not within tolerance\n" \
		          "\tTEST: %le ≈ %le\n" \
		          "\tWITH: ϵ = %le, Δ = %le", \
		          __FILE__, __LINE__, (actual), (expected), (double)(tolerance), __test_diff); \
	} \
} while(0)

#define ASSERT_DBL_COMPLEX(expt_real, expt_imgn, actual, tolerance) do { \
	double __test_r = creal(actual); \
	double __test_i = cimag(actual); \
	double __test_diff_r = (double)(expt_real) - __test_r; \
	double __test_diff_i = (double)(expt_imgn) - __test_i; \
	if (__test_diff_r < 0) __test_diff_r = -__test_diff_r; \
	if (__test_diff_i < 0) __test_diff_i = -__test_diff_i; \
	if (__test_diff_r > (tolerance) || __test_diff_i > (tolerance)) { \
		TEST_FAIL("\tFAIL: %s:%d - complex not in tolerance\n" \
		          "\tTEST: %.6lfr%+.6lfi ≈ %.6lfr%+.6lfi\n" \
		          "\tWITH: ϵ = %le, Δr = %le, Δi = %le", \
		          __FILE__, __LINE__, __test_r, __test_i, \
		          (double)(expt_real), (double)(expt_imgn), \
		          (double)(tolerance), __test_diff_r, __test_diff_i); \
	} \
} while(0)

#define ASSERT_LDBL_EQ(a, b) do { \
	if (!((long double)(a) == (long double)(b))) { \
		TEST_FAIL("\tFAIL: %s:%d - Values should be equal\n" \
		          "\tTEST: %Lf == %Lf", __FILE__, __LINE__, (long double)(a), (long double)(b)); \
	} \
} while(0)

#define ASSERT_LDBL_GT(a, b) do { \
	if (!((long double)(a) > (long double)(b))) { \
		TEST_FAIL("\tFAIL: %s:%d - Values should be more\n" \
		          "\tTEST: %Lg > %Lg", __FILE__, __LINE__, (long double)(a), (long double)(b)); \
	} \
} while(0)

#define ASSERT_LDBL_LT(a, b) do { \
	if (!((long double)(a) < (long double)(b))) { \
		TEST_FAIL("\tFAIL: %s:%d - Values should be less\n" \
		          "\tTEST: %Lf < %Lf", __FILE__, __LINE__, (long double)(a), (long double)(b)); \
	} \
} while(0)

#define ASSERT_LDBL_GE(a, b) do { \
	if (!((long double)(a) >= (long double)(b))) { \
		TEST_FAIL("\tFAIL: %s:%d - Values should be more\n" \
		          "\tTEST: %Lf >= %Lf", __FILE__, __LINE__, (long double)(a), (long double)(b)); \
	} \
} while(0)

#define ASSERT_LDBL_LE(a, b) do { \
	if (!((long double)(a) <= (long double)(b))) { \
		TEST_FAIL("\tFAIL: %s:%d - Values should be less\n" \
		          "\tTEST: %Lf <= %Lf", __FILE__, __LINE__, (long double)(a), (long double)(b)); \
	} \
} while(0)

#define ASSERT_LDBL_NE(a, b) do { \
	if (!((long double)(a) != (long double)(b))) { \
		TEST_FAIL("\tFAIL: %s:%d - Values should be different\n" \
		          "\tTEST: %Lf != %Lf", __FILE__, __LINE__, (long double)(a), (long double)(b)); \
	} \
} while(0)

#define ASSERT_LDBL_NEAR(expected, actual, tolerance) do { \
	long double __test_diff = (long double)(expected) - (long double)(actual); \
	if (__test_diff < 0) __test_diff = -__test_diff; \
	if (__test_diff > (long double)(tolerance)) { \
		TEST_FAIL("\tFAIL: %s:%d - not within tolerance\n" \
		          "\tTEST: %Lg ≈ %Lg\n" \
		          "\tWITH: ϵ = %Le, Δ = %Le", \
		          __FILE__, __LINE__, (long double)(actual), (long double)(expected), (long double)(tolerance), __test_diff); \
	} \
} while(0)

#define ASSERT_LDBL_COMPLEX(expt_real, expt_imgn, actual, tolerance) do { \
	long double __test_r = creal(actual); \
	long double __test_i = cimag(actual); \
	long double __test_diff_r = (long double)(expt_real) - __test_r; \
	long double __test_diff_i = (long double)(expt_imgn) - __test_i; \
	if (__test_diff_r < 0) __test_diff_r = -__test_diff_r; \
	if (__test_diff_i < 0) __test_diff_i = -__test_diff_i; \
	if (__test_diff_r > (tolerance) || __test_diff_i > (tolerance)) { \
		TEST_FAIL("\tFAIL: %s:%d - complex not in tolerance\n" \
		          "\tTEST: %.6Lgfr%+.6Lgfi ≈ %.6Lgfr%+.6Lgfi\n" \
		          "\tWITH: ϵ = %Lf, Δr = %Lf, Δi = %Lf", \
		          __FILE__, __LINE__, __test_r, __test_i, \
		          (long double)(expt_real), (long double)(expt_imgn), \
		          (long double)(tolerance), __test_diff_r, __test_diff_i); \
	} \
} while(0)

#define ASSERT_MEM_EQ(expected, actual, size) do { \
	if (memcmp((expected), (actual), (size)) != 0) { \
		TEST_FAIL("\tFAIL: %s:%d - Memory regions differ\n" \
		          "\tSIZE: %zu bytes", __FILE__, __LINE__, (size_t)(size)); \
	} \
} while(0)

/* ============================================================= */
/* Test Utilities                                                */
/* ============================================================= */
#define TEST_FAIL(fmt,...) do { __jacl_test_failed = __jacl_test_errored = 1; \
	snprintf(__jacl_test_error, sizeof(__jacl_test_error), fmt, ##__VA_ARGS__); \
	return; \
} while(0)
#define TEST_SKIP(reason) do { \
	__jacl_test_skipped = 1; \
	snprintf(__jacl_test_error, sizeof(__jacl_test_error), "\t%s", reason); \
	return; \
} while(0)
#define TEST_INFO(format, ...) do { \
  printf("	INFO: " format "\n", ##__VA_ARGS__); \
} while(0)

/* ============================================================= */
/* Test Definitions                                              */
/* ============================================================= */
#define TEST_TYPE(name) JACL_INIT(type) { __jacl_test_type = #name; }
#define TEST_UNIT(name) JACL_INIT(unit) { __jacl_test_unit = #name; }
#define TEST_SUITE(name) JACL_INIT(suite_##name) { __jacl_test_suite = #name; }
#define TEST(name) \
	static void test_##name(void); \
	JACL_INIT(test_##name) { __jacl_test_register(#name, test_##name); } \
	static void test_##name(void)

/* ============================================================= */
/* Test Running                                                  */
/* ============================================================= */
#define  TEST_REPORT() __jacl_test_report()
#define  TEST_EACH(suite) __jacl_test_each(suite)
#define  TEST_ONLY(name) __jacl_test_only(name)
#define  TEST_ALL() __jacl_test_all()
#define  TEST_RUN() JACL_INIT(run) { __jacl_test_all(); }

#ifndef NO_TEST_MAIN
#define TEST_MAIN() int main(int argc, char**argv) { \
	fflush(stdout); fflush(stderr); (void)stdin; \
	int r = 0; \
	if (argc > 1) { \
		if (strncmp(argv[1], "--suite=", 8) == 0) { r = TEST_EACH(argv[1] + 8); } \
		else if (strncmp(argv[1], "--test=", 7) == 0) { r = TEST_ONLY(argv[1] + 7); } \
		else if (!strcmp(argv[1], "--failed")) { __jacl_show_only_failed = 1; r = TEST_ALL(); } \
		else if (!strcmp(argv[1], "--skipped")) { __jacl_show_only_skipped = 1; r = TEST_ALL(); } \
		else { printf("Usage: %s [--suite=NAME | --test=NAME | --failed | --skipped]\n", argv[0]); return 1; } \
	} else { r = TEST_ALL(); } \
	TEST_REPORT(); \
	fflush(stdout); \
	return r; \
}
#else
#define TEST_MAIN() TEST_RUN()
#endif

#endif /* TESTING_H */
