/* (c) 2025 FRINKnet & Friends – MIT licence */
#ifndef SCHED_H
#define SCHED_H
#pragma once

#include <config.h>
#include <time.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* C23 feature test macro */
#if JACL_HAS_C23
#  define __STDC_VERSION_SCHED_H__ 202311L
#endif

/* Scheduling parameter structure */
struct sched_param {
  int sched_priority;  /* Process execution scheduling priority */
  int sched_ss_low_priority;           /* Low priority for sporadic server */
  struct timespec sched_ss_repl_period; /* Replenishment period */
  struct timespec sched_ss_init_budget; /* Initial budget */
  int sched_ss_max_repl;               /* Max pending replenishments */
};

/* Scheduling policies */
#define SCHED_OTHER    0  /* Default scheduling policy */
#define SCHED_FIFO     1  /* First-in, first-out scheduling */
#define SCHED_RR       2  /* Round-robin scheduling */
#define SCHED_SPORADIC 3  /* Sporadic server scheduling */

#if JACL_ARCH_WASM
/* ================================================================ */
/* WebAssembly - No real process scheduling                         */
/* ================================================================ */

/* Most scheduling operations don't make sense in WASM */
static inline int sched_yield(void) {
  /* In WASM, yielding is handled by the host runtime */
  return 0;  /* Success - no-op */
}

static inline int sched_get_priority_max(int policy) {
  (void)policy;
  /* Return a reasonable default max priority */
  return 99;
}

static inline int sched_get_priority_min(int policy) {
  (void)policy;

  /* Return a reasonable default min priority */
  return 1;
}

static inline int sched_getparam(pid_t pid, struct sched_param *param) {
  (void)pid;

  if (!param) return -1;

  /* Return default priority */
  param->sched_priority = 0;

  return 0;
}

static inline int sched_setparam(pid_t pid, const struct sched_param *param) {
    (void)pid; (void)param;

    /* Cannot set scheduling parameters in WASM */
    return -1; /* errno = EPERM */
}

static inline int sched_getscheduler(pid_t pid) {
    (void)pid;

    /* Always return default policy */
    return SCHED_OTHER;
}

static inline int sched_setscheduler(pid_t pid, int policy, const struct sched_param *param) {
    (void)pid; (void)policy; (void)param;

    /* Cannot set scheduler in WASM */
    return -1; /* errno = EPERM */
}

static inline int sched_rr_get_interval(pid_t pid, struct timespec *tp) {
  (void)pid;

  if (!tp) return -1;

  /* Return a default time slice */
  tp->tv_sec = 0;
  tp->tv_nsec = 10000000; /* 10ms */

  return 0;
}

/* CPU affinity functions - not supported */
static inline int sched_setaffinity(pid_t pid, size_t cpusetsize, const void *mask) {
  (void)pid; (void)cpusetsize; (void)mask;

  return -1; /* Not supported */
}

static inline int sched_getaffinity(pid_t pid, size_t cpusetsize, void *mask) {
  (void)pid; (void)cpusetsize; (void)mask;

  return -1; /* Not supported */
}

#else
/* ================================================================ */
/* Native Platforms - Full POSIX scheduling support               */
/* ================================================================ */

#include <errno.h>
#include <unistd.h>

#if JACL_HAS_POSIX
  #include <sys/syscall.h>
#endif

/* Linux-specific scheduling policies and clone flags */
#if JACL_OS_LINUX
	#define SCHED_BATCH    3
	#define SCHED_IDLE     5
	#define SCHED_DEADLINE 6

	/* Clone flags for clone() syscall */
	#define CLONE_VM              0x00000100
	#define CLONE_FS              0x00000200
	#define CLONE_FILES           0x00000400
	#define CLONE_SIGHAND         0x00000800
	#define CLONE_THREAD          0x00010000
	#define CLONE_NEWNS           0x00020000
	#define CLONE_SYSVSEM         0x00040000
	#define CLONE_SETTLS          0x00080000
	#define CLONE_PARENT_SETTID   0x00100000
	#define CLONE_CHILD_CLEARTID  0x00200000
	#define CLONE_DETACHED        0x00400000
	#define CLONE_CHILD_SETTID    0x01000000
#endif

/* Thread yielding */
static inline int sched_yield(void) {
	#if JACL_HAS_POSIX
	  return syscall(SYS_sched_yield);
	#elif JACL_OS_WINDOWS
	  /* Windows equivalent */
	  extern void __stdcall Sleep(unsigned long dwMilliseconds);

	  Sleep(0);  /* Yield to other threads */

	  return 0;
	#else
	  return 0;  /* Best effort - no-op */
	#endif
}

