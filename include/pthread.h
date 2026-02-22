/* (c) 2025 FRINKnet & Friends – MIT licence */
#ifndef PTHREAD_H
#define PTHREAD_H
#pragma once

#include <config.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>

#if JACL_HAS_C11
	#include <stdatomic.h>
#else
	#define atomic_load(ptr) (*(ptr))
	#define atomic_store(ptr, val) (*(ptr) = (val))
	#define atomic_fetch_add(ptr, val) ((*(ptr))++)
	#define atomic_fetch_sub(ptr, val) ((*(ptr))--)
	#define atomic_compare_exchange_strong(ptr, expected, desired) \
		((*(ptr) == *(expected)) ? (*(ptr) = (desired), 1) : (*(expected) = *(ptr), 0))
	#define atomic_compare_exchange_weak atomic_compare_exchange_strong
	#warning "WARNING: Before C11 threading happense without atomics"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Platform detection and includes */
#if JACL_OS_WINDOWS
	#define PTHREAD_WIN32 1
	#include <windows.h>
	#include <process.h>
#elif JACL_HAS_PTHREADS
	#define PTHREAD_POSIX 1
	#include <unistd.h>
	#include <signal.h>
	#include <sys/wait.h>
	#include <sys/mman.h>
	#include <sys/syscall.h>
	#if JACL_OS_DARWIN
		#include <dlfcn.h>
	#endif
#else
	#define PTHREAD_DUMMY 1
#endif

#if JACL_OS_LINUX
	#include <sched.h>
	#include <sys/futex.h>

	#define __ARCH_CLONE
	#include JACL_HEADER(arch,detect)
#endif

/* ================================================================ */
/* COMMON TYPES AND CONSTANTS                                       */
/* ================================================================ */

/* Constants */
#define PTHREAD_CREATE_JOINABLE 0
#define PTHREAD_CREATE_DETACHED 1
#define PTHREAD_DESTRUCTOR_ITERATIONS 4
#define PTHREAD_MUTEX_NORMAL    0
#define PTHREAD_MUTEX_RECURSIVE 1
#define PTHREAD_MUTEX_ERRORCHECK 2
#define PTHREAD_ONCE_INIT {0}
#define MAX_TLS_KEYS 64

/* platform dependent types */
#if PTHREAD_WIN32

typedef struct {
	HANDLE handle;
	DWORD id;
	void *result;
} pthread_t;

typedef struct {
	CRITICAL_SECTION cs;
	int type;
} pthread_mutex_t;

typedef struct {
	CONDITION_VARIABLE cv;
} pthread_cond_t;

typedef DWORD pthread_key_t;

#elif PTHREAD_POSIX

struct __jacl_pthread {
	pid_t tid;
	void *stack;
	size_t stack_size;
	void *result;
	_Atomic int finished;
	_Atomic int detached;
	_Atomic int joined;
};

typedef struct __jacl_pthread *pthread_t;

typedef struct {
	_Atomic int futex;
	pid_t owner;
	int type;
	int recursive_count;
} pthread_mutex_t;

typedef struct {
	_Atomic int futex;
	_Atomic int waiters;
} pthread_cond_t;

typedef unsigned int pthread_key_t;

/* Global pthread TLS state - defined once in core/pthread.h */
extern _Atomic unsigned int __jacl_pthread_tls_key_counter;
extern thread_local void *__jacl_pthread_tls_values[MAX_TLS_KEYS];
extern pthread_key_t __jacl_pthread_self_key;
extern void (*__jacl_pthread_key_destructors[MAX_TLS_KEYS])(void *);
extern _Atomic int __jacl_pthread_inited;

#else /* PTHREAD_DUMMY */

typedef struct { int dummy; } pthread_t;
typedef struct { int dummy; } pthread_mutex_t;
typedef struct { int dummy; } pthread_cond_t;
typedef int pthread_key_t;

#endif /* platform dependent types */

