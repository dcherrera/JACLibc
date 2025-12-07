/* (c) 2025 FRINKnet & Friends – MIT licence */
#include <testing.h>
#include <signal.h>

TEST_TYPE(unit);
TEST_UNIT(signal.h);

/* ============================================================================
 * SIGNAL CONSTANTS - ISO C (Required on all platforms)
 * ============================================================================ */
TEST_SUITE(constants);

TEST(constants_iso_signals_defined) {
	/* ISO C requires these 6 signals */
	ASSERT_TRUE(SIGABRT > 0);
	ASSERT_TRUE(SIGFPE > 0);
	ASSERT_TRUE(SIGILL > 0);
	ASSERT_TRUE(SIGINT > 0);
	ASSERT_TRUE(SIGSEGV > 0);
	ASSERT_TRUE(SIGTERM > 0);
}

TEST(constants_iso_signals_unique) {
	/* All ISO C signals must be distinct */
	ASSERT_NE(SIGABRT, SIGFPE);
	ASSERT_NE(SIGABRT, SIGILL);
	ASSERT_NE(SIGABRT, SIGINT);
	ASSERT_NE(SIGABRT, SIGSEGV);
	ASSERT_NE(SIGABRT, SIGTERM);

	ASSERT_NE(SIGFPE, SIGILL);
	ASSERT_NE(SIGFPE, SIGINT);
	ASSERT_NE(SIGFPE, SIGSEGV);
	ASSERT_NE(SIGFPE, SIGTERM);

	ASSERT_NE(SIGILL, SIGINT);
	ASSERT_NE(SIGILL, SIGSEGV);
	ASSERT_NE(SIGILL, SIGTERM);

	ASSERT_NE(SIGINT, SIGSEGV);
	ASSERT_NE(SIGINT, SIGTERM);

	ASSERT_NE(SIGSEGV, SIGTERM);
}

TEST(constants_handler_values) {
	/* SIG_DFL is NULL (0) by definition */
	ASSERT_EQ(0, (intptr_t)SIG_DFL);

	/* SIG_IGN is non-zero */
	ASSERT_NE(0, (intptr_t)SIG_IGN);

	/* SIG_ERR is -1 */
	ASSERT_EQ(-1, (intptr_t)SIG_ERR);

	/* All three must be distinct */
	ASSERT_NE(SIG_DFL, SIG_IGN);
	ASSERT_NE(SIG_DFL, SIG_ERR);
	ASSERT_NE(SIG_IGN, SIG_ERR);
}

#if JACL_HAS_POSIX
/* Generate tests for all POSIX signals using X macro */
#define TEST_SIGNAL_DEFINED(sig) \
	TEST(constants_posix_##sig##_defined) { \
		ASSERT_TRUE(sig > 0); \
	}

TEST_SIGNAL_DEFINED(SIGHUP)
TEST_SIGNAL_DEFINED(SIGQUIT)
TEST_SIGNAL_DEFINED(SIGKILL)
TEST_SIGNAL_DEFINED(SIGUSR1)
TEST_SIGNAL_DEFINED(SIGUSR2)
TEST_SIGNAL_DEFINED(SIGPIPE)
TEST_SIGNAL_DEFINED(SIGALRM)
TEST_SIGNAL_DEFINED(SIGCHLD)
TEST_SIGNAL_DEFINED(SIGCONT)
TEST_SIGNAL_DEFINED(SIGSTOP)
TEST_SIGNAL_DEFINED(SIGTSTP)
TEST_SIGNAL_DEFINED(SIGTTIN)
TEST_SIGNAL_DEFINED(SIGTTOU)

#undef TEST_SIGNAL_DEFINED

TEST(constants_realtime_range) {
	ASSERT_TRUE(SIGRTMIN > 0);
	ASSERT_TRUE(SIGRTMAX > 0);
	ASSERT_TRUE(SIGRTMAX > SIGRTMIN);
}

TEST(constants_mask_operations) {
	ASSERT_EQ(0, SIG_BLOCK);
	ASSERT_EQ(1, SIG_UNBLOCK);
	ASSERT_EQ(2, SIG_SETMASK);
}
#endif /* JACL_HAS_POSIX */

#if JACL_HAS_C23
TEST(constants_c23_version) {
	ASSERT_EQ(202311L, __STDC_VERSION_SIGNAL_H__);
}
#endif

/* ============================================================================
 * SIG_ATOMIC_T TYPE
 * ============================================================================ */
