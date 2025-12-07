/* (c) 2025 FRINKnet & Friends – MIT licence */
#ifndef _SYS_FUTEX_H
#define _SYS_FUTEX_H
#pragma once

#include <config.h>
#include <sys/syscall.h>
#include <sys/types.h>

#if JACL_OS_LINUX

#define FUTEX_WAIT 0
#define FUTEX_WAKE 1
#define FUTEX_PRIVATE_FLAG 128
#define FUTEX_CLOCK_REALTIME 256

static inline int futex(int *uaddr, int op, int val, const struct timespec *timeout, int *uaddr2, int val3) {
	return (int)syscall(SYS_futex, (long)uaddr, (long)op, (long)val, (long)timeout, (long)uaddr2, (long)val3);
}

#endif /* JACL_OS_LINUX */

#endif /* _SYS_FUTEX_H */

