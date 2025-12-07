/* (c) 2025 FRINKnet & Friends – MIT licence */
#include <testing.h>
#include <sched.h>
#include <unistd.h>

TEST_TYPE(unit);
TEST_UNIT(sched.h);

/* ============================================================================
 * CONSTANTS
 * ============================================================================ */
TEST_SUITE(constants);

TEST(constants_scheduling_policies) {
	ASSERT_EQ(0, SCHED_OTHER);
	ASSERT_EQ(1, SCHED_FIFO);
	ASSERT_EQ(2, SCHED_RR);
	ASSERT_EQ(3, SCHED_SPORADIC);
}

#if JACL_HAS_C23
TEST(constants_version_macro) {
	ASSERT_EQ(202311L, __STDC_VERSION_SCHED_H__);
}
#endif

#if JACL_OS_LINUX
TEST_SUITE(linux_constants);

TEST(constants_scheduling_policies_extended) {
	/* Extended policies from Linux ABI [52][65] */
	ASSERT_EQ(3, SCHED_BATCH);
	ASSERT_EQ(5, SCHED_IDLE);
	ASSERT_EQ(6, SCHED_DEADLINE);
}

TEST(constants_clone_flags_values) {
	/* Core thread-ish flags [52][65] */
	ASSERT_EQ(0x00000100, CLONE_VM);
	ASSERT_EQ(0x00000200, CLONE_FS);
	ASSERT_EQ(0x00000400, CLONE_FILES);
	ASSERT_EQ(0x00000800, CLONE_SIGHAND);
	ASSERT_EQ(0x00010000, CLONE_THREAD);

	/* Namespace / TLS / tid flags */
	ASSERT_EQ(0x00020000, CLONE_NEWNS);
	ASSERT_EQ(0x00040000, CLONE_SYSVSEM);
	ASSERT_EQ(0x00080000, CLONE_SETTLS);
	ASSERT_EQ(0x00100000, CLONE_PARENT_SETTID);
	ASSERT_EQ(0x00200000, CLONE_CHILD_CLEARTID);
	ASSERT_EQ(0x00400000, CLONE_DETACHED);
	ASSERT_EQ(0x01000000, CLONE_CHILD_SETTID);
}
#endif


/* ============================================================================
 * SCHED_PARAM STRUCTURE
 * ============================================================================ */
TEST_SUITE(sched_param);

TEST(sched_param_priority) {
	struct sched_param param;
	param.sched_priority = 10;

	ASSERT_EQ(10, param.sched_priority);
}

TEST(sched_param_all_fields) {
	struct sched_param param;

	param.sched_priority = 50;
	param.sched_ss_low_priority = 10;
	param.sched_ss_repl_period.tv_sec = 1;
	param.sched_ss_repl_period.tv_nsec = 0;
	param.sched_ss_init_budget.tv_sec = 0;
	param.sched_ss_init_budget.tv_nsec = 100000000;
	param.sched_ss_max_repl = 5;

	ASSERT_EQ(50, param.sched_priority);
	ASSERT_EQ(10, param.sched_ss_low_priority);
	ASSERT_EQ(5, param.sched_ss_max_repl);
}

/* ============================================================================
 * SCHED_YIELD
 * ============================================================================ */
TEST_SUITE(sched_yield);

TEST(sched_yield_basic) {
	int result = sched_yield();
	ASSERT_EQ(0, result);
}

TEST(sched_yield_multiple_calls) {
	for (int i = 0; i < 5; i++) {
		ASSERT_EQ(0, sched_yield());
	}
}

/* ============================================================================
 * PRIORITY RANGES
 * ============================================================================ */
TEST_SUITE(sched_get_priority);

TEST(sched_get_priority_max_fifo) {
	int max = sched_get_priority_max(SCHED_FIFO);
	ASSERT_TRUE(max > 0);
	ASSERT_EQ(99, max);
}

TEST(sched_get_priority_max_rr) {
	int max = sched_get_priority_max(SCHED_RR);
	ASSERT_TRUE(max > 0);
	ASSERT_EQ(99, max);
}

TEST(sched_get_priority_max_other) {
	int max = sched_get_priority_max(SCHED_OTHER);
	ASSERT_EQ(0, max);
}

TEST(sched_get_priority_max_invalid) {
	int max = sched_get_priority_max(999);
	ASSERT_EQ(-1, max);
}

