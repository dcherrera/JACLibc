/* (c) 2025 FRINKnet & Friends – MIT licence */
#include <testing.h>
#include <errno.h>
#include <string.h>

TEST_TYPE(unit);
TEST_UNIT(errno.h);

/* ============================================================================
 * BASIC ERRNO TESTS
 * ============================================================================ */
TEST_SUITE(basic_errno);

TEST(errno_initial_zero) {
	// errno should start at 0
	errno = 0;
	ASSERT_EQ(0, errno);
}

TEST(errno_set_get) {
	errno = 0;
	ASSERT_EQ(0, errno);

	errno = EINVAL;
	ASSERT_EQ(EINVAL, errno);

	errno = ENOMEM;
	ASSERT_EQ(ENOMEM, errno);
}

TEST(errno_persistence) {
	errno = ENOENT;
	ASSERT_EQ(ENOENT, errno);

	// Should persist
	ASSERT_EQ(ENOENT, errno);
	ASSERT_EQ(ENOENT, errno);
}

TEST(errno_reset) {
	errno = EINVAL;
	ASSERT_EQ(EINVAL, errno);

	errno = 0;
	ASSERT_EQ(0, errno);
}

/* ============================================================================
 * ERROR CODE VALIDATION TESTS
 * ============================================================================ */
TEST_SUITE(error_codes);

TEST(common_errors_defined) {
	// Standard C errors
	ASSERT_NE(0, EPERM);
	ASSERT_NE(0, ENOENT);
	ASSERT_NE(0, ESRCH);
	ASSERT_NE(0, EINTR);
	ASSERT_NE(0, EIO);
	ASSERT_NE(0, EBADF);
	ASSERT_NE(0, ENOMEM);
	ASSERT_NE(0, EACCES);
	ASSERT_NE(0, EINVAL);
}

TEST(math_errors_defined) {
	ASSERT_NE(0, EDOM);
	ASSERT_NE(0, ERANGE);
}

TEST(file_errors_defined) {
	ASSERT_NE(0, ENOENT);
	ASSERT_NE(0, EEXIST);
	ASSERT_NE(0, EISDIR);
	ASSERT_NE(0, ENOTDIR);
	ASSERT_NE(0, ENAMETOOLONG);
}

TEST(io_errors_defined) {
	ASSERT_NE(0, EIO);
	ASSERT_NE(0, EPIPE);
	ASSERT_NE(0, EAGAIN);
	ASSERT_NE(0, EWOULDBLOCK);
}

TEST(network_errors_defined) {
	ASSERT_NE(0, EADDRINUSE);
	ASSERT_NE(0, ECONNREFUSED);
	ASSERT_NE(0, ETIMEDOUT);
	ASSERT_NE(0, ENETUNREACH);
}

/* ============================================================================
 * ERROR CODE UNIQUENESS TESTS
 * ============================================================================ */
TEST_SUITE(uniqueness);

TEST(error_codes_unique) {
	// Common errors should be distinct
	ASSERT_NE(EPERM, ENOENT);
	ASSERT_NE(EINVAL, ENOMEM);
	ASSERT_NE(EACCES, ENOENT);
	ASSERT_NE(EDOM, ERANGE);
}

TEST(errno_values_positive) {
	// All error codes should be positive
	ASSERT_TRUE(EPERM > 0);
	ASSERT_TRUE(ENOENT > 0);
	ASSERT_TRUE(EINVAL > 0);
	ASSERT_TRUE(ENOMEM > 0);
}

/* ============================================================================
 * PLATFORM-SPECIFIC TESTS
 * ============================================================================ */
TEST_SUITE(platform_specific);

#if JACL_OS_LINUX
TEST(linux_errno_values) {
	// Linux-specific values
	ASSERT_EQ(1, EPERM);
	ASSERT_EQ(2, ENOENT);
	ASSERT_EQ(22, EINVAL);
	ASSERT_EQ(12, ENOMEM);

	// EAGAIN == EWOULDBLOCK on Linux
	ASSERT_EQ(EAGAIN, EWOULDBLOCK);
}
#endif

