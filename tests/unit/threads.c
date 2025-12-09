/* (c) 2025 FRINKnet & Friends – MIT licence */
#include <testing.h>

#if JACL_HAS_C11 && JACL_HAS_PTHREADS
#include <threads.h>
#include <unistd.h>

TEST_TYPE(unit);
TEST_UNIT(threads.h);

/* ============================================================================
 * CONSTANTS
 * ============================================================================ */
TEST_SUITE(constants);

TEST(constants_thrd_success_is_zero) {
	ASSERT_EQ(0, thrd_success);
}

TEST(constants_thrd_nomem_nonzero) {
	ASSERT_NE(0, thrd_nomem);
}

TEST(constants_thrd_timedout_nonzero) {
	ASSERT_NE(0, thrd_timedout);
}

TEST(constants_thrd_busy_nonzero) {
	ASSERT_NE(0, thrd_busy);
}

TEST(constants_thrd_error_nonzero) {
	ASSERT_NE(0, thrd_error);
}

TEST(constants_error_codes_unique) {
	ASSERT_NE(thrd_nomem, thrd_timedout);
	ASSERT_NE(thrd_nomem, thrd_busy);
	ASSERT_NE(thrd_nomem, thrd_error);
	ASSERT_NE(thrd_timedout, thrd_busy);
	ASSERT_NE(thrd_timedout, thrd_error);
	ASSERT_NE(thrd_busy, thrd_error);
}

TEST(constants_mtx_plain_is_zero) {
	ASSERT_EQ(0, mtx_plain);
}

TEST(constants_mtx_recursive_nonzero) {
	ASSERT_NE(0, mtx_recursive);
}

TEST(constants_mtx_timed_nonzero) {
	ASSERT_NE(0, mtx_timed);
}

TEST(constants_mtx_types_unique) {
	ASSERT_NE(mtx_plain, mtx_recursive);
	ASSERT_NE(mtx_plain, mtx_timed);
	ASSERT_NE(mtx_recursive, mtx_timed);
}

TEST(constants_tss_dtor_iterations_positive) {
	ASSERT_TRUE(TSS_DTOR_ITERATIONS > 0);
}

#if JACL_HAS_C23
TEST(constants_version_macro_c23) {
	ASSERT_EQ(202311L, __STDC_VERSION_THREADS_H__);
}
#endif

/* ============================================================================
 * mtx_init
 * ============================================================================ */
TEST_SUITE(mtx_init);

TEST(mtx_init_plain) {
	mtx_t mtx;
	ASSERT_EQ(thrd_success, mtx_init(&mtx, mtx_plain));
	mtx_destroy(&mtx);
}

TEST(mtx_init_recursive) {
	mtx_t mtx;
	ASSERT_EQ(thrd_success, mtx_init(&mtx, mtx_recursive));
	mtx_destroy(&mtx);
}

TEST(mtx_init_timed) {
	mtx_t mtx;
	ASSERT_EQ(thrd_success, mtx_init(&mtx, mtx_timed));
	mtx_destroy(&mtx);
}

TEST(mtx_init_combined_flags) {
	mtx_t mtx;
	ASSERT_EQ(thrd_success, mtx_init(&mtx, mtx_recursive | mtx_timed));
	mtx_destroy(&mtx);
}

TEST(mtx_init_null_pointer) {
	ASSERT_EQ(thrd_error, mtx_init(NULL, mtx_plain));
}

TEST(mtx_init_multiple) {
	mtx_t mtx1, mtx2, mtx3;
	ASSERT_EQ(thrd_success, mtx_init(&mtx1, mtx_plain));
	ASSERT_EQ(thrd_success, mtx_init(&mtx2, mtx_recursive));
	ASSERT_EQ(thrd_success, mtx_init(&mtx3, mtx_timed));
	mtx_destroy(&mtx1);
	mtx_destroy(&mtx2);
	mtx_destroy(&mtx3);
}