/* Thread attributes */
typedef struct {
	int detached;
	size_t stack_size;
	void *stack_addr;
} pthread_attr_t;

/* Thread args */
struct posix_thread_arg {
	void *(*start_routine)(void *);
	void *arg;
	pthread_t thread_ptr;
};

/* Mutex attributes */
typedef struct {
	int type;
} pthread_mutexattr_t;

/* Condition attributes */
typedef struct {
	int dummy;
} pthread_condattr_t;

/* Once flag */
typedef struct {
	_Atomic int done;
} pthread_once_t;

/* ================================================================ */
/* WINDOWS IMPLEMENTATION                                           */
/* ================================================================ */


#if PTHREAD_WIN32

/* Thread functions */
struct win_thread_arg {
	void *(*start_routine)(void *);
	void *arg;
	pthread_t *thread_ptr;
};

static unsigned __stdcall win_thread_start(void *arg) {
	struct win_thread_arg *ta = (struct win_thread_arg *)arg;
	void *result = ta->start_routine(ta->arg);
	ta->thread_ptr->result = result;
	free(ta);
	return (unsigned)(uintptr_t)result;
}

static inline int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                                void *(*start_routine)(void *), void *arg) {
	if (!thread || !start_routine) return EINVAL;

	struct win_thread_arg *ta = malloc(sizeof(*ta));
	if (!ta) return ENOMEM;

	ta->start_routine = start_routine;
	ta->arg = arg;
	ta->thread_ptr = thread;
	thread->result = NULL;

	thread->handle = (HANDLE)_beginthreadex(NULL,
		attr ? (unsigned)attr->stack_size : 0,
		win_thread_start, ta, 0, &thread->id);

	if (!thread->handle) {
		free(ta);
		return EAGAIN;
	}
	return 0;
}

static inline int pthread_join(pthread_t thread, void **retval) {
	DWORD result = WaitForSingleObject(thread.handle, INFINITE);
	if (result == WAIT_OBJECT_0) {
		if (retval) *retval = thread.result;
		CloseHandle(thread.handle);
		return 0;
	}
	return EINVAL;
}

static inline void pthread_exit(void *retval) {
	_endthreadex((unsigned)(uintptr_t)retval);
}

static inline pthread_t pthread_self(void) {
	pthread_t self = {0};
	self.handle = GetCurrentThread();
	self.id = GetCurrentThreadId();
	return self;
}

static inline int pthread_equal(pthread_t t1, pthread_t t2) {
	return t1.id == t2.id;
}

static inline int pthread_detach(pthread_t thread) {
	return CloseHandle(thread.handle) ? 0 : EINVAL;
}

/* Mutex functions */
static inline int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr) {
	if (!mutex) return EINVAL;
	InitializeCriticalSection(&mutex->cs);
	mutex->type = (attr && attr->type) ? attr->type : PTHREAD_MUTEX_NORMAL;
	return 0;
}

static inline int pthread_mutex_lock(pthread_mutex_t *mutex) {
	if (!mutex) return EINVAL;
	EnterCriticalSection(&mutex->cs);
	return 0;
}

static inline int pthread_mutex_trylock(pthread_mutex_t *mutex) {
	if (!mutex) return EINVAL;
	return TryEnterCriticalSection(&mutex->cs) ? 0 : EBUSY;
}

static inline int pthread_mutex_timedlock(pthread_mutex_t *mutex, const struct timespec *ts) {
	if (!mutex || !ts) return EINVAL;

	DWORD timeout = (ts->tv_sec * 1000) + (ts->tv_nsec / 1000000);
	DWORD result = WaitForSingleObject(mutex->cs, timeout);

	if (result == WAIT_OBJECT_0) {
		return 0;
	} else if (result == WAIT_TIMEOUT) {
		return ETIMEDOUT;
	} else {
		return EINVAL;
	}
}

static inline int pthread_mutex_unlock(pthread_mutex_t *mutex) {
	if (!mutex) return EINVAL;
	LeaveCriticalSection(&mutex->cs);
	return 0;
}

