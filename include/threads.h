/* (c) 2025 FRINKnet & Friends – MIT licence */
#ifndef THREADS_H
#define THREADS_H
#pragma once

#include <config.h>
#include <stdbool.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/* C23 feature test macro */
#if JACL_HAS_C23
#  define __STDC_VERSION_THREADS_H__ 202311L
#endif

/* C11/C23 thread result codes */
enum {
		thrd_success	= 0,
		thrd_nomem		= 1,
		thrd_timedout = 2,
		thrd_busy			= 3,
		thrd_error		= 4
};

/* Mutex types */
enum {
		mtx_plain			= 0,
		mtx_recursive = 1,
		mtx_timed			= 2
};

/* Thread function signature */
typedef int (*thrd_start_t)(void*);
typedef void (*tss_dtor_t)(void*);

/* TSS destructor iterations */
#ifndef TSS_DTOR_ITERATIONS
#define TSS_DTOR_ITERATIONS 4
#endif

#if !JACL_HAS_PTHREADS
typedef int thrd_t, mtx_t, cnd_t, tss_t;
typedef struct { int done; } once_flag;

#define ONCE_FLAG_INIT {0}

#define thrd_create(t,f,a)				(thrd_error)
#define thrd_exit(r)							__builtin_trap()
#define thrd_join(t,r)						(thrd_error)
#define thrd_detach(t)						(thrd_error)
#define thrd_current()						(0)
#define thrd_equal(a,b)						((a) == (b))
#define thrd_yield()							((void)0)
#define thrd_sleep(d,r)						(-1)
#define mtx_init(m,t)							(thrd_error)
#define mtx_lock(m)								(thrd_error)
#define mtx_trylock(m)						(thrd_busy)
#define mtx_timedlock(m,ts)				(thrd_error)
#define mtx_unlock(m)							(thrd_error)
#define mtx_destroy(m)						((void)0)
#define cnd_init(c)								(thrd_error)
#define cnd_signal(c)							(thrd_error)
#define cnd_broadcast(c)					(thrd_error)
#define cnd_wait(c,m)							(thrd_error)
#define cnd_timedwait(c,m,ts)			(thrd_error)
#define cnd_destroy(c)						((void)0)
#define call_once(f,fn)						((void)0)
#define tss_create(k,d)						(thrd_error)
#define tss_delete(k)							((void)0)
#define tss_set(k,v)							(thrd_error)
#define tss_get(k)								(NULL)

#elif JACL_ARCH_WASM
/* Basic atomic synchronization only */
#include <stdatomic.h>

typedef int thrd_t;  /* Thread IDs managed by host */
typedef struct { _Atomic int locked; int type; } mtx_t;
typedef struct { _Atomic int value; } cnd_t;
typedef int tss_t;
typedef struct { _Atomic int done; } once_flag;

#define ONCE_FLAG_INIT {ATOMIC_VAR_INIT(0)}

/* Thread management - NOT supported in raw WASM */
static inline int thrd_create(thrd_t *thr, thrd_start_t func, void *arg) {
		(void)thr; (void)func; (void)arg;
		return thrd_error;
}
static inline _Noreturn void thrd_exit(int res) {
		(void)res;
		__builtin_trap();
}
static inline int thrd_join(thrd_t thr, int *res) {
		(void)thr; (void)res;
		return thrd_error;
}
static inline int thrd_detach(thrd_t thr) {
		(void)thr;
		return thrd_error;
}
static inline thrd_t thrd_current(void) {
		return 0; /* Host must provide */
}
static inline int thrd_equal(thrd_t lhs, thrd_t rhs) {
		return lhs == rhs;
}
static inline void thrd_yield(void) {
		/* no-op */
}
static inline int thrd_sleep(const struct timespec *duration, struct timespec *remaining) {
		(void)duration; (void)remaining;
		return -1; /* not supported */
}

/* Mutex - atomic-based implementation */
static inline int mtx_init(mtx_t *mtx, int type) {
		if (!mtx) return thrd_error;
		atomic_store(&mtx->locked, 0);
		mtx->type = type;
		return thrd_success;
}

static inline int mtx_lock(mtx_t *mtx) {
		if (!mtx) return thrd_error;
		while (atomic_exchange(&mtx->locked, 1)) {
				/* Spin */
		}
		return thrd_success;
}