/* Priority range functions */
static inline int sched_get_priority_max(int policy) {
  switch (policy) {
    case SCHED_FIFO:
    case SCHED_RR:
      return 99;  /* Typical real-time max priority */
    case SCHED_OTHER:
      return 0;   /* Normal process priority */
    default:
      errno = EINVAL;

      return -1;
  }
}

static inline int sched_get_priority_min(int policy) {
    switch (policy) {
      case SCHED_FIFO:
      case SCHED_RR:
        return 1;   /* Minimum real-time priority */
      case SCHED_OTHER:
        return 0;   /* Normal process priority */
      default:
        errno = EINVAL;

        return -1;
    }
}

/* Process scheduling parameter functions */
static inline int sched_getparam(pid_t pid, struct sched_param *param) {
  if (!param) {
    errno = EINVAL;

    return -1;
  }

	#if JACL_OS_DARWIN
    /* macOS doesn't have full POSIX scheduling - return default */
    param->sched_priority = 0;

    return 0;
	#elif JACL_OS_WINDOWS
    /* Windows priority mapping */
    param->sched_priority = 0;  /* Normal priority */

    return 0;
	#elif JACL_HAS_POSIX
    return syscall(SYS_sched_getparam, pid, param);
	#else
    param->sched_priority = 0;

    return 0;
	#endif
}

static inline int sched_setparam(pid_t pid, const struct sched_param *param) {
  if (!param) {
    errno = EINVAL;

    return -1;
  }

	#if JACL_OS_DARWIN
    /* macOS has limited scheduling control */
    errno = EPERM;

    return -1;
	#elif JACL_OS_WINDOWS
    /* Would need SetPriorityClass/SetThreadPriority */
    errno = EPERM;

    return -1;
	#elif JACL_HAS_POSIX
    return syscall(SYS_sched_setparam, pid, param);
	#else
    errno = ENOSYS;

    return -1;
	#endif
}

static inline int sched_getscheduler(pid_t pid) {
	#if JACL_OS_DARWIN
    /* macOS uses SCHED_OTHER by default */
    (void)pid;

    return SCHED_OTHER;
	#elif JACL_OS_WINDOWS
    (void)pid;

    return SCHED_OTHER; /* Windows doesn't map directly */
	#elif JACL_HAS_POSIX
    return syscall(SYS_sched_getscheduler, pid);
	#else
    (void)pid;

    return SCHED_OTHER;
	#endif
}

static inline int sched_setscheduler(pid_t pid, int policy, const struct sched_param *param) {
  if (!param) {
    errno = EINVAL;

    return -1;
  }

	#if JACL_OS_DARWIN
    /* macOS has limited scheduler control */
    (void)pid; (void)policy;

    errno = EPERM;

    return -1;
	#elif JACL_OS_WINDOWS
    /* Windows scheduling is different */
    (void)pid; (void)policy;

    errno = EPERM;

    return -1;
	#elif JACL_HAS_POSIX
    return syscall(SYS_sched_setscheduler, pid, policy, param);
	#else
    (void)pid; (void)policy;

    errno = ENOSYS;

    return -1;
	#endif
}

static inline int sched_rr_get_interval(pid_t pid, struct timespec *tp) {
  if (!tp) {
    errno = EINVAL;

    return -1;
  }

	#if JACL_OS_LINUX || JACL_OS_BSD
    return syscall(SYS_sched_rr_get_interval, pid, tp);
	#else
    /* Default time slice for round-robin */
    (void)pid;

    tp->tv_sec = 0;
    tp->tv_nsec = 10000000;  /* 10ms typical */

    return 0;
	#endif
}

#if JACL_OS_LINUX

/* CPU affinity functions (Linux extension) */
static inline int sched_setaffinity(pid_t pid, size_t cpusetsize, const void *mask) {
	int ret = syscall(SYS_sched_setaffinity, pid, cpusetsize, mask);

	if (ret < 0) {
		errno = -ret;

		return -1;
	}

	return ret;
}


static inline int sched_getaffinity(pid_t pid, size_t cpusetsize, void *mask) {
	int ret = syscall(SYS_sched_getaffinity, pid, cpusetsize, mask);

	if (ret < 0) {
		errno = -ret;

		return -1;
	}

	return ret;
}
#else
/* CPU affinity not supported on non-Linux systems */
static inline int sched_setaffinity(pid_t pid, size_t cpusetsize, const void *mask) {
  (void)pid; (void)cpusetsize; (void)mask;

  errno = ENOSYS;

  return -1;
}

static inline int sched_getaffinity(pid_t pid, size_t cpusetsize, void *mask) {
  (void)pid; (void)cpusetsize; (void)mask;

  errno = ENOSYS;

  return -1;
}

#endif /* JACL_OS_LINUX */

#endif /* platform switch */

#ifdef __cplusplus
}
#endif

#endif /* SCHED_H */
