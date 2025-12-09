/* (c) 2025 FRINKnet & Friends – MIT licence */
#include <testing.h>
#include <poll.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

TEST_TYPE(unit);
TEST_UNIT(poll.h);

/* ============================================================================
 * CONSTANTS
 * ============================================================================ */
TEST_SUITE(constants);

TEST(constants_pollin) {
	ASSERT_EQ(0x0001, POLLIN);
}

TEST(constants_pollpri) {
	ASSERT_EQ(0x0002, POLLPRI);
}

TEST(constants_pollout) {
	ASSERT_EQ(0x0004, POLLOUT);
}

TEST(constants_pollerr) {
	ASSERT_EQ(0x0008, POLLERR);
}

TEST(constants_pollhup) {
	ASSERT_EQ(0x0010, POLLHUP);
}

TEST(constants_pollnval) {
	ASSERT_EQ(0x0020, POLLNVAL);
}

TEST(constants_inftim) {
	ASSERT_EQ(-1, INFTIM);
}

/* ============================================================================
 * POLLFD_INIT
 * ============================================================================ */
TEST_SUITE(pollfd_init);

TEST(pollfd_init_null) {
	pollfd_init(NULL, 0, 0);
	// Should not crash
}

TEST(pollfd_init_valid) {
	struct pollfd pfd;
	pollfd_init(&pfd, 10, POLLIN | POLLOUT);

	ASSERT_EQ(10, pfd.fd);
	ASSERT_EQ(POLLIN | POLLOUT, pfd.events);
	ASSERT_EQ(0, pfd.revents);
}

TEST(pollfd_init_zero_events) {
	struct pollfd pfd;
	pollfd_init(&pfd, 5, 0);

	ASSERT_EQ(5, pfd.fd);
	ASSERT_EQ(0, pfd.events);
	ASSERT_EQ(0, pfd.revents);
}

TEST(pollfd_init_negative_fd) {
	struct pollfd pfd;
	pollfd_init(&pfd, -1, POLLIN);

	ASSERT_EQ(-1, pfd.fd);
	ASSERT_EQ(POLLIN, pfd.events);
	ASSERT_EQ(0, pfd.revents);
}

/* ============================================================================
 * POLLFD_HAS_INPUT
 * ============================================================================ */
TEST_SUITE(pollfd_has_input);

TEST(pollfd_has_input_null) {
	ASSERT_FALSE(pollfd_has_input(NULL));
}

TEST(pollfd_has_input_true) {
	struct pollfd pfd = {0, 0, POLLIN};
	ASSERT_TRUE(pollfd_has_input(&pfd));
}

TEST(pollfd_has_input_false) {
	struct pollfd pfd = {0, 0, POLLOUT};
	ASSERT_FALSE(pollfd_has_input(&pfd));
}

TEST(pollfd_has_input_mixed) {
	struct pollfd pfd = {0, 0, POLLIN | POLLOUT};
	ASSERT_TRUE(pollfd_has_input(&pfd));
}

/* ============================================================================
 * POLLFD_HAS_OUTPUT
 * ============================================================================ */
TEST_SUITE(pollfd_has_output);

TEST(pollfd_has_output_null) {
	ASSERT_FALSE(pollfd_has_output(NULL));
}

TEST(pollfd_has_output_true) {
	struct pollfd pfd = {0, 0, POLLOUT};
	ASSERT_TRUE(pollfd_has_output(&pfd));
}

TEST(pollfd_has_output_false) {
	struct pollfd pfd = {0, 0, POLLIN};
	ASSERT_FALSE(pollfd_has_output(&pfd));
}

TEST(pollfd_has_output_mixed) {
	struct pollfd pfd = {0, 0, POLLIN | POLLOUT};
	ASSERT_TRUE(pollfd_has_output(&pfd));
}

/* ============================================================================
 * POLLFD_HAS_ERROR
 * ============================================================================ */
TEST_SUITE(pollfd_has_error);

TEST(pollfd_has_error_null) {
	ASSERT_FALSE(pollfd_has_error(NULL));
}