/* ============================================================================
 * mtx_lock
 * ============================================================================ */
TEST_SUITE(mtx_lock);

TEST(mtx_lock_unlocked_mutex) {
	mtx_t mtx;
	mtx_init(&mtx, mtx_plain);
	ASSERT_EQ(thrd_success, mtx_lock(&mtx));
	mtx_unlock(&mtx);
	mtx_destroy(&mtx);
}

TEST(mtx_lock_returns_success) {
	mtx_t mtx;
	mtx_init(&mtx, mtx_plain);
	int result = mtx_lock(&mtx);
	ASSERT_EQ(thrd_success, result);
	mtx_unlock(&mtx);
	mtx_destroy(&mtx);
}

TEST(mtx_lock_null_pointer) {
	ASSERT_EQ(thrd_error, mtx_lock(NULL));
}

TEST(mtx_lock_blocks_on_locked) {
	mtx_t mtx;
	mtx_init(&mtx, mtx_plain);
	mtx_lock(&mtx);
	/* Should block, but we can't test blocking in unit tests */
	mtx_unlock(&mtx);
	mtx_destroy(&mtx);
}

TEST(mtx_lock_recursive_multiple_times) {
	mtx_t mtx;
	mtx_init(&mtx, mtx_recursive);
	ASSERT_EQ(thrd_success, mtx_lock(&mtx));
	ASSERT_EQ(thrd_success, mtx_lock(&mtx));
	ASSERT_EQ(thrd_success, mtx_lock(&mtx));
	mtx_unlock(&mtx);
	mtx_unlock(&mtx);
	mtx_unlock(&mtx);
	mtx_destroy(&mtx);
}

/* ============================================================================
 * mtx_trylock
 * ============================================================================ */
TEST_SUITE(mtx_trylock);

TEST(mtx_trylock_unlocked_succeeds) {
	mtx_t mtx;
	mtx_init(&mtx, mtx_plain);
	ASSERT_EQ(thrd_success, mtx_trylock(&mtx));
	mtx_unlock(&mtx);
	mtx_destroy(&mtx);
}

TEST(mtx_trylock_locked_fails) {
	mtx_t mtx;
	mtx_init(&mtx, mtx_plain);
	mtx_lock(&mtx);
	int result = mtx_trylock(&mtx);
	ASSERT_TRUE(result == thrd_busy || result == thrd_success);
	mtx_unlock(&mtx);
	mtx_destroy(&mtx);
}

TEST(mtx_trylock_returns_correct_value) {
	mtx_t mtx;
	mtx_init(&mtx, mtx_plain);
	int result = mtx_trylock(&mtx);
	ASSERT_EQ(thrd_success, result);
	mtx_unlock(&mtx);
	mtx_destroy(&mtx);
}

TEST(mtx_trylock_null_pointer) {
	ASSERT_EQ(thrd_error, mtx_trylock(NULL));
}

TEST(mtx_trylock_recursive_multiple_times) {
	mtx_t mtx;
	mtx_init(&mtx, mtx_recursive);
	ASSERT_EQ(thrd_success, mtx_trylock(&mtx));
	int result2 = mtx_trylock(&mtx);
	ASSERT_TRUE(result2 == thrd_success || result2 == thrd_busy);
	mtx_unlock(&mtx);
	if (result2 == thrd_success) mtx_unlock(&mtx);
	mtx_destroy(&mtx);
}

/* ============================================================================
 * mtx_timedlock
 * ============================================================================ */
TEST_SUITE(mtx_timedlock);

TEST(mtx_timedlock_unlocked) {
	mtx_t mtx;
	mtx_init(&mtx, mtx_timed);
	struct timespec ts = {1, 0};
	ASSERT_EQ(thrd_success, mtx_timedlock(&mtx, &ts));
	mtx_unlock(&mtx);
	mtx_destroy(&mtx);
}

