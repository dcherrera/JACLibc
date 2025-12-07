/* (c) 2025 FRINKnet & Friends – MIT licence */

#ifdef __OS_CONFIG
	#undef openbsd
	#undef elf
	#define JACL_OS openbsd
	#define JACL_OS_BSD 1
	#define JACL_OS_OPENBSD 1
	#define JACL_FMT elf
	#define JACL_FMT_ELF
	#define __jacl_os_syscall __openbsd_syscall
#undef __OS_CONFIG
#endif

#ifdef __OS_SYSCALL
	struct __jacl_openbsd_libc {
		#define X(SYS, num, fn, rettype, params, ...) rettype (*fn##_)params;
		#include JACL_X_SYSCALL
		#undef X
	};

	extern struct __jacl_openbsd_libc _openbsd;

	/* syscall backend */
	static inline long __openbsd_syscall(long n,
		long a0, long a1, long a2, long a3, long a4, long a5)
	{
		switch (n) {
		#define X(SYS, num, fn, rettype, params, ...) \
			case SYS: return (long)_openbsd.fn##_(__VA_ARGS__);
		#include JACL_X_SYSCALL
		#undef X
		default:
			errno = ENOSYS;
			return -1;
		}
	}
#undef __OS_SYSCALL
#endif

#ifdef __OS_INIT
	/* dispatch table */
	struct __jacl_openbsd_libc {
	#define X(SYS, num, fn, rettype, params, ...) \
		rettype (*fn##_)params;
	#include JACL_X_SYSCALL
	#undef X
	};

	struct __jacl_openbsd_libc _openbsd;

	/* init */
	static inline void __jacl_init_os(void) {
		void *h = dlopen("libc.so", RTLD_LAZY);
		if (!h) h = dlopen(NULL, RTLD_LAZY);

	#define X(SYS, num, fn, rettype, params, ...) \
		_openbsd.fn##_ = (rettype(*)params)dlsym(h, #fn);
	#include JACL_X_SYSCALL
	#undef X
	}

#undef __OS_INIT
#endif