static inline int mtx_trylock(mtx_t *mtx) {
		if (!mtx) return thrd_error;
		return atomic_exchange(&mtx->locked, 1) == 0 ? thrd_success : thrd_busy;
}

static inline int mtx_timedlock(mtx_t *mtx, const struct timespec *ts) {
		(void)ts;
		return mtx_trylock(mtx);
}

static inline int mtx_unlock(mtx_t *mtx) {
		if (!mtx) return thrd_error;
		atomic_store(&mtx->locked, 0);
		return thrd_success;
}

static inline void mtx_destroy(mtx_t *mtx) {
		if (mtx) atomic_store(&mtx->locked, 0);
}

/* Condition variables - very limited */
static inline int cnd_init(cnd_t *cnd) {
		if (!cnd) return thrd_error;
		atomic_store(&cnd->value, 0);
		return thrd_success;
}

static inline int cnd_signal(cnd_t *cnd) {
		if (!cnd) return thrd_error;
		atomic_fetch_add(&cnd->value, 1);
		return thrd_success;
}

static inline int cnd_broadcast(cnd_t *cnd) {
		if (!cnd) return thrd_error;
		atomic_fetch_add(&cnd->value, 1000000);
		return thrd_success;
}

static inline int cnd_wait(cnd_t *cnd, mtx_t *mtx) {
		if (!cnd || !mtx) return thrd_error;
		int old_value = atomic_load(&cnd->value);
		mtx_unlock(mtx);
		while (atomic_load(&cnd->value) == old_value) {
				/* Busy wait */
		}
		mtx_lock(mtx);
		return thrd_success;
}

static inline int cnd_timedwait(cnd_t *cnd, mtx_t *mtx, const struct timespec *ts) {
		(void)ts;
		return cnd_wait(cnd, mtx);
}

static inline void cnd_destroy(cnd_t *cnd) {
		if (cnd) atomic_store(&cnd->value, 0);
}

/* One-time initialization */
static inline void call_once(once_flag *flag, void (*func)(void)) {
		if (!flag || !func) return;
		if (atomic_exchange(&flag->done, 1) == 0) {
				func();
		}
}

/* Thread-local storage - not supported */
static inline int tss_create(tss_t *key, tss_dtor_t dtor) {
		(void)key; (void)dtor;
		return thrd_error;
}
static inline void tss_delete(tss_t key) {
		(void)key;
}
static inline int tss_set(tss_t key, void *val) {
		(void)key; (void)val;
		return thrd_error;
}
static inline void *tss_get(tss_t key) {
		(void)key;
		return NULL;
}

#else
/* ================================================================ */
/* Native Platforms - Full pthread implementation									 */
/* ================================================================ */

#include <pthread.h>
#include <errno.h>
#include <stdlib.h>
#include <sched.h>
#include <unistd.h>

typedef pthread_t thrd_t;
typedef pthread_mutex_t mtx_t;
typedef pthread_cond_t cnd_t;
typedef pthread_key_t tss_t;
typedef pthread_once_t once_flag;

#define ONCE_FLAG_INIT PTHREAD_ONCE_INIT

struct __thrd_start_arg {
		thrd_start_t func;
		void *arg;
};

static void *__thrd_start_func(void *arg) {
		struct __thrd_start_arg *start_arg = arg;
		int result = start_arg->func(start_arg->arg);
		free(start_arg);
		return (void*)(intptr_t)result;
}

/* Thread functions */
static inline int thrd_create(thrd_t *thr, thrd_start_t func, void *arg) {
		if (!thr || !func) return thrd_error;
		struct __thrd_start_arg *start_arg = malloc(sizeof(*start_arg));
		if (!start_arg) return thrd_nomem;

		start_arg->func = func;
		start_arg->arg = arg;

		int result = pthread_create(thr, NULL, __thrd_start_func, start_arg);
		if (result != 0) {
				free(start_arg);
				return result == ENOMEM ? thrd_nomem : thrd_error;
		}
		return thrd_success;
}

static inline _Noreturn void thrd_exit(int res) {
		pthread_exit((void*)(intptr_t)res);
}

static inline int thrd_join(thrd_t thr, int *res) {
		void *pthread_res;
		int result = pthread_join(thr, &pthread_res);
		if (result == 0 && res) *res = (int)(intptr_t)pthread_res;
		return result == 0 ? thrd_success : thrd_error;
}