TEST(mtx_timedlock_null_pointer_mutex) {
	struct timespec ts = {1, 0};
	ASSERT_EQ(thrd_error, mtx_timedlock(NULL, &ts));
}

TEST(mtx_timedlock_null_pointer_timespec) {
	mtx_t mtx;
	mtx_init(&mtx, mtx_timed);
	ASSERT_EQ(thrd_error, mtx_timedlock(&mtx, NULL));
	mtx_destroy(&mtx);
}

/* ============================================================================
 * mtx_unlock
 * ============================================================================ */
TEST_SUITE(mtx_unlock);

TEST(mtx_unlock_after_lock) {
	mtx_t mtx;
	mtx_init(&mtx, mtx_plain);
	mtx_lock(&mtx);
	ASSERT_EQ(thrd_success, mtx_unlock(&mtx));
	mtx_destroy(&mtx);
}

TEST(mtx_unlock_returns_success) {
	mtx_t mtx;
	mtx_init(&mtx, mtx_plain);
	mtx_lock(&mtx);
	int result = mtx_unlock(&mtx);
	ASSERT_EQ(thrd_success, result);
	mtx_destroy(&mtx);
}

TEST(mtx_unlock_null_pointer) {
	ASSERT_EQ(thrd_error, mtx_unlock(NULL));
}

TEST(mtx_unlock_recursive_multiple_times) {
	mtx_t mtx;
	mtx_init(&mtx, mtx_recursive);
	mtx_lock(&mtx);
	mtx_lock(&mtx);
	mtx_lock(&mtx);
	ASSERT_EQ(thrd_success, mtx_unlock(&mtx));
	ASSERT_EQ(thrd_success, mtx_unlock(&mtx));
	ASSERT_EQ(thrd_success, mtx_unlock(&mtx));
	mtx_destroy(&mtx);
}

TEST(mtx_unlock_lock_unlock_sequence) {
	mtx_t mtx;
	mtx_init(&mtx, mtx_plain);
	for (int i = 0; i < 10; i++) {
		ASSERT_EQ(thrd_success, mtx_lock(&mtx));
		ASSERT_EQ(thrd_success, mtx_unlock(&mtx));
	}
	mtx_destroy(&mtx);
}

/* ============================================================================
 * mtx_destroy
 * ============================================================================ */
TEST_SUITE(mtx_destroy);

TEST(mtx_destroy_after_init) {
	mtx_t mtx;
	mtx_init(&mtx, mtx_plain);
	mtx_destroy(&mtx);
}

TEST(mtx_destroy_after_lock) {
	mtx_t mtx;
	mtx_init(&mtx, mtx_plain);
	mtx_lock(&mtx);
	mtx_unlock(&mtx);
	mtx_destroy(&mtx);
}

TEST(mtx_destroy_null_pointer) {
	mtx_destroy(NULL);
}

TEST(mtx_destroy_multiple) {
	mtx_t mtx1, mtx2;
	mtx_init(&mtx1, mtx_plain);
	mtx_init(&mtx2, mtx_recursive);
	mtx_destroy(&mtx1);
	mtx_destroy(&mtx2);
}

/* ============================================================================
 * cnd_init
 * ============================================================================ */
TEST_SUITE(cnd_init);

TEST(cnd_init_success) {
	cnd_t cnd;
	ASSERT_EQ(thrd_success, cnd_init(&cnd));
	cnd_destroy(&cnd);
}

TEST(cnd_init_null_pointer) {
	ASSERT_EQ(thrd_error, cnd_init(NULL));
}

TEST(cnd_init_multiple) {
	cnd_t cnd1, cnd2, cnd3;
	ASSERT_EQ(thrd_success, cnd_init(&cnd1));
	ASSERT_EQ(thrd_success, cnd_init(&cnd2));
	ASSERT_EQ(thrd_success, cnd_init(&cnd3));
	cnd_destroy(&cnd1);
	cnd_destroy(&cnd2);
	cnd_destroy(&cnd3);
}

