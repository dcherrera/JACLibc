/* (c) 2025 FRINKnet & Friends – MIT licence */
#include <testing.h>

#if JACL_HAS_POSIX
#include <sys/syscall.h>
#include <unistd.h>
#include <fcntl.h>

TEST_TYPE(unit);
TEST_UNIT(sys/syscall.h);

/* ============================================================================
 * SYSCALL WRAPPER - BASIC FUNCTIONALITY
 * ============================================================================ */
TEST_SUITE(syscall_wrapper);

TEST(syscall_exists) {
	// Verify syscall function is available
	// Try a simple syscall like getpid
	long result = syscall(SYS_getpid);

	ASSERT_TRUE(result > 0);
}

TEST(syscall_with_args) {
	// Test syscall with arguments (write to stdout)
	const char *msg = "\n";
	long result = syscall(SYS_write, STDOUT_FILENO, msg, 1);

	ASSERT_TRUE(result >= 0);
}

TEST(syscall_return_value) {
	// Test that syscall returns correct values
	pid_t pid1 = (pid_t)syscall(SYS_getpid);
	pid_t pid2 = getpid();

	ASSERT_EQ(pid1, pid2);
}

/* ============================================================================
 * SYSCALL NUMBERS - BASIC OPERATIONS
 * ============================================================================ */
TEST_SUITE(syscall_numbers);

#define X(SYS, num, name, ...) TEST(sys_##name) { ASSERT_TRUE(SYS > -1); }
#include JACL_X_SYSCALL

/* ============================================================================
 * SYSCALL - FILE OPERATIONS
 * ============================================================================ */
TEST_SUITE(syscall_file_ops);

TEST(syscall_open_close) {
	long fd = syscall(SYS_open, "/tmp/syscall_test.txt", O_CREAT | O_RDWR, 0644);
	ASSERT_TRUE(fd >= 0);

	long result = syscall(SYS_close, fd);
	ASSERT_EQ(0, result);

	unlink("/tmp/syscall_test.txt");
}

TEST(syscall_write_read) {
	long fd = syscall(SYS_open, "/tmp/syscall_test2.txt", O_CREAT | O_RDWR | O_TRUNC, 0644);
	ASSERT_TRUE(fd >= 0);

	const char *data = "test";
	long written = syscall(SYS_write, fd, data, 4);
	ASSERT_EQ(4, written);

	syscall(SYS_lseek, fd, 0, SEEK_SET);

	char buf[10] = {0};
	long read_bytes = syscall(SYS_read, fd, buf, 4);
	ASSERT_EQ(4, read_bytes);
	ASSERT_STR_EQ("test", buf);

	syscall(SYS_close, fd);
	unlink("/tmp/syscall_test2.txt");
}

/* ============================================================================
 * SYSCALL - PROCESS OPERATIONS
 * ============================================================================ */
TEST_SUITE(syscall_process_ops);

TEST(syscall_getpid) {
	long pid = syscall(SYS_getpid);
	ASSERT_TRUE(pid > 0);
}

TEST(syscall_getppid) {
	long ppid = syscall(SYS_getppid);
	ASSERT_TRUE(ppid > 0);
}

TEST(syscall_getuid) {
	long uid = syscall(SYS_getuid);
	ASSERT_TRUE(uid >= 0);
}

TEST(syscall_getgid) {
	long gid = syscall(SYS_getgid);
	ASSERT_TRUE(gid >= 0);
}

/* ============================================================================
 * SYSCALL - ERROR HANDLING
 * ============================================================================ */
TEST_SUITE(syscall_errors);

TEST(syscall_invalid_fd) {
	long result = syscall(SYS_close, -1);
	ASSERT_EQ(-1, result);
}

TEST(syscall_invalid_open) {
	long result = syscall(SYS_open, "/nonexistent/path/file.txt", O_RDONLY);
	ASSERT_EQ(-1, result);
}

/* ============================================================================
 * SYSCALL - VARIADIC ARGUMENTS
 * ============================================================================ */
TEST_SUITE(syscall_variadic);

TEST(syscall_zero_args) {
	long result = syscall(SYS_getpid);
	ASSERT_TRUE(result > 0);
}

TEST(syscall_one_arg) {
	long fd = syscall(SYS_close, -1);
	ASSERT_EQ(-1, fd);
}

TEST(syscall_three_args) {
	long fd = syscall(SYS_open, "/tmp", O_RDONLY, 0);
	ASSERT_TRUE(fd >= 0 || fd == -1);

	if (fd >= 0) {
		syscall(SYS_close, fd);
	}
}

TEST(syscall_six_args) {
	// Most syscalls use <= 6 arguments
	// This tests that the syscall wrapper handles all 6
	long result = syscall(SYS_getpid, 0, 0, 0, 0, 0, 0);
	ASSERT_TRUE(result > 0);
}

/* ============================================================================
 * SYSCALL CONSISTENCY
 * ============================================================================ */
TEST_SUITE(syscall_consistency);

TEST(syscall_vs_wrapper_getpid) {
	long sc_pid = syscall(SYS_getpid);
	pid_t wp_pid = getpid();

	ASSERT_EQ(sc_pid, wp_pid);
}

TEST(syscall_vs_wrapper_write) {
	const char *msg = "\n";

	// Using syscall
	long sc_result = syscall(SYS_write, STDOUT_FILENO, msg, 1);

	// Using wrapper
	ssize_t wp_result = write(STDOUT_FILENO, msg, 1);

	ASSERT_TRUE(sc_result >= 0);
	ASSERT_TRUE(wp_result >= 0);
}

TEST(syscall_multiple_calls) {
	for (int i = 0; i < 10; i++) {
		long pid = syscall(SYS_getpid);
		ASSERT_TRUE(pid > 0);
	}
}

TEST_MAIN()

#else

int main(void) {
	printf("sys/syscall.h requires POSIX\n");
	return 0;
}

#endif