TEST_SUITE(sig_atomic_t);

TEST(sig_atomic_t_defined) {
	sig_atomic_t val = 42;
	ASSERT_EQ(42, val);
}

TEST(sig_atomic_t_assignment) {
	sig_atomic_t val = 0;
	val = 1;
	ASSERT_EQ(1, val);
	val = -1;
	ASSERT_EQ(-1, val);
}

TEST(sig_atomic_t_volatile) {
	volatile sig_atomic_t flag = 0;
	flag = 1;
	ASSERT_EQ(1, flag);
}

#if JACL_HAS_C11 && !defined(__STDC_NO_THREADS__)
TEST(sig_atomic_t_store) {
	sig_atomic_t val = 0;
	signal_atomic_store(&val, 42);
	ASSERT_EQ(42, val);
}

TEST(sig_atomic_t_load) {
	sig_atomic_t val = 123;
	sig_atomic_t loaded = signal_atomic_load(&val);
	ASSERT_EQ(123, loaded);
}
#endif

/* ============================================================================
 * SIGNAL HANDLER TYPES
 * ============================================================================ */
TEST_SUITE(types);

TEST(types_sighandler_t) {
	__sighandler_t handler = SIG_DFL;
	ASSERT_EQ(SIG_DFL, handler);
}

TEST(types_sig_t) {
	sig_t handler = SIG_IGN;
	ASSERT_EQ(SIG_IGN, handler);
}

#if JACL_HAS_POSIX
TEST(types_sigset_t_size) {
	ASSERT_TRUE(sizeof(sigset_t) >= 16); /* 2 × 64-bit words */
}

TEST(types_sigval_int) {
	union sigval val;
	val.sival_int = 123;
	ASSERT_EQ(123, val.sival_int);
}

TEST(types_sigval_ptr) {
	union sigval val;
	int dummy = 0;
	val.sival_ptr = &dummy;
	ASSERT_EQ(&dummy, val.sival_ptr);
}

TEST(types_siginfo_t_fields) {
	siginfo_t info;

	info.si_signo = SIGINT;
	info.si_code = SI_USER;
	info.si_value.sival_int = 42;
	info.si_pid = 1234;
	info.si_uid = 5678;

	ASSERT_EQ(SIGINT, info.si_signo);
	ASSERT_EQ(SI_USER, info.si_code);
	ASSERT_EQ(42, info.si_value.sival_int);
	ASSERT_EQ(1234, info.si_pid);
	ASSERT_EQ(5678, info.si_uid);
}

TEST(types_sigaction_struct) {
	struct sigaction act;

	act.sa_handler = SIG_DFL;
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);

	ASSERT_EQ(SIG_DFL, act.sa_handler);
	ASSERT_TRUE(__sigisemptyset(&act.sa_mask));
}
#endif /* JACL_HAS_POSIX */

/* ============================================================================
 * RAISE FUNCTION
 * ============================================================================ */
TEST_SUITE(raise);

static volatile sig_atomic_t raise_signal_received = 0;

static void raise_test_handler(int sig) {
	raise_signal_received = sig;
}

TEST(raise_with_handler) {
	raise_signal_received = 0;

	sig_t old = signal(SIGUSR1, raise_test_handler);
	int result = raise(SIGUSR1);
	signal(SIGUSR1, old);  /* Restore immediately */

	ASSERT_EQ(0, result);
	ASSERT_EQ(SIGUSR1, raise_signal_received);
}

TEST(raise_with_ignore) {
	sig_t old = signal(SIGUSR2, SIG_IGN);
	int result = raise(SIGUSR2);
	signal(SIGUSR2, old);  /* Restore */

	/* Should succeed even though signal is ignored */
	ASSERT_EQ(0, result);
}

TEST(raise_multiple_different) {
	raise_signal_received = 0;

	sig_t old1 = signal(SIGUSR1, raise_test_handler);
	raise(SIGUSR1);
	ASSERT_EQ(SIGUSR1, raise_signal_received);
	signal(SIGUSR1, old1);

	sig_t old2 = signal(SIGUSR2, raise_test_handler);
	raise(SIGUSR2);
	ASSERT_EQ(SIGUSR2, raise_signal_received);
	signal(SIGUSR2, old2);
}

/* ============================================================================
 * SIGNAL FUNCTION
 * ============================================================================ */
TEST_SUITE(signal);

static volatile sig_atomic_t signal_received = 0;

static void signal_test_handler(int sig) {
	signal_received = sig;
}