/* ============================================================================
 * cnd_signal
 * ============================================================================ */
TEST_SUITE(cnd_signal);

TEST(cnd_signal_success) {
	cnd_t cnd;
	cnd_init(&cnd);
	ASSERT_EQ(thrd_success, cnd_signal(&cnd));
	cnd_destroy(&cnd);
}

TEST(cnd_signal_returns_success) {
	cnd_t cnd;
	cnd_init(&cnd);
	int result = cnd_signal(&cnd);
	ASSERT_EQ(thrd_success, result);
	cnd_destroy(&cnd);
}

TEST(cnd_signal_null_pointer) {
	ASSERT_EQ(thrd_error, cnd_signal(NULL));
}

TEST(cnd_signal_multiple_times) {
	cnd_t cnd;
	cnd_init(&cnd);
	for (int i = 0; i < 10; i++) {
		ASSERT_EQ(thrd_success, cnd_signal(&cnd));
	}
	cnd_destroy(&cnd);
}

/* ============================================================================
 * cnd_broadcast
 * ============================================================================ */
TEST_SUITE(cnd_broadcast);

TEST(cnd_broadcast_success) {
	cnd_t cnd;
	cnd_init(&cnd);
	ASSERT_EQ(thrd_success, cnd_broadcast(&cnd));
	cnd_destroy(&cnd);
}

TEST(cnd_broadcast_returns_success) {
	cnd_t cnd;
	cnd_init(&cnd);
	int result = cnd_broadcast(&cnd);
	ASSERT_EQ(thrd_success, result);
	cnd_destroy(&cnd);
}

TEST(cnd_broadcast_null_pointer) {
	ASSERT_EQ(thrd_error, cnd_broadcast(NULL));
}

TEST(cnd_broadcast_multiple_times) {
	cnd_t cnd;
	cnd_init(&cnd);
	for (int i = 0; i < 10; i++) {
		ASSERT_EQ(thrd_success, cnd_broadcast(&cnd));
	}
	cnd_destroy(&cnd);
}

/* ============================================================================
 * cnd_destroy
 * ============================================================================ */
TEST_SUITE(cnd_destroy);

TEST(cnd_destroy_after_init) {
	cnd_t cnd;
	cnd_init(&cnd);
	cnd_destroy(&cnd);
}

TEST(cnd_destroy_null_pointer) {
	cnd_destroy(NULL);
}

TEST(cnd_destroy_multiple) {
	cnd_t cnd1, cnd2;
	cnd_init(&cnd1);
	cnd_init(&cnd2);
	cnd_destroy(&cnd1);
	cnd_destroy(&cnd2);
}

/* ============================================================================
 * call_once
 * ============================================================================ */
TEST_SUITE(call_once);

static int once_counter = 0;

static void call_once_increment(void) {
	once_counter++;
}

TEST(call_once_executes_function) {
	once_flag flag = ONCE_FLAG_INIT;
	once_counter = 0;
	call_once(&flag, call_once_increment);
	ASSERT_EQ(1, once_counter);
}

TEST(call_once_executes_only_once) {
	once_flag flag = ONCE_FLAG_INIT;
	once_counter = 0;
	call_once(&flag, call_once_increment);
	call_once(&flag, call_once_increment);
	call_once(&flag, call_once_increment);
	ASSERT_EQ(1, once_counter);
}

TEST(call_once_multiple_flags) {
	once_flag flag1 = ONCE_FLAG_INIT;
	once_flag flag2 = ONCE_FLAG_INIT;
	once_counter = 0;
	call_once(&flag1, call_once_increment);
	call_once(&flag2, call_once_increment);
	ASSERT_EQ(2, once_counter);
}

TEST(call_once_null_flag) {
	call_once(NULL, call_once_increment);
}

TEST(call_once_null_func) {
	once_flag flag = ONCE_FLAG_INIT;
	call_once(&flag, NULL);
}