TEST(pollfd_has_error_true) {
	struct pollfd pfd = {0, 0, POLLERR};
	ASSERT_TRUE(pollfd_has_error(&pfd));
}

TEST(pollfd_has_error_false) {
	struct pollfd pfd = {0, 0, POLLIN};
	ASSERT_FALSE(pollfd_has_error(&pfd));
}

TEST(pollfd_has_error_mixed) {
	struct pollfd pfd = {0, 0, POLLERR | POLLIN};
	ASSERT_TRUE(pollfd_has_error(&pfd));
}

/* ============================================================================
 * POLLFD_HAS_PRIORITY
 * ============================================================================ */
TEST_SUITE(pollfd_has_priority);

TEST(pollfd_has_priority_null) {
	ASSERT_FALSE(pollfd_has_priority(NULL));
}

TEST(pollfd_has_priority_true) {
	struct pollfd pfd = {0, 0, POLLPRI};
	ASSERT_TRUE(pollfd_has_priority(&pfd));
}

TEST(pollfd_has_priority_false) {
	struct pollfd pfd = {0, 0, POLLIN};
	ASSERT_FALSE(pollfd_has_priority(&pfd));
}

/* ============================================================================
 * POLLFD_HAS_HANGUP
 * ============================================================================ */
TEST_SUITE(pollfd_has_hangup);

TEST(pollfd_has_hangup_null) {
	ASSERT_FALSE(pollfd_has_hangup(NULL));
}

TEST(pollfd_has_hangup_true) {
	struct pollfd pfd = {0, 0, POLLHUP};
	ASSERT_TRUE(pollfd_has_hangup(&pfd));
}

TEST(pollfd_has_hangup_false) {
	struct pollfd pfd = {0, 0, POLLIN};
	ASSERT_FALSE(pollfd_has_hangup(&pfd));
}

/* ============================================================================
 * POLL_VALIDATE
 * ============================================================================ */
TEST_SUITE(poll_validate);

TEST(poll_validate_null_fds) {
	ASSERT_EQ(-1, poll_validate(NULL, 1));
}

TEST(poll_validate_zero_nfds) {
	struct pollfd pfd;
	ASSERT_EQ(0, poll_validate(&pfd, 0));
}

TEST(poll_validate_invalid_fd) {
	struct pollfd pfd = {-100, POLLIN, 0};
	ASSERT_EQ(-1, poll_validate(&pfd, 1));
}

TEST(poll_validate_valid_stdin) {
	struct pollfd pfd = {STDIN_FILENO, POLLIN, 0};
	ASSERT_EQ(0, poll_validate(&pfd, 1));
}

TEST(poll_validate_multiple_valid) {
	struct pollfd pfds[3] = {
		{0, POLLIN, 0},
		{1, POLLOUT, 0},
		{2, POLLIN, 0}
	};
	ASSERT_EQ(0, poll_validate(pfds, 3));
}

TEST(poll_validate_one_invalid) {
	struct pollfd pfds[2] = {
		{0, POLLIN, 0},
		{-100, POLLIN, 0}
	};
	ASSERT_EQ(-1, poll_validate(pfds, 2));
}

/* ============================================================================
 * POLL_EVENTS_TO_STRING_R
 * ============================================================================ */
TEST_SUITE(poll_events_to_string_r);

TEST(poll_events_to_string_r_null_buffer) {
	poll_events_to_string_r(POLLIN, NULL, 64);
	// Should not crash
}

TEST(poll_events_to_string_r_zero_size) {
	char buf[64];
	poll_events_to_string_r(POLLIN, buf, 0);
	// Should not crash
}

TEST(poll_events_to_string_r_pollin) {
	char buf[64];
	poll_events_to_string_r(POLLIN, buf, sizeof(buf));
	ASSERT_STR_EQ("POLLIN", buf);
}

TEST(poll_events_to_string_r_pollout) {
	char buf[64];
	poll_events_to_string_r(POLLOUT, buf, sizeof(buf));
	ASSERT_STR_EQ("POLLOUT", buf);
}

TEST(poll_events_to_string_r_pollerr) {
	char buf[64];
	poll_events_to_string_r(POLLERR, buf, sizeof(buf));
	ASSERT_STR_EQ("POLLERR", buf);
}

