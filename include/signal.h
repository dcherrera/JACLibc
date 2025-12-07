/* (c) 2025 FRINKnet & Friends – MIT licence */
#ifndef _SIGNAL_H
#define _SIGNAL_H
#pragma once

#include <config.h>
#include <errno.h>
#include <stddef.h>
#include <time.h>
#include <sys/types.h>
#include <sys/syscall.h>

/* C23 version identification (C23 §7.14) */
#if JACL_HAS_C23
#define __STDC_VERSION_SIGNAL_H__ 202311L
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* — Core ISO C Types (C99+) — */
typedef int sig_atomic_t;
typedef void (*__sighandler_t)(int);

/* Implementation-defined signal handler type name */
#if defined(__GNUC__) || defined(__clang__) || defined(_GNU_SOURCE)
typedef __sighandler_t sig_t;	/* BSD/GNU extension */
#else
typedef void (*sig_t)(int);		/* Generic fallback */
#endif

/* — ISO C Signal Handler Constants (C99+) — */
#define SIG_DFL  ((__sighandler_t) 0)
#define SIG_IGN  ((__sighandler_t) 1)
#define SIG_ERR  ((__sighandler_t) -1)

#include JACL_X_SIGNALS
#define X_ENUM(name, val) name = (val),
enum { SIGNAL_NUMBERS(X_ENUM) };
enum { SIGNAL_ACTION_FLAGS(X_ENUM) };
enum { SIGNAL_MASK_OPS(X_ENUM) };
enum { SIGNAL_INFO_CODES(X_ENUM) };
enum { SIGNAL_ILL_CODES(X_ENUM) };
enum { SIGNAL_FPE_CODES(X_ENUM) };
enum { SIGNAL_SEGV_CODES(X_ENUM) };
enum { SIGNAL_BUS_CODES(X_ENUM) };
enum { SIGNAL_TRAP_CODES(X_ENUM) };
enum { SIGNAL_CLD_CODES(X_ENUM) };
enum { SIGNAL_POLL_CODES(X_ENUM) };
enum { SIGNAL_EVENT_TYPES(X_ENUM) };
#undef X_ENUM

#if JACL_OS_LINUX
	/* Modern arches that DON'T need it */
	#if !(JACL_ARCH_RISCV32 || JACL_ARCH_RISCV64 || JACL_ARCH_LOONGARCH64)
		#define JACL_RESTORER 0x04000000
	#endif
#endif

/* — POSIX Extensions (when feature test macros are defined) — */
#if JACL_HAS_POSIX

/* Portable syscall name mapping */
#if JACL_OS_LINUX
	#define JACL_SYS_SIGACTION    SYS_rt_sigaction
	#define JACL_SYS_SIGPROCMASK  SYS_rt_sigprocmask
	#define JACL_SYS_SIGPENDING   SYS_rt_sigpending
	#define JACL_SYS_SIGSUSPEND   SYS_rt_sigsuspend
	#define JACL_SYS_SIGTIMEDWAIT SYS_rt_sigtimedwait
	#define JACL_SYS_SIGQUEUEINFO SYS_rt_sigqueueinfo
#else
    /* BSD/Darwin: use classic syscalls */
    #define JACL_SYS_SIGACTION    SYS_sigaction
    #define JACL_SYS_SIGPROCMASK  SYS_sigprocmask
    #define JACL_SYS_SIGPENDING   SYS_sigpending
    #define JACL_SYS_SIGSUSPEND   SYS_sigsuspend

    /* Advanced functions - only on some BSDs */
    #if JACL_OS_FREEBSD || JACL_OS_DRAGONFLY
        #define JACL_SYS_SIGTIMEDWAIT SYS_sigtimedwait
        #define JACL_SYS_SIGWAITINFO  SYS_sigwaitinfo
    #endif

    #if JACL_OS_FREEBSD
        #define JACL_SYS_SIGQUEUE SYS_sigqueue
    #elif JACL_OS_NETBSD
        #define JACL_SYS_SIGQUEUE SYS_sigqueueinfo
    #endif
#endif

typedef struct { unsigned long __bits[2]; } sigset_t;

#define JACL_SIGSET_MAX 128  /* 2 * 64-bit words */

/* — Signal Value Union — */
union sigval {
	int   sival_int;
	void *sival_ptr;
};

/* — Signal Information Structure — */
typedef struct {
	int          si_signo;
	int          si_errno;
	int          si_code;
	pid_t        si_pid;
	uid_t        si_uid;
	void        *si_addr;
	int          si_status;
	long         si_band;
	union sigval si_value;
	int          si_int;
	void        *si_ptr;
	void        *si_addr_lsb;
	short        si_addr_bnd[2];
} siginfo_t;