static inline int thrd_detach(thrd_t thr) {
		return pthread_detach(thr) == 0 ? thrd_success : thrd_error;
}

static inline thrd_t thrd_current(void) {
		return pthread_self();
}

static inline int thrd_equal(thrd_t lhs, thrd_t rhs) {
		return pthread_equal(lhs, rhs);
}

static inline void thrd_yield(void) {
		sched_yield();
}

static inline int thrd_sleep(const struct timespec *duration, struct timespec *remaining) {
		return nanosleep(duration, remaining);
}

/* Mutex functions */
static inline int mtx_init(mtx_t *mtx, int type) {
		if (!mtx) return thrd_error;

		pthread_mutexattr_t attr;
		if (pthread_mutexattr_init(&attr) != 0) return thrd_error;

		if (type & mtx_recursive) {
				if (pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE) != 0) {
						pthread_mutexattr_destroy(&attr);
						return thrd_error;
				}
		}

		int result = pthread_mutex_init(mtx, &attr);
		pthread_mutexattr_destroy(&attr);
		return result == 0 ? thrd_success : thrd_error;
}

static inline int mtx_lock(mtx_t *mtx) {
		if (!mtx) return thrd_error;
		return pthread_mutex_lock(mtx) == 0 ? thrd_success : thrd_error;
}

static inline int mtx_trylock(mtx_t *mtx) {
		if (!mtx) return thrd_error;
		int result = pthread_mutex_trylock(mtx);
		if (result == 0) return thrd_success;
		if (result == EBUSY) return thrd_busy;
		return thrd_error;
}

static inline int mtx_timedlock(mtx_t *mtx, const struct timespec *ts) {
		if (!mtx || !ts) return thrd_error;
		int result = pthread_mutex_timedlock(mtx, ts);
		if (result == 0) return thrd_success;
		if (result == ETIMEDOUT) return thrd_timedout;
		return thrd_error;
}

static inline int mtx_unlock(mtx_t *mtx) {
		if (!mtx) return thrd_error;
		return pthread_mutex_unlock(mtx) == 0 ? thrd_success : thrd_error;
}

static inline void mtx_destroy(mtx_t *mtx) {
		if (mtx) pthread_mutex_destroy(mtx);
}

/* Condition variables */
static inline int cnd_init(cnd_t *cnd) {
		if (!cnd) return thrd_error;
		return pthread_cond_init(cnd, NULL) == 0 ? thrd_success : thrd_error;
}

static inline int cnd_signal(cnd_t *cnd) {
		if (!cnd) return thrd_error;
		return pthread_cond_signal(cnd) == 0 ? thrd_success : thrd_error;
}

static inline int cnd_broadcast(cnd_t *cnd) {
		if (!cnd) return thrd_error;
		return pthread_cond_broadcast(cnd) == 0 ? thrd_success : thrd_error;
}

static inline int cnd_wait(cnd_t *cnd, mtx_t *mtx) {
		if (!cnd || !mtx) return thrd_error;
		return pthread_cond_wait(cnd, mtx) == 0 ? thrd_success : thrd_error;
}

static inline int cnd_timedwait(cnd_t *cnd, mtx_t *mtx, const struct timespec *ts) {
		if (!cnd || !mtx || !ts) return thrd_error;
		int result = pthread_cond_timedwait(cnd, mtx, ts);
		if (result == 0) return thrd_success;
		if (result == ETIMEDOUT) return thrd_timedout;
		return thrd_error;
}

static inline void cnd_destroy(cnd_t *cnd) {
		if (cnd) pthread_cond_destroy(cnd);
}

/* One-time initialization */
static inline void call_once(once_flag *flag, void (*func)(void)) {
		if (!flag || !func) return;
		pthread_once(flag, func);
}

/* Thread-local storage */
static inline int tss_create(tss_t *key, tss_dtor_t dtor) {
		if (!key) return thrd_error;
		return pthread_key_create(key, dtor) == 0 ? thrd_success : thrd_error;
}

static inline void tss_delete(tss_t key) {
		pthread_key_delete(key);
}

static inline int tss_set(tss_t key, void *val) {
		return pthread_setspecific(key, val) == 0 ? thrd_success : thrd_error;
}

static inline void *tss_get(tss_t key) {
		return pthread_getspecific(key);
}

#endif /* !JACL_HAS_THREADS */

#ifdef __cplusplus
}
#endif
#endif /* THREADS_H */