TEST(poll_events_to_string_r_pollhup) {
	char buf[64];
	poll_events_to_string_r(POLLHUP, buf, sizeof(buf));
	ASSERT_STR_EQ("POLLHUP", buf);
}

TEST(poll_events_to_string_r_pollpri) {
	char buf[64];
	poll_events_to_string_r(POLLPRI, buf, sizeof(buf));
	ASSERT_STR_EQ("POLLPRI", buf);
}

TEST(poll_events_to_string_r_pollnval) {
	char buf[64];
	poll_events_to_string_r(POLLNVAL, buf, sizeof(buf));
	ASSERT_STR_EQ("POLLNVAL", buf);
}

TEST(poll_events_to_string_r_multiple) {
	char buf[128];
	poll_events_to_string_r(POLLIN | POLLOUT, buf, sizeof(buf));
	ASSERT_STR_HAS("POLLIN", buf);
	ASSERT_STR_HAS("POLLOUT", buf);
}

TEST(poll_events_to_string_r_all) {
	char buf[256];
	poll_events_to_string_r(POLLIN | POLLOUT | POLLERR | POLLHUP | POLLPRI, buf, sizeof(buf));
	ASSERT_STR_HAS("POLLIN", buf);
	ASSERT_STR_HAS("POLLOUT", buf);
	ASSERT_STR_HAS("POLLERR", buf);
}

TEST(poll_events_to_string_r_zero) {
	char buf[64];
	poll_events_to_string_r(0, buf, sizeof(buf));
	ASSERT_STR_EQ("(none)", buf);
}

TEST(poll_events_to_string_r_buffer_truncation) {
	char buf[10];
	poll_events_to_string_r(POLLIN | POLLOUT | POLLERR, buf, sizeof(buf));
	ASSERT_LT(strlen(buf), sizeof(buf));
}

TEST(poll_events_to_string_r_single_char_buffer) {
	char buf[1];
	poll_events_to_string_r(POLLIN, buf, sizeof(buf));
	ASSERT_EQ('\0', buf[0]);
}

TEST(poll_events_to_string_r_unknown_flags) {
	char buf[64];
	poll_events_to_string_r(0x8000, buf, sizeof(buf));
	ASSERT_NOT_NULL(buf);
}

/* ============================================================================
 * POLL_EVENTS_TO_STRING
 * ============================================================================ */
TEST_SUITE(poll_events_to_string);

TEST(poll_events_to_string_pollin) {
	const char *str = poll_events_to_string(POLLIN);
	ASSERT_NOT_NULL(str);
	ASSERT_STR_HAS("POLLIN", str);
}

TEST(poll_events_to_string_multiple) {
	const char *str = poll_events_to_string(POLLIN | POLLOUT);
	ASSERT_NOT_NULL(str);
	ASSERT_STR_HAS("POLLIN", str);
	ASSERT_STR_HAS("POLLOUT", str);
}

TEST(poll_events_to_string_zero) {
	const char *str = poll_events_to_string(0);
	ASSERT_NOT_NULL(str);
}

/* ============================================================================
 * POLL
 * ============================================================================ */
TEST_SUITE(poll);

TEST(poll_null_fds_zero_nfds) {
	int result = poll(NULL, 0, 0);
	ASSERT_EQ(0, result);
}

TEST(poll_null_fds_nonzero_nfds) {
	int result = poll(NULL, 1, 0);
	ASSERT_EQ(-1, result);
}

TEST(poll_zero_timeout) {
	struct pollfd pfd = {-1, POLLIN, 0};
	int result = poll(&pfd, 1, 0);
	ASSERT_GE(result, 0);
}

TEST(poll_invalid_fd_marked) {
	struct pollfd pfd = {-100, POLLIN, 0};
	int result = poll(&pfd, 1, 0);
	if (result == 1) {
		ASSERT_TRUE(pfd.revents & POLLNVAL);
	}
}

#if !POLL_DUMMY