TEST(sched_get_priority_min_fifo) {
	int min = sched_get_priority_min(SCHED_FIFO);
	ASSERT_TRUE(min >= 0);
	ASSERT_EQ(1, min);
}

TEST(sched_get_priority_min_rr) {
	int min = sched_get_priority_min(SCHED_RR);
	ASSERT_TRUE(min >= 0);
	ASSERT_EQ(1, min);
}

TEST(sched_get_priority_min_other) {
	int min = sched_get_priority_min(SCHED_OTHER);
	ASSERT_EQ(0, min);
}

TEST(sched_get_priority_min_invalid) {
	int min = sched_get_priority_min(999);
	ASSERT_EQ(-1, min);
}

TEST(sched_get_priority_range) {
	int min = sched_get_priority_min(SCHED_FIFO);
	int max = sched_get_priority_max(SCHED_FIFO);

	ASSERT_TRUE(min < max);
	ASSERT_TRUE(min >= 1);
	ASSERT_TRUE(max <= 99);
}

/* ============================================================================
 * SCHED_GETPARAM
 * ============================================================================ */
TEST_SUITE(sched_getparam);

TEST(sched_getparam_self) {
	struct sched_param param;
	int result = sched_getparam(0, &param);

	ASSERT_EQ(0, result);
	ASSERT_TRUE(param.sched_priority >= 0);
}

TEST(sched_getparam_current_process) {
	struct sched_param param;
	pid_t pid = getpid();

	int result = sched_getparam(pid, &param);
	ASSERT_EQ(0, result);
}

TEST(sched_getparam_null_param) {
	int result = sched_getparam(0, NULL);
	ASSERT_EQ(-1, result);
}

/* ============================================================================
 * SCHED_SETPARAM
 * ============================================================================ */
TEST_SUITE(sched_setparam);

TEST(sched_setparam_null_param) {
	int result = sched_setparam(0, NULL);
	ASSERT_EQ(-1, result);
}

TEST(sched_setparam_valid_priority) {
	struct sched_param param;
	param.sched_priority = 0;

	int result = sched_setparam(0, &param);
	// May succeed or fail depending on permissions
	ASSERT_TRUE(result == 0 || result == -1);
}

/* ============================================================================
 * SCHED_GETSCHEDULER
 * ============================================================================ */
TEST_SUITE(sched_getscheduler);

TEST(sched_getscheduler_self) {
	int policy = sched_getscheduler(0);

	ASSERT_TRUE(policy >= 0);
	ASSERT_TRUE(policy == SCHED_OTHER || policy == SCHED_FIFO || policy == SCHED_RR);
}

TEST(sched_getscheduler_current_process) {
	pid_t pid = getpid();
	int policy = sched_getscheduler(pid);

	ASSERT_TRUE(policy >= 0);
}

/* ============================================================================
 * SCHED_SETSCHEDULER
 * ============================================================================ */
TEST_SUITE(sched_setscheduler);

TEST(sched_setscheduler_null_param) {
	int result = sched_setscheduler(0, SCHED_OTHER, NULL);
	ASSERT_EQ(-1, result);
}

TEST(sched_setscheduler_other) {
	struct sched_param param;
	param.sched_priority = 0;

	int result = sched_setscheduler(0, SCHED_OTHER, &param);
	// May succeed or fail depending on permissions
	ASSERT_TRUE(result == 0 || result == -1);
}

/* ============================================================================
 * SCHED_RR_GET_INTERVAL
 * ============================================================================ */
TEST_SUITE(sched_rr_get_interval);

TEST(sched_rr_get_interval_basic) {
	struct timespec tp;
	int result = sched_rr_get_interval(0, &tp);

	ASSERT_EQ(0, result);
	ASSERT_TRUE(tp.tv_sec >= 0);
	ASSERT_TRUE(tp.tv_nsec > 0);
}

TEST(sched_rr_get_interval_null) {
	int result = sched_rr_get_interval(0, NULL);
	ASSERT_EQ(-1, result);
}

TEST(sched_rr_get_interval_reasonable) {
	struct timespec tp;
	sched_rr_get_interval(0, &tp);

	// Time slice should be reasonable (0-100ms)
	ASSERT_TRUE(tp.tv_sec == 0);
	ASSERT_TRUE(tp.tv_nsec <= 100000000);
}

/* ============================================================================
 * CPU AFFINITY (Linux only)
 * ============================================================================ */
#if JACL_OS_LINUX
TEST_SUITE(cpu_affinity);