static inline int pthread_mutex_destroy(pthread_mutex_t *mutex) {
	if (!mutex) return EINVAL;

	DeleteCriticalSection(&mutex->cs);

	return 0;
}

/* Condition variables */
static inline int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr) {
	(void)attr;
	if (!cond) return EINVAL;
	InitializeConditionVariable(&cond->cv);
	return 0;
}

static inline int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex) {
	if (!cond || !mutex) return EINVAL;
	return SleepConditionVariableCS(&cond->cv, &mutex->cs, INFINITE) ? 0 : EINVAL;
}

static inline int pthread_cond_signal(pthread_cond_t *cond) {
	if (!cond) return EINVAL;
	WakeConditionVariable(&cond->cv);
	return 0;
}

static inline int pthread_cond_broadcast(pthread_cond_t *cond) {
	if (!cond) return EINVAL;
	WakeAllConditionVariable(&cond->cv);
	return 0;
}

static inline int pthread_cond_destroy(pthread_cond_t *cond) {
	if (!cond) return EINVAL;

	return 0;
}

/* Thread-local storage */
static inline int pthread_key_create(pthread_key_t *key, void (*destructor)(void*)) {
	(void)destructor;
	if (!key) return EINVAL;
	*key = TlsAlloc();
	return (*key != TLS_OUT_OF_INDEXES) ? 0 : EAGAIN;
}

static inline int pthread_key_delete(pthread_key_t key) {
	return TlsFree(key) ? 0 : EINVAL;
}

static inline int pthread_setspecific(pthread_key_t key, const void *value) {
	return TlsSetValue(key, (void*)value) ? 0 : EINVAL;
}

static inline void *pthread_getspecific(pthread_key_t key) {
	return TlsGetValue(key);
}

#elif PTHREAD_POSIX

/* ================================================================ */
/* POSIX IMPLEMENTATION                                             */
/* ================================================================ */

#if JACL_OS_LINUX
	/* linux: direct syscalls */
	static inline pid_t __jacl_pthread_gettid(void) {
		return (pid_t)syscall(SYS_gettid);
	}

	static inline int __jacl_pthread_futex_wait(void *addr, int val) {
		return syscall(SYS_futex, addr, FUTEX_WAIT, val, NULL, NULL, 0);
	}

	static inline int __jacl_pthread_futex_wake(void *addr, int num) {
		return syscall(SYS_futex, addr, FUTEX_WAKE, num, NULL, NULL, 0);
	}

#elif JACL_OS_DARWIN
	/* Darwin: direct syscalls (no libc wrappers) */
	static inline pid_t __jacl_pthread_gettid(void) {
		return (pid_t)syscall(SYS_thread_selfid);
	}

	extern int __ulock_wait(uint32_t operation, void *addr, uint64_t value, uint32_t timeout);
	extern int __ulock_wake(uint32_t operation, void *addr);

	static inline int __jacl_pthread_futex_wait(void *addr, int val) {
		return syscall(SYS_ulock_wait, 1, addr, val, 0);
	}

	static inline int __jacl_pthread_futex_wake(void *addr, int num) {
		uint32_t op = (num == 1) ? 0x101 : 0x100;  // UL_WAKE vs UL_WAKE_ALL
		return syscall(SYS_ulock_wake, op, addr);
	}

#elif JACL_OS_FREEBSD
	/* FreeBSD: direct syscall */
	static inline pid_t __jacl_pthread_gettid(void) {
		long tid;
		syscall(SYS_thr_self, &tid);
		return (pid_t)tid;
	}

	static inline int __jacl_pthread_futex_wait(void *addr, int val) {
		return syscall(SYS__umtx_op, addr, 3 /* UMTX_OP_WAIT */, val, NULL, NULL);
	}

	static inline int __jacl_pthread_futex_wake(void *addr, int num) {
		return syscall(SYS__umtx_op, addr, 4 /* UMTX_OP_WAKE */, num, NULL, NULL);
	}