TEST(poll_stdin_immediate) {
	struct pollfd pfd;
	pollfd_init(&pfd, STDIN_FILENO, POLLIN);
	int result = poll(&pfd, 1, 0);
	ASSERT_GE(result, 0);
}

TEST(poll_large_nfds) {
	struct pollfd pfds[100];
	for (int i = 0; i < 100; i++) {
		pollfd_init(&pfds[i], -1, POLLIN);
	}
	int result = poll(pfds, 100, 0);
	ASSERT_GE(result, 0);
}

TEST(poll_mixed_valid_invalid) {
	struct pollfd pfds[3] = {
		{STDIN_FILENO, POLLIN, 0},
		{-100, POLLIN, 0},
		{STDOUT_FILENO, POLLOUT, 0}
	};
	int result = poll(pfds, 3, 0);
	ASSERT_GE(result, 0);
}

#endif

#if !POLL_DUMMY && !JACL_OS_WINDOWS

TEST(poll_pipe_writable) {
	int pipefd[2];
	ASSERT_EQ(0, pipe(pipefd));

	struct pollfd pfd;
	pollfd_init(&pfd, pipefd[1], POLLOUT);

	int result = poll(&pfd, 1, 0);

	ASSERT_GE(result, 0);

	if (result > 0) {
		ASSERT_TRUE(pfd.revents & POLLOUT);
	}

	close(pipefd[0]);
	close(pipefd[1]);
}

TEST(poll_multiple_fds) {
	int pipe1[2], pipe2[2];
	pipe(pipe1);
	pipe(pipe2);
	write(pipe1[1], "data", 4);

	struct pollfd pfds[2];
	pollfd_init(&pfds[0], pipe1[0], POLLIN);
	pollfd_init(&pfds[1], pipe2[0], POLLIN);

	int result = poll(pfds, 2, 0);
	ASSERT_EQ(1, result);
	ASSERT_TRUE(pfds[0].revents & POLLIN);

	close(pipe1[0]); close(pipe1[1]);
	close(pipe2[0]); close(pipe2[1]);
}

TEST(poll_closed_fd) {
	int pipefd[2];
	pipe(pipefd);
	close(pipefd[0]);

	struct pollfd pfd;
	pollfd_init(&pfd, pipefd[0], POLLIN);

	int result = poll(&pfd, 1, 0);
	ASSERT_GE(result, 0);

	close(pipefd[1]);
}

TEST(poll_pipe_hangup) {
	int pipefd[2];
	pipe(pipefd);
	close(pipefd[1]);  // Close write end

	struct pollfd pfd;
	pollfd_init(&pfd, pipefd[0], POLLIN);

	int result = poll(&pfd, 1, 100);

	fprintf(stderr, "POLLIN=0x%x, POLLOUT=0x%x, POLLHUP=0x%x\n",
				        POLLIN, POLLOUT, POLLHUP);
	fprintf(stderr, "DEBUG: result=%d, revents=0x%x (%s)\n",
	        result, pfd.revents, poll_events_to_string(pfd.revents));
	fprintf(stderr, "Read fd=%d, Write fd=%d, Polling fd=%d\n",
			        pipefd[0], pipefd[1], pfd.fd);

	ASSERT_GE(result, 0);

	close(pipefd[0]);
}

TEST(poll_negative_timeout_with_ready_fd) {
	int pipefd[2];
	pipe(pipefd);
	write(pipefd[1], "x", 1);

	struct pollfd pfd;
	pollfd_init(&pfd, pipefd[0], POLLIN);

	int result = poll(&pfd, 1, -1);  // Infinite timeout, but fd is ready
	ASSERT_EQ(1, result);
	ASSERT_TRUE(pfd.revents & POLLIN);

	close(pipefd[0]);
	close(pipefd[1]);
}

TEST(poll_validate_invalid_events) {
	struct pollfd pfd = {STDIN_FILENO, 0x8000, 0};  // Invalid event flag
	ASSERT_EQ(-1, poll_validate(&pfd, 1));
	ASSERT_ERRNO_EQ(EINVAL);
}

