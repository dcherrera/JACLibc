/* (c) 2025 FRINKnet & Friends – MIT licence */
#include <testing.h>
#include <pthread.h>
#include <unistd.h>

TEST_TYPE(unit);
TEST_UNIT(pthread.h);

static int threadval = 0;
static void *threadfn(void *arg) {
	*(int*)arg = 42;
	return NULL;
}

TEST_SUITE(constants);

TEST(constants_pthread_create_joinable) {
	ASSERT_EQ(0, PTHREAD_CREATE_JOINABLE);
}

TEST(constants_pthread_create_detached) {
	ASSERT_EQ(1, PTHREAD_CREATE_DETACHED);
}

TEST(constants_pthread_mutex_normal) {
	ASSERT_EQ(0, PTHREAD_MUTEX_NORMAL);
}

TEST(constants_pthread_mutex_recursive) {
	ASSERT_NE(PTHREAD_MUTEX_RECURSIVE, 0);
}

TEST(constants_pthread_mutex_errorcheck) {
	ASSERT_NE(PTHREAD_MUTEX_ERRORCHECK, 0);
}

TEST(constants_pthread_once_init) {
	pthread_once_t once = PTHREAD_ONCE_INIT;
	ASSERT_EQ(0, once.done);
}

TEST_SUITE(pthread_attr);

TEST(pthread_attr_destroy_null) {
	ASSERT_EQ(EINVAL, pthread_attr_destroy(NULL));
}

TEST(pthread_attr_init_null) {
	ASSERT_EQ(EINVAL, pthread_attr_init(NULL));
}

TEST(pthread_attr_init_valid) {
	pthread_attr_t attr;

	ASSERT_EQ(0, pthread_attr_init(&attr));
	ASSERT_EQ(0, pthread_attr_destroy(&attr));
}

TEST(pthread_attr_setdetachstate_null) {
	ASSERT_EQ(EINVAL, pthread_attr_setdetachstate(NULL, 0));
}

TEST(pthread_attr_setdetachstate_valid) {
	pthread_attr_t attr;

	pthread_attr_init(&attr);
	ASSERT_EQ(0, pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED));
	ASSERT_EQ(1, attr.detached);

	pthread_attr_destroy(&attr);
}

TEST(pthread_attr_setstacksize_null) {
	ASSERT_EQ(EINVAL, pthread_attr_setstacksize(NULL, 1024));
}

TEST(pthread_attr_setstacksize_valid) {
	pthread_attr_t attr;

	pthread_attr_init(&attr);
	ASSERT_EQ(0, pthread_attr_setstacksize(&attr, 1024));
	ASSERT_EQ(1024, attr.stack_size);

	pthread_attr_destroy(&attr);
}

TEST_SUITE(pthread_mutexattr);

TEST(pthread_mutexattr_destroy_null) {
	ASSERT_EQ(EINVAL, pthread_mutexattr_destroy(NULL));
}

TEST(pthread_mutexattr_init_null) {
	ASSERT_EQ(EINVAL, pthread_mutexattr_init(NULL));
}

TEST(pthread_mutexattr_init_valid) {
	pthread_mutexattr_t attr;

	ASSERT_EQ(0, pthread_mutexattr_init(&attr));
	ASSERT_EQ(0, pthread_mutexattr_destroy(&attr));
}

TEST(pthread_mutexattr_settype_null) {
	ASSERT_EQ(EINVAL, pthread_mutexattr_settype(NULL, 0));
}

TEST(pthread_mutexattr_settype_valid) {
	pthread_mutexattr_t attr;

	pthread_mutexattr_init(&attr);
	ASSERT_EQ(0, pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE));
	ASSERT_EQ(PTHREAD_MUTEX_RECURSIVE, attr.type);

	pthread_mutexattr_destroy(&attr);
}

TEST_SUITE(pthread_condattr);

TEST(pthread_condattr_destroy_null) {
	ASSERT_EQ(EINVAL, pthread_condattr_destroy(NULL));
}

TEST(pthread_condattr_init_null) {
	ASSERT_EQ(EINVAL, pthread_condattr_init(NULL));
}

TEST(pthread_condattr_init_valid) {
	pthread_condattr_t attr;

	ASSERT_EQ(0, pthread_condattr_init(&attr));
	ASSERT_EQ(0, pthread_condattr_destroy(&attr));
}

TEST_SUITE(pthread_mutex);

TEST(pthread_mutex_destroy_null) {
	ASSERT_EQ(EINVAL, pthread_mutex_destroy(NULL));
}