TEST(signal_set_handler) {
	sig_t old = signal(SIGUSR1, signal_test_handler);
	ASSERT_NE(SIG_ERR, old);
	signal(SIGUSR1, old); /* Restore */
}

TEST(signal_set_default) {
	sig_t old = signal(SIGUSR1, SIG_DFL);
	ASSERT_NE(SIG_ERR, old);
	signal(SIGUSR1, old); /* Restore */
}

TEST(signal_set_ignore) {
	sig_t old = signal(SIGUSR1, SIG_IGN);
	ASSERT_NE(SIG_ERR, old);
	signal(SIGUSR1, old); /* Restore */
}

TEST(signal_restore_handler) {
	sig_t old = signal(SIGUSR1, signal_test_handler);
	sig_t restored = signal(SIGUSR1, old);

	ASSERT_EQ(signal_test_handler, restored);
	signal(SIGUSR1, old); /* Restore original */
}

/* ============================================================================
 * POSIX SIGSET OPERATIONS
 * ============================================================================ */
#if JACL_HAS_POSIX

/* ============================================================================
 * SIGEMPTYSET FUNCTION
 * ============================================================================ */
TEST_SUITE(sigemptyset);

TEST(sigemptyset_basic) {
	sigset_t set;
	int result = sigemptyset(&set);

	ASSERT_EQ(0, result);
	ASSERT_TRUE(__sigisemptyset(&set));
}

TEST(sigemptyset_null_pointer) {
	errno = 0;
	ASSERT_EQ(-1, sigemptyset(NULL));
	ASSERT_EQ(EINVAL, errno);
}

TEST(sigemptyset_clears_all) {
	sigset_t set;
	sigfillset(&set);
	sigemptyset(&set);

	ASSERT_TRUE(__sigisemptyset(&set));
	ASSERT_FALSE(sigismember(&set, SIGINT));
	ASSERT_FALSE(sigismember(&set, SIGTERM));
}

#if JACL_HAS_C99
TEST(sigemptyset_fast_helper) {
	sigset_t set;
	__sigemptyset_fast(&set);

	ASSERT_TRUE(__sigisemptyset(&set));
}
#endif

/* ============================================================================
 * SIGFILLSET FUNCTION
 * ============================================================================ */
TEST_SUITE(sigfillset);

TEST(sigfillset_basic) {
	sigset_t set;
	int result = sigfillset(&set);

	ASSERT_EQ(0, result);
	ASSERT_FALSE(__sigisemptyset(&set));
}

TEST(sigfillset_null_pointer) {
	errno = 0;
	ASSERT_EQ(-1, sigfillset(NULL));
	ASSERT_EQ(EINVAL, errno);
}

TEST(sigfillset_sets_all) {
	sigset_t set;
	sigemptyset(&set);
	sigfillset(&set);

	ASSERT_TRUE(sigismember(&set, SIGINT));
	ASSERT_TRUE(sigismember(&set, SIGTERM));
	ASSERT_TRUE(sigismember(&set, SIGHUP));
}

#if JACL_HAS_C99
TEST(sigfillset_fast_helper) {
	sigset_t set;
	__sigfillset_fast(&set);

	ASSERT_FALSE(__sigisemptyset(&set));
}
#endif

/* ============================================================================
 * SIGADDSET FUNCTION
 * ============================================================================ */
TEST_SUITE(sigaddset);

TEST(sigaddset_basic) {
	sigset_t set;
	sigemptyset(&set);

	int result = sigaddset(&set, SIGINT);
	ASSERT_EQ(0, result);
	ASSERT_TRUE(sigismember(&set, SIGINT));
}

/* Test multiple standard signals across platforms */
#define TEST_SIGADDSET_FOR(sig) \
	TEST(sigaddset_##sig) { \
		sigset_t set; \
		sigemptyset(&set); \
		ASSERT_EQ(0, sigaddset(&set, sig)); \
		ASSERT_TRUE(sigismember(&set, sig)); \
	}

TEST_SIGADDSET_FOR(SIGINT)
TEST_SIGADDSET_FOR(SIGTERM)
TEST_SIGADDSET_FOR(SIGHUP)
TEST_SIGADDSET_FOR(SIGQUIT)
TEST_SIGADDSET_FOR(SIGKILL)
TEST_SIGADDSET_FOR(SIGUSR1)
TEST_SIGADDSET_FOR(SIGUSR2)