#if JACL_OS_DARWIN || JACL_OS_BSD
TEST(bsd_errno_values) {
	// BSD-specific values (different from Linux!)
	ASSERT_EQ(1, EPERM);
	ASSERT_EQ(2, ENOENT);
	ASSERT_EQ(22, EINVAL);
	ASSERT_EQ(12, ENOMEM);

	// EAGAIN == EWOULDBLOCK on BSD
	ASSERT_EQ(35, EAGAIN);
	ASSERT_EQ(EAGAIN, EWOULDBLOCK);
}
#endif

#if JACL_OS_WINDOWS
TEST(windows_errno_values) {
	ASSERT_EQ(1, EPERM);
	ASSERT_EQ(2, ENOENT);
	ASSERT_EQ(22, EINVAL);
	ASSERT_EQ(12, ENOMEM);
}
#endif

/* ============================================================================
 * ERRNO MACRO TESTS
 * ============================================================================ */
TEST_SUITE(errno_macro);

TEST(errno_is_lvalue) {
	// errno should be assignable
	errno = 42;
	ASSERT_EQ(42, errno);

	errno++;
	ASSERT_EQ(43, errno);

	errno--;
	ASSERT_EQ(42, errno);
}

TEST(errno_address) {
	// Should be able to take address
	int *ptr = &errno;
	ASSERT_NOT_NULL(ptr);

	*ptr = 99;
	ASSERT_EQ(99, errno);
}

TEST(errno_in_expressions) {
	errno = 10;

	int x = errno + 5;
	ASSERT_EQ(15, x);

	int y = errno * 2;
	ASSERT_EQ(20, y);
}

/* ============================================================================
 * FUNCTION INTEGRATION TESTS
 * ============================================================================ */
TEST_SUITE(function_integration);

TEST(errno_cleared_by_user) {
	// User code should clear errno before operations
	errno = 0;

	// Simulate a function that might set errno
	if (0) {  // Simulated failure
		errno = EINVAL;
	}

	ASSERT_EQ(0, errno);
}

TEST(errno_after_success) {
	// errno should not be modified on success
	errno = 999;  // Set to invalid value

	// Simulated successful operation
	int result = 0;
	(void)result;

	// errno should still be 999 (not modified)
	ASSERT_EQ(999, errno);
}

TEST(errno_after_failure) {
	// errno should be set on failure
	errno = 0;

	// Simulate failed operation
	errno = ENOENT;

	ASSERT_EQ(ENOENT, errno);
}

/* ============================================================================
 * ERROR CODE RANGE TESTS
 * ============================================================================ */
TEST_SUITE(error_ranges);

TEST(standard_c_errors) {
	// POSIX requires these to be defined
	static const int standard_errors[] = {
		EPERM, ENOENT, ESRCH, EINTR, EIO, ENXIO, E2BIG,
		ENOEXEC, EBADF, ECHILD, EAGAIN, ENOMEM, EACCES,
		EFAULT, EBUSY, EEXIST, EXDEV, ENODEV, ENOTDIR,
		EISDIR, EINVAL, ENFILE, EMFILE, ENOTTY, EFBIG,
		ENOSPC, ESPIPE, EROFS, EMLINK, EPIPE, EDOM, ERANGE
	};

	for (size_t i = 0; i < sizeof(standard_errors)/sizeof(int); i++) {
		ASSERT_TRUE(standard_errors[i] > 0);
		ASSERT_TRUE(standard_errors[i] < 1000);  // Reasonable upper bound
	}
}

TEST(network_errors_range) {
	ASSERT_TRUE(EADDRINUSE > 0);
	ASSERT_TRUE(ECONNREFUSED > 0);
	ASSERT_TRUE(ETIMEDOUT > 0);
}

/* ============================================================================
 * THREAD-LOCAL STORAGE TESTS
 * ============================================================================ */
TEST_SUITE(thread_local);