/* — Signal Action Structure — */
#if JACL_OS_LINUX
	#if JACL_ARCH_X64 || JACL_ARCH_ARM64
		/* Linux x64/arm64: sa_flags before sa_mask, no sa_sigaction union */
		struct sigaction {
			void      (*sa_handler)(int);
			unsigned long sa_flags;
			void      (*sa_restorer)(void);
			sigset_t   sa_mask;  /* LAST for kernel extensibility */
		};
	#else
		/* Linux other arches: union + different order */
		struct sigaction {
			union {
				void (*_sa_handler)(int);
				void (*_sa_sigaction)(int, siginfo_t *, void *);
			} _u;
			sigset_t   sa_mask;
			unsigned long sa_flags;
			void      (*sa_restorer)(void);
		};
		#define sa_handler _u._sa_handler
		#define sa_sigaction _u._sa_sigaction
	#endif
#else
	/* POSIX/BSD layout */
	struct sigaction {
		void      (*sa_handler)(int);
		void      (*sa_sigaction)(int, siginfo_t *, void *);
		sigset_t   sa_mask;
		int        sa_flags;
		void      (*sa_restorer)(void);
	};
#endif


/* — Signal Event Structure — */
struct sigevent {
	int          sigev_notify;
	int          sigev_signo;
	union sigval sigev_value;
	void       (*sigev_notify_function)(union sigval);
	void        *sigev_notify_attributes;
};

static inline int kill(pid_t pid, int sig) {
	return (int)syscall(SYS_kill, (long)pid, (long)sig);
}

static inline int sigemptyset(sigset_t *set) {
	if (!set) { errno = EINVAL; return -1; }

	set->__bits[0] = 0;
	set->__bits[1] = 0;

	return 0;
}

static inline int sigfillset(sigset_t *set) {
	if (!set) { errno = EINVAL; return -1; }

	set->__bits[0] = ~0UL;
	set->__bits[1] = ~0UL;

	return 0;
}

static inline int sigaddset(sigset_t *set, int signo) {
	if (!set || signo < 1 || signo > JACL_SIGSET_MAX) { errno = EINVAL; return -1; }

	unsigned idx  = (unsigned)(signo - 1) / (8U * sizeof(unsigned long));
	unsigned bit  = (unsigned)(signo - 1) % (8U * sizeof(unsigned long));

	set->__bits[idx] |= (1UL << bit);

	return 0;
}
static inline int sigdelset(sigset_t *set, int signo) {
	if (!set || signo < 1 || signo > JACL_SIGSET_MAX) { errno = EINVAL; return -1; }

	unsigned idx  = (unsigned)(signo - 1) / (8U * sizeof(unsigned long));
	unsigned bit  = (unsigned)(signo - 1) % (8U * sizeof(unsigned long));

	set->__bits[idx] &= ~(1UL << bit);

	return 0;
}

static inline int sigismember(const sigset_t *set, int signo) {
	if (!set || signo < 1 || signo > JACL_SIGSET_MAX) { errno = EINVAL; return -1; }

	unsigned idx  = (unsigned)(signo - 1) / (8U * sizeof(unsigned long));
	unsigned bit  = (unsigned)(signo - 1) % (8U * sizeof(unsigned long));

	return (set->__bits[idx] & (1UL << bit)) != 0;
}

static inline int sigaction(int sig, const struct sigaction *restrict act, struct sigaction *restrict oact) {
	#if JACL_OS_LINUX
		/* Linux kernel expects _NSIG/8 = 64/8 = 8 bytes, not sizeof(sigset_t) */
		return syscall(JACL_SYS_SIGACTION, (long)sig, (long)act, (long)oact, 8L);
	#else
		return syscall(JACL_SYS_SIGACTION, (long)sig, (long)act, (long)oact);
	#endif
}


static inline int sigprocmask(int how, const sigset_t *restrict set, sigset_t *restrict oset) {
	#if JACL_OS_LINUX
		return syscall(JACL_SYS_SIGPROCMASK, (long)how, (long)set, (long)oset, (long)sizeof(sigset_t));
	#else
		return syscall(JACL_SYS_SIGPROCMASK, (long)how, (long)set, (long)oset);
	#endif
}

static inline int sigpending(sigset_t *set) {
	#if JACL_OS_LINUX
		return syscall(JACL_SYS_SIGPENDING, (long)set, (long)sizeof(sigset_t));
	#else
		return syscall(JACL_SYS_SIGPENDING, (long)set);
	#endif
}

static inline int sigsuspend(const sigset_t *mask) {
	#if JACL_OS_LINUX
		return (int)syscall(JACL_SYS_SIGSUSPEND, (long)mask, (long)sizeof(sigset_t));
	#else
		return (int)syscall(JACL_SYS_SIGSUSPEND, (long)mask);
	#endif
}