#undef TEST_SIGADDSET_FOR

TEST(sigaddset_multiple) {
	sigset_t set;
	sigemptyset(&set);

	sigaddset(&set, SIGINT);
	sigaddset(&set, SIGTERM);
	sigaddset(&set, SIGHUP);

	ASSERT_TRUE(sigismember(&set, SIGINT));
	ASSERT_TRUE(sigismember(&set, SIGTERM));
	ASSERT_TRUE(sigismember(&set, SIGHUP));
}

TEST(sigaddset_null_pointer) {
	errno = 0;
	ASSERT_EQ(-1, sigaddset(NULL, SIGINT));
	ASSERT_EQ(EINVAL, errno);
}

TEST(sigaddset_invalid_zero) {
	sigset_t set;
	sigemptyset(&set);

	errno = 0;
	ASSERT_EQ(-1, sigaddset(&set, 0));
	ASSERT_EQ(EINVAL, errno);
}

TEST(sigaddset_invalid_high) {
	sigset_t set;
	sigemptyset(&set);

	errno = 0;
	ASSERT_EQ(-1, sigaddset(&set, JACL_SIGSET_MAX + 1));
	ASSERT_EQ(EINVAL, errno);
}

TEST(sigaddset_boundary_first) {
	sigset_t set;
	sigemptyset(&set);

	ASSERT_EQ(0, sigaddset(&set, 1));
	ASSERT_TRUE(sigismember(&set, 1));
}

TEST(sigaddset_boundary_word) {
	sigset_t set;
	sigemptyset(&set);

	ASSERT_EQ(0, sigaddset(&set, 64));
	ASSERT_TRUE(sigismember(&set, 64));

	ASSERT_EQ(0, sigaddset(&set, 65));
	ASSERT_TRUE(sigismember(&set, 65));
}

TEST(sigaddset_bit_isolation) {
	sigset_t set;
	sigemptyset(&set);

	sigaddset(&set, SIGINT);

	ASSERT_TRUE(sigismember(&set, SIGINT));

	/* Only test isolation if we can safely check neighbors */
	if (SIGINT > 1) {
		ASSERT_FALSE(sigismember(&set, SIGINT - 1));
	}
	if (SIGINT < JACL_SIGSET_MAX) {
		ASSERT_FALSE(sigismember(&set, SIGINT + 1));
	}
}

#if JACL_HAS_C99
TEST(sigaddset_fast_helper) {
	sigset_t set;
	sigemptyset(&set);

	int result = __sigaddset_fast(&set, SIGINT);
	ASSERT_EQ(0, result);
	ASSERT_TRUE(sigismember(&set, SIGINT));
}
#endif

/* ============================================================================
 * SIGDELSET FUNCTION
 * ============================================================================ */
TEST_SUITE(sigdelset);

TEST(sigdelset_basic) {
	sigset_t set;
	sigfillset(&set);

	int result = sigdelset(&set, SIGINT);
	ASSERT_EQ(0, result);
	ASSERT_FALSE(sigismember(&set, SIGINT));
}

TEST(sigdelset_from_empty) {
	sigset_t set;
	sigemptyset(&set);

	int result = sigdelset(&set, SIGINT);
	ASSERT_EQ(0, result);
	ASSERT_FALSE(sigismember(&set, SIGINT));
}

TEST(sigdelset_null_pointer) {
	errno = 0;
	ASSERT_EQ(-1, sigdelset(NULL, SIGINT));
	ASSERT_EQ(EINVAL, errno);
}

TEST(sigdelset_invalid_zero) {
	sigset_t set;
	sigfillset(&set);

	errno = 0;
	ASSERT_EQ(-1, sigdelset(&set, 0));
	ASSERT_EQ(EINVAL, errno);
}

TEST(sigdelset_invalid_high) {
	sigset_t set;
	sigfillset(&set);

	errno = 0;
	ASSERT_EQ(-1, sigdelset(&set, JACL_SIGSET_MAX + 1));
	ASSERT_EQ(EINVAL, errno);
}

#if JACL_HAS_C99
TEST(sigdelset_fast_helper) {
	sigset_t set;
	sigfillset(&set);

	int result = __sigdelset_fast(&set, SIGINT);
	ASSERT_EQ(0, result);
	ASSERT_FALSE(sigismember(&set, SIGINT));
}
#endif

/* ============================================================================
 * SIGISMEMBER FUNCTION
 * ============================================================================ */
TEST_SUITE(sigismember);