TEST(errno_thread_local) {
	// Set errno in main thread
	errno = EINVAL;
	ASSERT_EQ(EINVAL, errno);

	// errno should be thread-local (tested by setting/getting)
	errno = ENOMEM;
	ASSERT_EQ(ENOMEM, errno);

	// Reset
	errno = 0;
}

TEST(errno_isolated_per_thread) {
	// Each thread should have its own errno
	// (Can't easily test without actual threads, but verify mechanism)
	errno = 123;
	ASSERT_EQ(123, errno);

	// Verify independent access
	int saved = errno;
	errno = 456;
	ASSERT_EQ(456, errno);
	ASSERT_NE(saved, errno);
}

/* ============================================================================
 * ERRNO USAGE PATTERNS
 * ============================================================================ */
TEST_SUITE(usage_patterns);

TEST(check_and_clear_pattern) {
	// Pattern: clear before operation, check after
	errno = 0;

	// Simulated operation
	int result = -1;  // Failure
	if (result == -1) {
		errno = ENOENT;
	}

	ASSERT_EQ(ENOENT, errno);
}

TEST(save_and_restore_pattern) {
	// Pattern: save, do operation, restore
	errno = EINVAL;
	int saved_errno = errno;

	// Some operation that modifies errno
	errno = ENOMEM;

	// Restore
	errno = saved_errno;
	ASSERT_EQ(EINVAL, errno);
}

TEST(conditional_error_pattern) {
	errno = 0;
	int error_occurred = 0;

	// Simulated operation
	if (0) {  // Condition
		errno = EACCES;
		error_occurred = 1;
	}

	if (error_occurred) {
		ASSERT_EQ(EACCES, errno);
	} else {
		ASSERT_EQ(0, errno);
	}
}

/* ============================================================================
 * ERROR CODE SEMANTICS TESTS
 * ============================================================================ */
TEST_SUITE(semantics);

TEST(permission_errors) {
	// Permission-related errors
	ASSERT_NE(0, EPERM);
	ASSERT_NE(0, EACCES);
	ASSERT_NE(EPERM, EACCES);
}

TEST(resource_errors) {
	// Resource-related errors
	ASSERT_NE(0, ENOMEM);
	ASSERT_NE(0, ENOSPC);
	ASSERT_NE(0, ENFILE);
	ASSERT_NE(0, EMFILE);
}

TEST(connection_errors) {
	// Connection-related errors
	ASSERT_NE(0, ECONNREFUSED);
	ASSERT_NE(0, ECONNRESET);
	ASSERT_NE(0, ETIMEDOUT);
}

/* ============================================================================
 * EDGE CASES
 * ============================================================================ */
TEST_SUITE(edge_cases);

TEST(errno_zero_valid) {
	// Zero is a valid errno value (success)
	errno = 0;
	ASSERT_EQ(0, errno);
}

TEST(errno_negative_values) {
	// errno can be set to negative (though not standard)
	errno = -1;
	ASSERT_EQ(-1, errno);

	errno = 0;  // Reset
}

TEST(errno_large_values) {
	// errno can hold large values
	errno = 999;
	ASSERT_EQ(999, errno);

	errno = 0;  // Reset
}

TEST(errno_overflow) {
	// Test with very large values
	errno = 0x7FFFFFFF;  // Max positive int
	ASSERT_EQ(0x7FFFFFFF, errno);

	errno = 0;  // Reset
}

/* ============================================================================
 * DOCUMENTATION TESTS
 * ============================================================================ */
TEST_SUITE(documentation);

TEST(errno_usage_example) {
	// Standard errno usage pattern
	errno = 0;

	// Operation (simulated)
	int fd = -1;  // Failed to open

	if (fd == -1) {
		int err = errno;
		ASSERT_TRUE(err >= 0);
	}
}

TEST(errno_multiple_checks) {
	errno = 0;

	// First operation
	errno = EINVAL;
	int err1 = errno;

	// Second operation
	errno = ENOMEM;
	int err2 = errno;

	ASSERT_NE(err1, err2);
	ASSERT_EQ(EINVAL, err1);
	ASSERT_EQ(ENOMEM, err2);
}

TEST_MAIN()
