// (c) 2025 FRINKnet & Friends – MIT licence
#ifndef SYS_SYSCALL_H
#define SYS_SYSCALL_H
#pragma once

#include <config.h>
#include <stdarg.h>

// Include syscall numbers
typedef enum {
#define X(SYS, num, ...) SYS = (num),
#include JACL_X_SYSCALL
#undef X
	SYS_max
} syscall_t;

// Include OS-specific syscall
#define __OS_SYSCALL
#include JACL_HEADER(os,JACL_OS)

#ifdef __cplusplus
extern "C" {
#endif

static inline long syscall(long num, ...) {
	va_list args;
	long a1, a2, a3, a4, a5, a6;

	va_start(args, num);
	a1 = va_arg(args, long);
	a2 = va_arg(args, long);
	a3 = va_arg(args, long);
	a4 = va_arg(args, long);
	a5 = va_arg(args, long);
	a6 = va_arg(args, long);
	va_end(args);

	long result = __jacl_os_syscall(num, a1, a2, a3, a4, a5, a6);

	// Convert Linux negative errno to POSIX -1 + errno
	if (result < 0 && result >= -4095) {
		errno = -result;

		return -1;
	}

	return result;
}

#ifdef __cplusplus
}
#endif

#endif // SYS_SYSCALL_H