#else
	/* Unknown OS: use getpid() and no-ops for now */
	static inline pid_t __jacl_pthread_gettid(void) { return getpid(); }
	static inline int __jacl_pthread_futex_wait(void *addr, int val) { return -1; }
	static inline int __jacl_pthread_futex_wake(void *addr, int num) { return -1; }
#endif

static inline void __jacl_pthread_init(void) {
	int expected = 0;

	if (atomic_compare_exchange_strong(&__jacl_pthread_inited, &expected, 1)) {
		/* Create TLS key for pthread_self storage */
		__jacl_pthread_self_key = atomic_fetch_add(&__jacl_pthread_tls_key_counter, 1);
		if (__jacl_pthread_self_key >= MAX_TLS_KEYS) return;

		/* Create main thread's pthread_t handle */
		pthread_t t = calloc(1, sizeof *t);
		if (!t) return;

		t->tid        = __jacl_pthread_gettid();
		t->stack      = NULL;
		t->stack_size = 0;
		t->result     = NULL;
		atomic_store(&t->finished, 0);
		atomic_store(&t->detached, 0);
		atomic_store(&t->joined, 0);

		/* Store directly in TLS array */
		__jacl_pthread_tls_values[__jacl_pthread_self_key] = t;
	}
}

static inline void __jacl_pthread_destroy(void) {
	int iterations = PTHREAD_DESTRUCTOR_ITERATIONS;
	int had_values;

	do {
		had_values = 0;

		for (unsigned int i = 0; i < atomic_load(&__jacl_pthread_tls_key_counter) && i < MAX_TLS_KEYS; i++) {
			void *value = __jacl_pthread_tls_values[i];
			void (*destructor)(void *) = __jacl_pthread_key_destructors[i];

			if (value && destructor) {
				__jacl_pthread_tls_values[i] = NULL;
				destructor(value);

				had_values = 1;
			}
		}
	} while (had_values && --iterations > 0);
}

static inline void __jacl_pthread_set_self(pthread_t thread_ptr) {
	if (__jacl_pthread_self_key < MAX_TLS_KEYS) __jacl_pthread_tls_values[__jacl_pthread_self_key] = thread_ptr;
}

static inline int __jacl_pthread_entry(void *arg) {
	struct posix_thread_arg *ta = (struct posix_thread_arg *)arg;

	__jacl_pthread_set_self(ta->thread_ptr);

	void *result = ta->start_routine(ta->arg);

	ta->thread_ptr->result = result;

	atomic_store(&ta->thread_ptr->finished, 1);

	__jacl_pthread_destroy();

	free(ta);
#if JACL_OS_DARWIN
	return 0;  /* system pthread handles thread cleanup */
#else
	_exit(0);
#endif
}

static inline pid_t __jacl_pthread_clone_thread(void *stack, size_t stack_size, int (*fn)(void *), void *arg) {
	#if JACL_OS_LINUX
		return __jacl_arch_clone_thread(stack, stack_size, fn, arg);
	#elif JACL_OS_DARWIN
		(void)stack;
		(void)stack_size;

		/* Use system libpthread (always available on macOS via libSystem) */
		typedef int (*sys_pthread_create_t)(void *, const void *, void *(*)(void *), void *);
		static sys_pthread_create_t sys_create = NULL;
		if (!sys_create) {
			sys_create = (sys_pthread_create_t)dlsym(RTLD_DEFAULT, "pthread_create");
			if (!sys_create) return -1;
		}

		unsigned long sys_thread;
		/* Cast fn: int(*)(void*) -> void*(*)(void*) — return value unused */
		int ret = sys_create((void *)&sys_thread, NULL, (void *(*)(void *))fn, arg);
		if (ret != 0) return -1;

		return (pid_t)syscall(SYS_thread_selfid);
	#else
		(void)stack;
		(void)stack_size;

		pid_t pid = fork();

		if (pid == 0) {
			fn(arg);
			_exit(0);
		}

		return pid;
	#endif
}