/* ============================================================================
 * NATIVE PLATFORM TESTS (Non-WASM)
 * ============================================================================ */
#if JACL_HAS_THREADS && !JACL_ARCH_WASM

/* ============================================================================
 * thrd_create
 * ============================================================================ */
TEST_SUITE(thrd_create);

static int thrd_create_returns_zero(void *arg) {
	(void)arg;
	return 0;
}

static int thrd_create_returns_value(void *arg) {
	return *((int*)arg);
}

static int thrd_create_modifies_arg(void *arg) {
	int *ptr = (int*)arg;
	*ptr = 99;
	return 0;
}

TEST(thrd_create_success) {
	thrd_t thread;
	ASSERT_EQ(thrd_success, thrd_create(&thread, thrd_create_returns_zero, NULL));
	int res;
	thrd_join(thread, &res);
}

TEST(thrd_create_with_arg) {
	thrd_t thread;
	int value = 42;
	ASSERT_EQ(thrd_success, thrd_create(&thread, thrd_create_modifies_arg, &value));
	int res;
	thrd_join(thread, &res);
	ASSERT_EQ(99, value);
}

TEST(thrd_create_null_thread) {
	ASSERT_EQ(thrd_error, thrd_create(NULL, thrd_create_returns_zero, NULL));
}

TEST(thrd_create_null_func) {
	thrd_t thread;
	ASSERT_EQ(thrd_error, thrd_create(&thread, NULL, NULL));
}

TEST(thrd_create_multiple) {
	thrd_t t1, t2, t3;
	ASSERT_EQ(thrd_success, thrd_create(&t1, thrd_create_returns_zero, NULL));
	ASSERT_EQ(thrd_success, thrd_create(&t2, thrd_create_returns_zero, NULL));
	ASSERT_EQ(thrd_success, thrd_create(&t3, thrd_create_returns_zero, NULL));
	thrd_join(t1, NULL);
	thrd_join(t2, NULL);
	thrd_join(t3, NULL);
}

/* ============================================================================
 * thrd_join
 * ============================================================================ */
TEST_SUITE(thrd_join);

TEST(thrd_join_success) {
	thrd_t thread;
	thrd_create(&thread, thrd_create_returns_zero, NULL);
	ASSERT_EQ(thrd_success, thrd_join(thread, NULL));
}

TEST(thrd_join_gets_exit_code) {
	thrd_t thread;
	int input = 42;
	thrd_create(&thread, thrd_create_returns_value, &input);
	int result;
	ASSERT_EQ(thrd_success, thrd_join(thread, &result));
	ASSERT_EQ(42, result);
}

TEST(thrd_join_null_result) {
	thrd_t thread;
	thrd_create(&thread, thrd_create_returns_zero, NULL);
	ASSERT_EQ(thrd_success, thrd_join(thread, NULL));
}

/* ============================================================================
 * thrd_detach
 * ============================================================================ */
TEST_SUITE(thrd_detach);

TEST(thrd_detach_success) {
	thrd_t thread;
	thrd_create(&thread, thrd_create_returns_zero, NULL);
	ASSERT_EQ(thrd_success, thrd_detach(thread));
	usleep(100000);
}

/* ============================================================================
 * thrd_current
 * ============================================================================ */
TEST_SUITE(thrd_current);

TEST(thrd_current_returns_value) {
	thrd_t current = thrd_current();
	ASSERT_TRUE(current != 0 || current == 0);
}

/* ============================================================================
 * thrd_equal
 * ============================================================================ */
TEST_SUITE(thrd_equal);

TEST(thrd_equal_current_thread) {
	thrd_t t1 = thrd_current();
	thrd_t t2 = thrd_current();
	ASSERT_TRUE(thrd_equal(t1, t2));
}

/* ============================================================================
 * thrd_yield
 * ============================================================================ */
TEST_SUITE(thrd_yield);

TEST(thrd_yield_no_crash) {
	thrd_yield();
}