TEST(sigismember_present) {
	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, SIGINT);

	ASSERT_TRUE(sigismember(&set, SIGINT));
}

TEST(sigismember_absent) {
	sigset_t set;
	sigemptyset(&set);

	ASSERT_FALSE(sigismember(&set, SIGINT));
}

TEST(sigismember_add_remove) {
	sigset_t set;
	sigemptyset(&set);

	sigaddset(&set, SIGINT);
	ASSERT_TRUE(sigismember(&set, SIGINT));

	sigdelset(&set, SIGINT);
	ASSERT_FALSE(sigismember(&set, SIGINT));
}

TEST(sigismember_null_pointer) {
	errno = 0;
	ASSERT_EQ(-1, sigismember(NULL, SIGINT));
	ASSERT_EQ(EINVAL, errno);
}

TEST(sigismember_invalid_zero) {
	sigset_t set;
	sigemptyset(&set);

	errno = 0;
	ASSERT_EQ(-1, sigismember(&set, 0));
	ASSERT_EQ(EINVAL, errno);
}

TEST(sigismember_invalid_high) {
	sigset_t set;
	sigemptyset(&set);

	errno = 0;
	ASSERT_EQ(-1, sigismember(&set, JACL_SIGSET_MAX + 1));
	ASSERT_EQ(EINVAL, errno);
}

/* ============================================================================
 * SIGACTION FUNCTION
 * ============================================================================ */
TEST_SUITE(sigaction);

TEST(sigaction_flags_restart) {
	struct sigaction act;

	act.sa_flags = SA_RESTART;
	ASSERT_EQ(SA_RESTART, act.sa_flags);
}

TEST(sigaction_flags_nocldstop) {
	struct sigaction act;

	act.sa_flags = SA_NOCLDSTOP;
	ASSERT_EQ(SA_NOCLDSTOP, act.sa_flags);
}

TEST(sigaction_flags_combined) {
	struct sigaction act;

	act.sa_flags = SA_RESTART | SA_NOCLDSTOP;
	ASSERT_TRUE(act.sa_flags & SA_RESTART);
	ASSERT_TRUE(act.sa_flags & SA_NOCLDSTOP);
}

TEST(sigaction_mask_operations) {
	struct sigaction act;
	sigemptyset(&act.sa_mask);

	sigaddset(&act.sa_mask, SIGINT);
	sigaddset(&act.sa_mask, SIGTERM);

	ASSERT_TRUE(sigismember(&act.sa_mask, SIGINT));
	ASSERT_TRUE(sigismember(&act.sa_mask, SIGTERM));
}

/* ============================================================================
 * SIGSET INTEGRITY
 * ============================================================================ */
TEST_SUITE(sigset_integrity);

TEST(sigset_integrity_capacity) {
	ASSERT_TRUE(JACL_SIGSET_MAX >= 64);
}

TEST(sigset_integrity_independent_bits) {
	sigset_t set;
	sigemptyset(&set);

	/* Add every 8th signal up to 64 */
	for (int sig = 1; sig <= 64 && sig <= JACL_SIGSET_MAX; sig += 8) {
		sigaddset(&set, sig);
	}

	/* Verify only those are set */
	for (int sig = 1; sig <= 64 && sig <= JACL_SIGSET_MAX; sig++) {
		if (sig % 8 == 1) {
			ASSERT_TRUE(sigismember(&set, sig));
		} else {
			ASSERT_FALSE(sigismember(&set, sig));
		}
	}
}

TEST(sigset_integrity_word_boundary) {
	sigset_t set;
	sigemptyset(&set);

	/* Test across 64-bit word boundary */
	if (JACL_SIGSET_MAX >= 65) {
		sigaddset(&set, 64);
		sigaddset(&set, 65);

		ASSERT_TRUE(sigismember(&set, 64));
		ASSERT_TRUE(sigismember(&set, 65));
		ASSERT_FALSE(sigismember(&set, 63));

		if (JACL_SIGSET_MAX >= 66) {
			ASSERT_FALSE(sigismember(&set, 66));
		}
	}
}

#if JACL_HAS_C99
TEST(sigset_integrity_isempty_helper) {
	sigset_t set;
	sigemptyset(&set);

	ASSERT_TRUE(__sigisemptyset(&set));

	sigaddset(&set, SIGINT);
	ASSERT_FALSE(__sigisemptyset(&set));
}
#endif

#endif /* JACL_HAS_POSIX */

TEST_MAIN()