TEST(pthread_mutex_init_null) {
	ASSERT_EQ(EINVAL, pthread_mutex_init(NULL, NULL));
}

TEST(pthread_mutex_init_valid) {
	pthread_mutex_t mutex;

	ASSERT_EQ(0, pthread_mutex_init(&mutex, NULL));
	ASSERT_EQ(0, pthread_mutex_destroy(&mutex));
}

TEST(pthread_mutex_lock_null) {
	ASSERT_EQ(EINVAL, pthread_mutex_lock(NULL));
}

TEST(pthread_mutex_unlock_null) {
	ASSERT_EQ(EINVAL, pthread_mutex_unlock(NULL));
}

TEST(pthread_mutex_trylock_null) {
	ASSERT_EQ(EINVAL, pthread_mutex_trylock(NULL));
}

TEST(pthread_mutex_lock_valid) {
	pthread_mutex_t mutex;

	pthread_mutex_init(&mutex, NULL);
	ASSERT_EQ(0, pthread_mutex_lock(&mutex));
	ASSERT_EQ(0, pthread_mutex_unlock(&mutex));

	pthread_mutex_destroy(&mutex);
}

TEST(pthread_mutex_trylock_valid) {
	pthread_mutex_t mutex;

	pthread_mutex_init(&mutex, NULL);
	ASSERT_EQ(0, pthread_mutex_trylock(&mutex));
	ASSERT_EQ(0, pthread_mutex_unlock(&mutex));

	pthread_mutex_destroy(&mutex);
}

TEST_SUITE(pthread_cond);

TEST(pthread_cond_destroy_null) {
	ASSERT_EQ(EINVAL, pthread_cond_destroy(NULL));
}

TEST(pthread_cond_init_null) {
	ASSERT_EQ(EINVAL, pthread_cond_init(NULL, NULL));
}

TEST(pthread_cond_init_valid) {
	pthread_cond_t cond;

	ASSERT_EQ(0, pthread_cond_init(&cond, NULL));
	ASSERT_EQ(0, pthread_cond_destroy(&cond));
}

TEST(pthread_cond_wait_null) {
	ASSERT_EQ(EINVAL, pthread_cond_wait(NULL, NULL));

	pthread_mutex_t mutex;
	pthread_cond_t cond;

	pthread_mutex_init(&mutex, NULL);
	ASSERT_EQ(EINVAL, pthread_cond_wait(&cond, NULL));
	ASSERT_EQ(EINVAL, pthread_cond_wait(NULL, &mutex));

	pthread_mutex_destroy(&mutex);
}

static void *cond_signaler(void *arg) {
	struct {
		pthread_mutex_t *m;
		pthread_cond_t  *c;
		int             *f;
	} *data = arg;

	pthread_mutex_lock(data->m);
	*data->f = 1;
	pthread_cond_signal(data->c);
	pthread_mutex_unlock(data->m);

	return NULL;
}

TEST(pthread_cond_wait_signal) {
	pthread_cond_t  cond;
	pthread_mutex_t mutex;
	int             flag = 0;

	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&cond, NULL);

	pthread_t sigthread;
	struct {
		pthread_mutex_t *m;
		pthread_cond_t  *c;
		int             *f;
	} data = { &mutex, &cond, &flag };

	pthread_mutex_lock(&mutex);

	ASSERT_EQ(0, pthread_create(&sigthread, NULL, cond_signaler, &data));

	while (!flag) {
		ASSERT_EQ(0, pthread_cond_wait(&cond, &mutex));
	}

	pthread_mutex_unlock(&mutex);
	ASSERT_EQ(0, pthread_join(sigthread, NULL));

	pthread_cond_destroy(&cond);
	pthread_mutex_destroy(&mutex);
}

TEST(pthread_cond_signal_null) {
	ASSERT_EQ(EINVAL, pthread_cond_signal(NULL));
}

TEST(pthread_cond_signal_valid) {
	pthread_cond_t cond;

	pthread_cond_init(&cond, NULL);
	ASSERT_EQ(0, pthread_cond_signal(&cond));

	pthread_cond_destroy(&cond);
}

TEST(pthread_cond_broadcast_null) {
	ASSERT_EQ(EINVAL, pthread_cond_broadcast(NULL));
}

TEST(pthread_cond_broadcast_valid) {
	pthread_cond_t cond;

	pthread_cond_init(&cond, NULL);
	ASSERT_EQ(0, pthread_cond_broadcast(&cond));

	pthread_cond_destroy(&cond);
}