static inline int sigwaitinfo(const sigset_t *restrict set, siginfo_t *restrict info) {
	#ifdef JACL_SYS_SIGWAITINFO
		return syscall(JACL_SYS_SIGWAITINFO, (long)set, (long)info);
	#elif defined(JACL_SYS_SIGTIMEDWAIT)
		#if JACL_OS_LINUX
			return syscall(JACL_SYS_SIGTIMEDWAIT, (long)set, (long)info, (long)NULL, (long)sizeof(sigset_t));
		#else
			return syscall(JACL_SYS_SIGTIMEDWAIT, (long)set, (long)info, (long)NULL);
		#endif
	#else
		errno = ENOSYS;

		return -1;
#endif
}

static inline int sigtimedwait(const sigset_t *restrict set, siginfo_t *restrict info, const struct timespec *restrict timeout) {
	#ifdef JACL_SYS_SIGTIMEDWAIT
		#if JACL_OS_LINUX
			return syscall(JACL_SYS_SIGTIMEDWAIT, (long)set, (long)info, (long)timeout, (long)sizeof(sigset_t));
		#else
			return syscall(JACL_SYS_SIGTIMEDWAIT, (long)set, (long)info, (long)timeout);
		#endif
	#else
		errno = ENOSYS;

		return -1;
	#endif
}

static inline int sigqueue(pid_t pid, int sig, const union sigval value) {
	#ifdef JACL_SYS_SIGQUEUE
		siginfo_t info;

		for (size_t i = 0; i < sizeof(info); ++i) ((unsigned char *)&info)[i] = 0;

		info.si_signo = sig;
		info.si_code = SI_QUEUE;
		info.si_pid = pid;
		info.si_value = value;

		return (int)syscall(JACL_SYS_SIGQUEUE, (long)pid, (long)sig, (long)&info);
	#else
		errno = ENOSYS;

		return -1;
	#endif
}

static inline int sigwait(const sigset_t *restrict set, int *restrict sig) {
	if (!set || !sig) { errno = EINVAL; return -1; }

	siginfo_t info;
	int r = sigtimedwait(set, &info, NULL);

	if (r > 0) {
		*sig = r;	/* POSIX: return value == signal number */

		return 0;
	}

	return -1;		/* errno already set by sigtimedwait */
}

static inline int raise(int sig) {
	pid_t pid = (pid_t)syscall(SYS_getpid);
	long r = syscall(SYS_kill, (long)pid, (long)sig);

	return (r == 0) ? 0 : -1;
}

#else  /* !JACL_HAS_POSIX */

typedef unsigned long sigset_t;

#endif /* JACL_HAS_POSIX */

/* — Core ISO C Functions (C99+) — */

/* Minimal signal() built on sigaction when POSIX is present. */
#if JACL_HAS_POSIX

#if JACL_RESTORER
/* Linux requires signal return trampoline - defined in arch files */
extern void __restore_rt(void);
#endif

static inline sig_t signal(int sig, sig_t func) {
	struct sigaction act, oact;

	act.sa_handler = func;

	sigemptyset(&act.sa_mask);

	act.sa_flags = SA_RESTART;

	#if JACL_RESTORER
		act.sa_flags |= JACL_RESTORER;
		act.sa_restorer = &__restore_rt;
	#endif

	if (sigaction(sig, &act, &oact) < 0) return SIG_ERR;

	return oact.sa_handler;
}

#else

/* Fallback stub: no real signal handling available. */
static inline sig_t signal(int sig, sig_t func) {
	(void)sig;
	(void)func;

	errno = ENOSYS;

	return SIG_ERR;
}
#endif

/* — C11+ Thread Safety Extensions — */
#if JACL_HAS_C11 && !defined(__STDC_NO_THREADS__)
#define signal_atomic_store(obj, val) ((void)(*(obj) = (val)))
#define signal_atomic_load(obj)       (*(obj))
#endif

/* — C99+ inline signal set helpers — */
#if JACL_HAS_C99 && JACL_HAS_POSIX

static inline int __sigisemptyset(const sigset_t *set) { return set ? (set->__bits[0] == 0 && set->__bits[1] == 0) : 0; }
static inline void __sigemptyset_fast(sigset_t *set) { if (set) { set->__bits[0] = 0; set->__bits[1] = 0; } }
static inline void __sigfillset_fast(sigset_t *set) { if (set) { set->__bits[0] = ~0UL; set->__bits[1] = ~0UL; } }
static inline int __sigaddset_fast(sigset_t *set, int sig) { return sigaddset(set, sig); }
static inline int __sigdelset_fast(sigset_t *set, int sig) { return sigdelset(set, sig); }

#endif /* C99+ inline optimizations */

#ifdef __cplusplus
}
#endif

#endif /* _SIGNAL_H */