/* Thread functions */
static inline int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void *), void *arg) {
	if (!thread || !start_routine) return EINVAL;

	__jacl_pthread_init();

	pthread_t t = calloc(1, sizeof *t);

	if (!t) return ENOMEM;

	struct posix_thread_arg *ta = malloc(sizeof *ta);

	if (!ta) { free(t); return ENOMEM; }

	size_t stack_size = (attr && attr->stack_size) ? attr->stack_size : 1024 * 1024;
	void *stack = NULL;

	#if JACL_OS_LINUX
		stack = mmap(NULL, stack_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
		if (stack == MAP_FAILED) stack = NULL;
	#elif JACL_OS_DARWIN
		stack = mmap(NULL, stack_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		if (stack == MAP_FAILED) stack = NULL;
	#endif

	if (!stack) {
		stack = malloc(stack_size);

		if (!stack) {
			free(ta);
			free(t);

			return ENOMEM;
		}
	}

	ta->start_routine = start_routine;
	ta->arg           = arg;
	ta->thread_ptr    = t;

	t->stack      = stack;
	t->stack_size = stack_size;
	t->result     = NULL;

	atomic_store(&t->finished, 0);
	atomic_store(&t->detached, (attr && attr->detached) ? 1 : 0);
	atomic_store(&t->joined,   0);

	pid_t tid = __jacl_pthread_clone_thread(stack, stack_size, __jacl_pthread_entry, ta);

	if (tid > 0) {
		t->tid = tid;
		*thread = t;
		return 0;
	} else {
		#if JACL_OS_LINUX || JACL_OS_DARWIN
			if (stack) munmap(stack, stack_size);
		#else
			free(stack);
		#endif

		free(ta);
		free(t);

		return EAGAIN;
	}
}

static inline int pthread_join(pthread_t thread, void **retval) {
	if (!thread) return EINVAL;
	if (atomic_load(&thread->detached)) return EINVAL;

	__jacl_pthread_init();

	int expected = 0;
	struct timespec ts = {0, 1000000}; /* 1ms */

	if (!atomic_compare_exchange_strong(&thread->joined, &expected, 1)) return EINVAL;
	while (!atomic_load(&thread->finished)) nanosleep(&ts, NULL);
	if (retval) *retval = thread->result;

	#if JACL_OS_LINUX || JACL_OS_DARWIN
		if (thread->stack && thread->stack_size && munmap(thread->stack, thread->stack_size) != 0) free(thread->stack);
	#else
		free(thread->stack);
	#endif

	free(thread);

	return 0;
}

static inline pthread_t pthread_self(void) {
	__jacl_pthread_init();

	if (__jacl_pthread_self_key < MAX_TLS_KEYS) return (pthread_t)__jacl_pthread_tls_values[__jacl_pthread_self_key];

	return NULL;
}

static inline noreturn void pthread_exit(void *retval) {
	pthread_t self = pthread_self();

	if (self) {
		self->result = retval;

		atomic_store(&self->finished, 1);
	}

	__jacl_pthread_destroy();

	#if JACL_OS_LINUX
		syscall(SYS_exit, 0);
	#elif JACL_OS_DARWIN
		syscall(SYS_bsdthread_terminate, 0, 0, 0, 0, 0);
	#elif JACL_OS_FREEBSD
		syscall(SYS_thr_exit, NULL);
	#else
		_exit(0);
	#endif
}

static inline int pthread_equal(pthread_t t1, pthread_t t2) {
	return t1 == t2;
}

static inline int pthread_detach(pthread_t thread) {
	if (!thread) return EINVAL;

	__jacl_pthread_init();
	int expected = 0;

	if (!atomic_compare_exchange_strong(&thread->detached, &expected, 1)) return EINVAL; /* already detached */

	if (atomic_load(&thread->finished)) {
		#if JACL_OS_LINUX || JACL_OS_DARWIN
			if (thread->stack && thread->stack_size && munmap(thread->stack, thread->stack_size) != 0) free(thread->stack);
		#else
			free(thread->stack);
		#endif
		free(thread);
	}

	return 0;
}

/* Mutex functions */
static inline int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr) {
	if (!mutex) return EINVAL;

	atomic_store(&mutex->futex, 0);

	mutex->owner = 0;
	mutex->type = (attr && attr->type) ? attr->type : PTHREAD_MUTEX_NORMAL;
	mutex->recursive_count = 0;

	return 0;
}

static inline int pthread_mutex_lock(pthread_mutex_t *mutex) {
	if (!mutex) return EINVAL;

	if (mutex->type == PTHREAD_MUTEX_RECURSIVE) {
		pid_t current_tid = __jacl_pthread_gettid();

		if (mutex->owner == current_tid) {
			mutex->recursive_count++;

			return 0;
		}
	}

	int expected = 0;
	int backoff = 1000;

	while (!atomic_compare_exchange_weak(&mutex->futex, &expected, 1)) {
		expected = 0;

		if (__jacl_pthread_futex_wait(&mutex->futex, 1) != 0) {
			struct timespec ts = {0, backoff};

			nanosleep(&ts, NULL);

			if (backoff < 1000000) backoff *= 2;
		}
	}

	if (mutex->type == PTHREAD_MUTEX_RECURSIVE) {
		mutex->owner = __jacl_pthread_gettid();
		mutex->recursive_count = 1;
	}

	return 0;
}

static inline int pthread_mutex_trylock(pthread_mutex_t *mutex) {
	if (!mutex) return EINVAL;

	int expected = 0;

	if (atomic_compare_exchange_strong(&mutex->futex, &expected, 1)) {
		if (mutex->type == PTHREAD_MUTEX_RECURSIVE) {
			mutex->owner = __jacl_pthread_gettid();
			mutex->recursive_count = 1;
		}

		return 0;
	}

	return EBUSY;
}

static inline int pthread_mutex_timedlock(pthread_mutex_t *mutex, const struct timespec *ts) {
	if (!mutex || !ts) return EINVAL;

	if (mutex->type == PTHREAD_MUTEX_RECURSIVE) {
		pid_t current_tid = __jacl_pthread_gettid();
		if (mutex->owner == current_tid) {
			mutex->recursive_count++;
			return 0;
		}
	}

	// Calculate absolute timeout
	struct timespec end;
	clock_gettime(CLOCK_REALTIME, &end);
	end.tv_sec += ts->tv_sec;
	end.tv_nsec += ts->tv_nsec;
	if (end.tv_nsec >= 1000000000) {
		end.tv_sec++;
		end.tv_nsec -= 1000000000;
	}

	int expected = 0;
	int backoff = 1000;

	while (!atomic_compare_exchange_weak(&mutex->futex, &expected, 1)) {
		expected = 0;

		// Check for timeout
		struct timespec now;
		clock_gettime(CLOCK_REALTIME, &now);
		if (now.tv_sec > end.tv_sec || (now.tv_sec == end.tv_sec && now.tv_nsec >= end.tv_nsec)) {
			return ETIMEDOUT;
		}

		// Wait a bit before retrying
		struct timespec ts_wait = {0, backoff};
		nanosleep(&ts_wait, NULL);
		if (backoff < 1000000) backoff *= 2;
	}

	if (mutex->type == PTHREAD_MUTEX_RECURSIVE) {
		mutex->owner = __jacl_pthread_gettid();
		mutex->recursive_count = 1;
	}

	return 0;
}

static inline int pthread_mutex_unlock(pthread_mutex_t *mutex) {
	if (!mutex) return EINVAL;

	if (mutex->type == PTHREAD_MUTEX_RECURSIVE) {
		if (mutex->recursive_count > 1) {
			mutex->recursive_count--;

			return 0;
		}

		mutex->owner = 0;
		mutex->recursive_count = 0;
	}

	atomic_store(&mutex->futex, 0);
	__jacl_pthread_futex_wake(&mutex->futex, 1);

	return 0;
}

static inline int pthread_mutex_destroy(pthread_mutex_t *mutex) {
	if (!mutex) return EINVAL;

	return 0;
}

/* Condition variables */
static inline int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr) {
	(void)attr;

	if (!cond) return EINVAL;

	atomic_store(&cond->futex, 0);
	atomic_store(&cond->waiters, 0);

	return 0;
}