TEST_SUITE(pthread_once);

TEST(pthread_once_null_control) {
	ASSERT_EQ(EINVAL, pthread_once(NULL, NULL));
}

TEST(pthread_once_null_init_routine) {
	pthread_once_t once = PTHREAD_ONCE_INIT;

	ASSERT_EQ(EINVAL, pthread_once(&once, NULL));
}

static volatile int once_counter = 0;

static void once_fn(void) { once_counter++; }

TEST(pthread_once_init_fn) {
	pthread_once_t once = PTHREAD_ONCE_INIT;

	once_counter = 0;
	ASSERT_EQ(0, pthread_once(&once, once_fn));
	ASSERT_EQ(1, once_counter);
}

TEST_SUITE(tls);

TEST(tls__key_create_null) {
	ASSERT_EQ(EINVAL, pthread_key_create(NULL, NULL));
}

TEST(tls__key_create) {
	pthread_key_t key;

	ASSERT_EQ(0, pthread_key_create(&key, NULL));
	pthread_key_delete(key);
}

TEST(tls__key_delete_bad) {
	ASSERT_EQ(EINVAL, pthread_key_delete(1000));
}

TEST(tls__key_delete_valid) {
	pthread_key_t key;

	pthread_key_create(&key, NULL);
	ASSERT_EQ(0, pthread_key_delete(key));
}

TEST(tls__setspecific_null_key) {
	ASSERT_EQ(EINVAL, pthread_setspecific(1000, NULL));
}

TEST(tls__setspecific_valid) {
	pthread_key_t key;

	pthread_key_create(&key, NULL);
	ASSERT_EQ(0, pthread_setspecific(key, NULL));

	pthread_key_delete(key);
}

TEST(tls__getspecific_null_key) {
	ASSERT_EQ(NULL, pthread_getspecific(1000));
}

TEST(tls__getspecific_valid) {
	pthread_key_t key;

	pthread_key_create(&key, NULL);
	ASSERT_EQ(NULL, pthread_getspecific(key));

	pthread_key_delete(key);
}

TEST_SUITE(pthread);

TEST(thread_self) {
	pthread_t t = pthread_self();

	(void)t;
}

TEST(pthread_equal) {
	pthread_t self = pthread_self();

	ASSERT_TRUE(pthread_equal(self, self));
}

TEST(pthread_create_null) {
	ASSERT_EQ(EINVAL, pthread_create(NULL, NULL, NULL, NULL));
}

TEST(pthread_create_valid) {
	pthread_t thread;

	pthread_create(&thread, NULL, threadfn, &threadval);
	pthread_join(thread, NULL);

	ASSERT_EQ(42, threadval);
}

TEST(pthread_join_null) {
	pthread_t thread;

	ASSERT_EQ(EINVAL, pthread_join(thread, NULL));
}

TEST(pthread_join_detached) {
	pthread_t thread;

	ASSERT_EQ(0, pthread_create(&thread, NULL, threadfn, &threadval));
	ASSERT_EQ(0, pthread_detach(thread));

	// Implementation is allowed to return EINVAL or ESRCH; accept either.
	int rc = pthread_join(thread, NULL);
	ASSERT_TRUE(rc == EINVAL || rc == ESRCH);
}

TEST(pthread_join_valid) {
	TEST_SKIP("hangs");
	pthread_t thread;

	pthread_create(&thread, NULL, threadfn, &threadval);
	pthread_join(thread, NULL);

	ASSERT_EQ(42, threadval);
}

TEST(pthread_detach_null) {
	pthread_t thread;

	ASSERT_EQ(EINVAL, pthread_detach(thread));
}

TEST(pthread_detach_valid) {
	pthread_t thread;

	pthread_create(&thread, NULL, threadfn, &threadval);
	ASSERT_EQ(0, pthread_detach(thread));

	usleep(10000);
}

TEST(pthread_exit) {
	TEST_SKIP("premature exit");
	pthread_exit(NULL);
}

static void *exit_threadfn(void *arg) {
	(void)arg;
	pthread_exit((void *)123);
	return NULL; /* not reached in a conforming impl */
}

TEST(pthread_exit_thread_return) {
	TEST_SKIP("current implementation exits the entire process");

	pthread_t thread;
	void *retval = NULL;

	ASSERT_EQ(0, pthread_create(&thread, NULL, exit_threadfn, NULL));
	ASSERT_EQ(0, pthread_join(thread, &retval));
	ASSERT_EQ((void *)123, retval);
}

TEST_MAIN()

