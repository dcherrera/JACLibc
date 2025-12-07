/* (c) 2025 FRINKnet & Friends – MIT licence */

#ifdef __cplusplus
	extern "C" {
#endif

#ifdef __ARCH_CONFIG
	#undef sparc64
	#define JACL_ARCH sparc64
	#define JACL_ARCH_SPARC64 1
	#define __jacl_arch_syscall __sparc64_syscall
	#define __jacl_arch_tls_set __sparc64_set_tp_register
	#define __jacl_arch_tls_get __sparc64_get_tp_register
	#define JACL_BITS 64
#undef __ARCH_CONFIG
#endif

#ifdef __ARCH_SYSCALL
	static inline long __sparc64_syscall(long num, long a1, long a2, long a3, long a4, long a5, long a6) {
		long result;
		register long g1 __asm__("g1") = num;
		register long o0 __asm__("o0") = a1;
		register long o1 __asm__("o1") = a2;
		register long o2 __asm__("o2") = a3;
		register long o3 __asm__("o3") = a4;
		register long o4 __asm__("o4") = a5;
		register long o5 __asm__("o5") = a6;

		__asm__ volatile ("t 0x6d"
			: "=r"(o0)
			: "r"(g1), "r"(o0), "r"(o1), "r"(o2), "r"(o3), "r"(o4), "r"(o5)
			: "memory", "cc");

		result = o0;

		return result;
	}
#undef __ARCH_SYSCALL
#endif

#ifdef __ARCH_START
	__asm__(
		".global _start\n"
		"_start:\n"
		"mov %g0, %fp\n"
		"mov %sp, %o0\n"
		"and %sp, -16, %sp\n"
		"call _start_main\n"
		"nop\n"
	);

#if JACL_OS_LINUX
	__asm__(
		".text\n"
		".globl __restore_rt\n"
		".type __restore_rt,@function\n"
		"__restore_rt:\n"
		"mov 101, %g1\n"
		"ta 0x6d\n"
	);
#endif

#undef __ARCH_START
#endif

#ifdef __ARCH_TLS
	static inline void __sparc64_set_tp_register(void* addr) {
		__asm__ volatile("mov %0, %%g7" : : "r"(addr) : "memory");
	}

	static inline void* __sparc64_get_tp_register(void) {
		void* result;

		__asm__("mov %%g7, %0" : "=r"(result));

		return result;
	}
#undef __ARCH_TLS
#endif

#ifdef __cplusplus
	}
#endif