static inline int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex) {
	if (!cond || !mutex) return EINVAL;

	atomic_fetch_add(&cond->waiters, 1);

	int old_futex = atomic_load(&cond->futex);

	pthread_mutex_unlock(mutex);

	if (__jacl_pthread_futex_wait(&cond->futex, old_futex) != 0) {
		while (atomic_load(&cond->futex) == old_futex) {
			struct timespec ts = {0, 1000000}; /* 1ms */

			nanosleep(&ts, NULL);
		}
	}

	pthread_mutex_lock(mutex);
	atomic_fetch_sub(&cond->waiters, 1);

	return 0;
}

static inline int pthread_cond_signal(pthread_cond_t *cond) {
	if (!cond) return EINVAL;
	if (atomic_load(&cond->waiters) > 0) {
		atomic_fetch_add(&cond->futex, 1);
		__jacl_pthread_futex_wake(&cond->futex, 1);
	}

	return 0;
}

static inline int pthread_cond_broadcast(pthread_cond_t *cond) {
	if (!cond) return EINVAL;

	int waiters = atomic_load(&cond->waiters);

	if (waiters > 0) {
		atomic_fetch_add(&cond->futex, 1);
		__jacl_pthread_futex_wake(&cond->futex, waiters);
	}

	return 0;
}