TEST(poll_preserves_errno_on_success) {
	struct pollfd pfd = {-1, POLLIN, 0};
	errno = 42;  // Set to something
	int result = poll(&pfd, 1, 0);
	ASSERT_GE(result, 0);
	ASSERT_EQ(42, errno);  // Should be unchanged
}

#endif

#if POLL_DUMMY

TEST(poll_returns_enosys) {
	struct pollfd pfd;
	int result = poll(&pfd, 1, 0);
	ASSERT_EQ(-1, result);
	ASSERT_ERRNO_EQ(ENOSYS);
}

#endif

/* ============================================================================
 * PPOLL
 * ============================================================================ */
TEST_SUITE(ppoll);

#if !POLL_DUMMY && !JACL_OS_WINDOWS
TEST(ppoll_null_fds_zero_nfds) {
	struct timespec timeout = {0, 0};
	int result = ppoll(NULL, 0, &timeout, NULL);
	ASSERT_EQ(0, result);
}

TEST(ppoll_zero_timeout) {
	struct pollfd pfd = {-1, POLLIN, 0};
	struct timespec timeout = {0, 0};
	int result = ppoll(&pfd, 1, &timeout, NULL);
	ASSERT_GE(result, 0);
}

TEST(ppoll_null_timeout) {
	int pipefd[2];
	pipe(pipefd);
	write(pipefd[1], "x", 1);

	struct pollfd pfd;
	pollfd_init(&pfd, pipefd[0], POLLIN);

	int result = ppoll(&pfd, 1, NULL, NULL);
	ASSERT_EQ(1, result);
	ASSERT_TRUE(pfd.revents & POLLIN);

	close(pipefd[0]);
	close(pipefd[1]);
}

TEST(ppoll_nanosecond_timeout) {
	struct pollfd pfd = {-1, POLLIN, 0};
	struct timespec timeout = {0, 100};

	int result = ppoll(&pfd, 1, &timeout, NULL);
	ASSERT_EQ(0, result);
}

TEST(ppoll_large_timeout) {
	int pipefd[2];
	pipe(pipefd);
	write(pipefd[1], "x", 1);

	struct pollfd pfd;
	pollfd_init(&pfd, pipefd[0], POLLIN);
	struct timespec timeout = {1000, 0};

	int result = ppoll(&pfd, 1, &timeout, NULL);
	ASSERT_EQ(1, result);
	ASSERT_TRUE(pfd.revents & POLLIN);

	close(pipefd[0]);
	close(pipefd[1]);
}

TEST(ppoll_with_sigmask_null) {
	struct pollfd pfd = {-1, POLLIN, 0};
	struct timespec timeout = {0, 0};
	int result = ppoll(&pfd, 1, &timeout, NULL);
	ASSERT_GE(result, 0);
}

TEST(ppoll_with_sigmask_empty) {
	struct pollfd pfd = {-1, POLLIN, 0};
	struct timespec timeout = {0, 1000000};  // 1ms
	sigset_t mask;
	sigemptyset(&mask);

	int result = ppoll(&pfd, 1, &timeout, &mask);
	ASSERT_GE(result, 0);
}

TEST(ppoll_with_sigmask_sigusr1) {
	struct pollfd pfd = {-1, POLLIN, 0};
	struct timespec timeout = {0, 1000000};

	sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask, SIGUSR1);

	int result = ppoll(&pfd, 1, &timeout, &mask);
	ASSERT_GE(result, 0);
}

TEST(ppoll_timeout_immediate) {
	int pipefd[2];
	pipe(pipefd);

	struct pollfd pfd;
	pollfd_init(&pfd, pipefd[0], POLLIN);
	struct timespec timeout = {0, 1000000};

	int result = ppoll(&pfd, 1, &timeout, NULL);
	ASSERT_EQ(0, result);

	close(pipefd[0]);
	close(pipefd[1]);
}
#endif

#if POLL_DUMMY
TEST(ppoll_returns_enosys) {
	struct pollfd pfd;
	int result = ppoll(&pfd, 1, NULL, NULL);
	ASSERT_EQ(-1, result);
	ASSERT_ERRNO_EQ(ENOSYS);
}
#endif

TEST_MAIN()