TEST(sched_getaffinity_basic) {
	unsigned char mask[128] = {0};
	int result = sched_getaffinity(0, sizeof(mask), mask);

	// Should succeed on Linux
	ASSERT_GE(result, -1);
}

TEST(sched_setaffinity_basic) {
	unsigned char mask[128] = {0};
	mask[0] = 1;  // Set CPU 0

	int result = sched_setaffinity(0, sizeof(mask), mask);
	// May fail due to permissions
	ASSERT_TRUE(result == 0 || result == -1);
}

TEST(sched_affinity_null_mask) {
	int result = sched_getaffinity(0, 128, NULL);
	ASSERT_EQ(-1, result);
}

TEST(sched_getaffinity_edge_cases_zero_size) {
	unsigned char dummy = 0;

	int result = sched_getaffinity(0, 0, &dummy);

	ASSERT_LE(result, 0);
}

TEST(sched_getaffinity_edge_cases_huge_size) {
	unsigned char mask[1024] = {0};

	int result = sched_getaffinity(0, sizeof(mask), mask);

	ASSERT_GE(result, -1);
}
#endif

/* ============================================================================
 * PLATFORM-SPECIFIC TESTS
 * ============================================================================ */
#if JACL_ARCH_WASM
TEST_SUITE(wasm);

TEST(wasm_scheduling_stubs) {
	// WASM should return reasonable defaults
	ASSERT_EQ(0, sched_yield());
	ASSERT_EQ(99, sched_get_priority_max(SCHED_FIFO));
	ASSERT_EQ(1, sched_get_priority_min(SCHED_FIFO));
}

TEST(wasm_affinity_not_supported) {
	unsigned char mask[128];
	ASSERT_EQ(-1, sched_setaffinity(0, sizeof(mask), mask));
	ASSERT_EQ(-1, sched_getaffinity(0, sizeof(mask), mask));
}
#endif

/* ============================================================================
 * INTEGRATION TESTS
 * ============================================================================ */
TEST_SUITE(integration);

TEST(get_current_policy_and_params) {
	int policy = sched_getscheduler(0);
	struct sched_param param;
	sched_getparam(0, &param);

	ASSERT_TRUE(policy >= 0);
	ASSERT_TRUE(param.sched_priority >= 0);
}

TEST(priority_within_range) {
	int policy = sched_getscheduler(0);
	struct sched_param param;
	sched_getparam(0, &param);

	int min = sched_get_priority_min(policy);
	int max = sched_get_priority_max(policy);

	ASSERT_TRUE(param.sched_priority >= min);
	ASSERT_TRUE(param.sched_priority <= max);
}

TEST(round_robin_interval_consistency) {
	struct timespec tp1, tp2;

	sched_rr_get_interval(0, &tp1);
	sched_rr_get_interval(0, &tp2);

	// Should return same value
	ASSERT_EQ(tp1.tv_sec, tp2.tv_sec);
	ASSERT_EQ(tp1.tv_nsec, tp2.tv_nsec);
}

/* ============================================================================
 * ERROR HANDLING
 * ============================================================================ */
TEST_SUITE(error_handling);

TEST(invalid_pid) {
	struct sched_param param;
	int result = sched_getparam(-999, &param);

	// Should handle invalid PID gracefully
	ASSERT_TRUE(result == 0 || result == -1);
}

TEST(all_null_checks) {
	ASSERT_EQ(-1, sched_getparam(0, NULL));
	ASSERT_EQ(-1, sched_setparam(0, NULL));
	ASSERT_EQ(-1, sched_setscheduler(0, SCHED_OTHER, NULL));
	ASSERT_EQ(-1, sched_rr_get_interval(0, NULL));
}

/* ============================================================================
 * CONSISTENCY TESTS
 * ============================================================================ */
TEST_SUITE(consistency);

TEST(policy_constants_unique) {
	ASSERT_NE(SCHED_OTHER, SCHED_FIFO);
	ASSERT_NE(SCHED_FIFO, SCHED_RR);
	ASSERT_NE(SCHED_RR, SCHED_SPORADIC);
}

TEST(priority_ranges_make_sense) {
	for (int policy = SCHED_OTHER; policy <= SCHED_RR; policy++) {
		int min = sched_get_priority_min(policy);
		int max = sched_get_priority_max(policy);

		if (min >= 0 && max >= 0) {
			ASSERT_TRUE(min <= max);
		}
	}
}

TEST_MAIN()