static inline int pthread_cond_destroy(pthread_cond_t *cond) {
	if (!cond) return EINVAL;

	return 0;
}

/* Thread-local storage */
static inline int pthread_key_create(pthread_key_t *key, void (*destructor)(void*)) {
	if (!key) return EINVAL;

	__jacl_pthread_init();

	unsigned int k = atomic_fetch_add(&__jacl_pthread_tls_key_counter, 1);

	if (k >= MAX_TLS_KEYS) return EAGAIN;

	*key = k;

	__jacl_pthread_key_destructors[k] = destructor;

	return 0;
}

static inline int pthread_key_delete(pthread_key_t key) {
	if (key >= MAX_TLS_KEYS) return EINVAL;

	__jacl_pthread_key_destructors[key] = NULL;

	return 0;
}

static inline int pthread_setspecific(pthread_key_t key, const void *value) {
	if (key >= MAX_TLS_KEYS) return EINVAL;

	__jacl_pthread_tls_values[key] = (void *)value;

	return 0;
}

static inline void *pthread_getspecific(pthread_key_t key) {
	if (key >= MAX_TLS_KEYS) return NULL;

	return __jacl_pthread_tls_values[key];
}

static inline int pthread_sigmask(int how, const sigset_t *set, sigset_t *oldset) {
	#if JACL_HAS_POSIX
		return sigprocmask(how, set, oldset);
	#else
		(void)how; (void)set; (void)oldset;
		return ENOSYS;
	#endif
}

#else /* PTHREAD_DUMMY */

/* ================================================================ */
/* DUMMY IMPLEMENTATION (Single-threaded stubs)                     */
/* ================================================================ */

/* Thread functions - all return ENOSYS */
#define pthread_create(thread, attr, start_routine, arg) (ENOSYS)
#define pthread_join(thread, retval) (ENOSYS)
#define pthread_detach(thread) (ENOSYS)
#define pthread_exit(retval) do { exit(0); } while(0)
#define pthread_self() ((pthread_t){0})
#define pthread_equal(t1, t2) (1)

