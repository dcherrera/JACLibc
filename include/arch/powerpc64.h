/* (c) 2025 FRINKnet & Friends – MIT licence */

#ifdef __cplusplus
	extern "C" {
#endif

#ifdef __ARCH_CONFIG
	#undef powerpc64
	#define JACL_ARCH powerpc64
	#define JACL_ARCH_POWERPC64 1
	#define __jacl_arch_syscall __powerpc64_syscall
	#define __jacl_arch_tls_set __powerpc64_set_tp_register
	#define __jacl_arch_tls_get __powerpc64_get_tp_register
	#define JACL_BITS 64
#undef __ARCH_CONFIG
#endif

#ifdef __ARCH_SYSCALL
	static inline long __powerpc64_syscall(long num, long a1, long a2, long a3, long a4, long a5, long a6) {
		long result;
		register long r0 __asm__("r0") = num;
		register long r3 __asm__("r3") = a1;
		register long r4 __asm__("r4") = a2;
		register long r5 __asm__("r5") = a3;
		register long r6 __asm__("r6") = a4;
		register long r7 __asm__("r7") = a5;
		register long r8 __asm__("r8") = a6;

		__asm__ volatile ("sc"
			: "=r"(r3)
			: "r"(r0), "r"(r3), "r"(r4), "r"(r5), "r"(r6), "r"(r7), "r"(r8)
			: "memory", "cr0", "r9", "r10", "r11", "r12");

		result = r3;
		return result;
	}
#undef __ARCH_SYSCALL
#endif

#ifdef __ARCH_START
	__asm__(
		".global _start\n"
		"_start:\n"
		"li 31, 0\n"
		"mr 3, 1\n"
		"clrrdi 1, 1, 4\n"
		"bl _start_main\n"
	);

	#if JACL_OS_LINUX
		__asm__(
			".text\n"
			".globl __restore_rt\n"
			".type __restore_rt,@function\n"
			"__restore_rt:\n"
			"li 0, 173\n"
			"sc\n"
		);
	#endif
#undef __ARCH_START
#endif

#ifdef __ARCH_TLS
	static inline void __powerpc64_set_tp_register(void* addr) {
		__asm__ volatile("mr 13, %0" : : "r"(addr) : "memory");
	}

	static inline void* __powerpc64_get_tp_register(void) {
		void* result;

		__asm__("mr %0, 13" : "=r"(result));

		return result;
	}
#undef __ARCH_TLS
#endif

#ifdef __cplusplus
	}
#endif