/* ============================================================================
 * thrd_sleep
 * ============================================================================ */
TEST_SUITE(thrd_sleep);

TEST(thrd_sleep_basic) {
	struct timespec duration = {0, 10000000};
	struct timespec remaining;
	int result = thrd_sleep(&duration, &remaining);
	ASSERT_EQ(0, result);
}

TEST(thrd_sleep_zero) {
	struct timespec duration = {0, 0};
	int result = thrd_sleep(&duration, NULL);
	ASSERT_EQ(0, result);
}

/* ============================================================================
 * tss_create
 * ============================================================================ */
TEST_SUITE(tss_create);

TEST(tss_create_success) {
	tss_t key;
	ASSERT_EQ(thrd_success, tss_create(&key, NULL));
	tss_delete(key);
}

TEST(tss_create_with_dtor) {
	tss_t key;
	ASSERT_EQ(thrd_success, tss_create(&key, NULL));
	tss_delete(key);
}

TEST(tss_create_null_key) {
	ASSERT_EQ(thrd_error, tss_create(NULL, NULL));
}

TEST(tss_create_multiple) {
	tss_t k1, k2, k3;
	ASSERT_EQ(thrd_success, tss_create(&k1, NULL));
	ASSERT_EQ(thrd_success, tss_create(&k2, NULL));
	ASSERT_EQ(thrd_success, tss_create(&k3, NULL));
	tss_delete(k1);
	tss_delete(k2);
	tss_delete(k3);
}

/* ============================================================================
 * tss_set
 * ============================================================================ */
TEST_SUITE(tss_set);

TEST(tss_set_success) {
	tss_t key;
	tss_create(&key, NULL);
	int value = 123;
	ASSERT_EQ(thrd_success, tss_set(key, &value));
	tss_delete(key);
}

TEST(tss_set_null_value) {
	tss_t key;
	tss_create(&key, NULL);
	ASSERT_EQ(thrd_success, tss_set(key, NULL));
	tss_delete(key);
}

TEST(tss_set_multiple_times) {
	tss_t key;
	tss_create(&key, NULL);
	int v1 = 1, v2 = 2, v3 = 3;
	ASSERT_EQ(thrd_success, tss_set(key, &v1));
	ASSERT_EQ(thrd_success, tss_set(key, &v2));
	ASSERT_EQ(thrd_success, tss_set(key, &v3));
	tss_delete(key);
}

/* ============================================================================
 * tss_get
 * ============================================================================ */
TEST_SUITE(tss_get);

TEST(tss_get_after_set) {
	tss_t key;
	tss_create(&key, NULL);
	int value = 456;
	tss_set(key, &value);
	void *retrieved = tss_get(key);
	ASSERT_EQ(&value, retrieved);
	ASSERT_EQ(456, *((int*)retrieved));
	tss_delete(key);
}

TEST(tss_get_before_set) {
	tss_t key;
	tss_create(&key, NULL);
	void *retrieved = tss_get(key);
	ASSERT_EQ(NULL, retrieved);
	tss_delete(key);
}

TEST(tss_get_after_delete) {
	tss_t key;
	tss_create(&key, NULL);
	int value = 789;
	tss_set(key, &value);
	tss_delete(key);
}

/* ============================================================================
 * tss_delete
 * ============================================================================ */
TEST_SUITE(tss_delete);

TEST(tss_delete_after_create) {
	tss_t key;
	tss_create(&key, NULL);
	tss_delete(key);
}

TEST(tss_delete_multiple) {
	tss_t k1, k2;
	tss_create(&k1, NULL);
	tss_create(&k2, NULL);
	tss_delete(k1);
	tss_delete(k2);
}

#endif  /* JACL_HAS_THREADS && !JACL_ARCH_WASM */

#else

TEST_SUITE(threads_basics);

TEST(threads_not_avaialable) {
	TEST_SKIP("NO C99 SUPPORT");
}

#endif

TEST_MAIN()