/* Mutex functions - no-ops (no contention in single-threaded) */
#define pthread_mutex_init(mutex, attr) (ENOSYS)
#define pthread_mutex_lock(mutex) (ENOSYS)
#define pthread_mutex_trylock(mutex) (ENOSYS)
#define pthread_mutex_timedlock(mutex, ts) (ENOSYS)
#define pthread_mutex_unlock(mutex) (ENOSYS)
#define pthread_mutex_destroy(mutex) (ENOSYS)

/* Condition variables - return ENOSYS */
#define pthread_cond_init(cond, attr) (ENOSYS)
#define pthread_cond_wait(cond, mutex) (ENOSYS)
#define pthread_cond_signal(cond) (ENOSYS)
#define pthread_cond_broadcast(cond) (ENOSYS)
#define pthread_cond_destroy(cond) (ENOSYS)

#define pthread_once(once_control, init_routine) (ENOSYS)

/* Thread-local storage - return ENOSYS */
#define pthread_attr_init(attr) (ENOSYS)
#define pthread_attr_destroy(attr) (ENOSYS)
#define pthread_attr_setdetachstate(attr, state) (ENOSYS)
#define pthread_attr_setstacksize(attr, size) (ENOSYS)

#define pthread_mutexattr_init(attr) (ENOSYS)
#define pthread_mutexattr_destroy(attr) (ENOSYS)
#define pthread_mutexattr_settype(attr, type) (ENOSYS)
#define pthread_condattr_init(attr) (ENOSYS)
#define pthread_condattr_destroy(attr) (ENOSYS)

#define pthread_key_create(key, destructor) (ENOSYS)
#define pthread_key_delete(key) (ENOSYS)

#define pthread_setspecific(key, value) (ENOSYS)
#define pthread_getspecific(key) (NULL)

#define pthread_sigmask(how, set, oldset) (ENOSYS)

#endif /* Platform implementations */

/* ================================================================ */
/* COMMON FUNCTIONS - Same across all platforms                    */
/* ================================================================ */

static inline int pthread_once(pthread_once_t *once_control, void (*init_routine)(void)) {
	if (!once_control || !init_routine) return EINVAL;
	int expected = 0;
	if (atomic_compare_exchange_strong(&once_control->done, &expected, 1)) {
		init_routine();
	}
	return 0;
}

/* Attribute functions */
static inline int pthread_attr_init(pthread_attr_t *attr) {
	if (!attr) return EINVAL;
	attr->detached = 0;
	attr->stack_size = 0;
	attr->stack_addr = NULL;
	return 0;
}

static inline int pthread_attr_destroy(pthread_attr_t *attr) {
	if (!attr) return EINVAL;

	return 0;
}

static inline int pthread_attr_setdetachstate(pthread_attr_t *attr, int detachstate) {
	if (!attr) return EINVAL;
	attr->detached = (detachstate == PTHREAD_CREATE_DETACHED);
	return 0;
}

static inline int pthread_attr_setstacksize(pthread_attr_t *attr, size_t stacksize) {
	if (!attr) return EINVAL;
	attr->stack_size = stacksize;
	return 0;
}

static inline int pthread_mutexattr_init(pthread_mutexattr_t *attr) {
	if (!attr) return EINVAL;
	attr->type = PTHREAD_MUTEX_NORMAL;
	return 0;
}

static inline int pthread_mutexattr_destroy(pthread_mutexattr_t *attr) {
	if (!attr) return EINVAL;

	return 0;
}

static inline int pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type) {
	if (!attr) return EINVAL;
	attr->type = type;
	return 0;
}

static inline int pthread_condattr_init(pthread_condattr_t *attr) {
	if (!attr) return EINVAL;
	attr->dummy = 0;
	return 0;
}

static inline int pthread_condattr_destroy(pthread_condattr_t *attr) {
	if (!attr) return EINVAL;

	return 0;
}

#ifdef __cplusplus
}
#endif

#endif /* PTHREAD_H */
