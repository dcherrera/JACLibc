/* (c) 2025 FRINKnet & Friends – MIT licence */

#ifdef __cplusplus
	extern "C" {
#endif

#ifdef __ARCH_CONFIG
	#undef hexagon
	#define JACL_ARCH hexagon
	#define JACL_ARCH_HEXAGON 1
	#define __jacl_arch_syscall __hexagon_syscall
	#define __jacl_arch_tls_set __hexagon_set_tp_register
	#define __jacl_arch_tls_get __hexagon_get_tp_register
	#define JACL_BITS 32
#undef __ARCH_CONFIG
#endif

#ifdef __ARCH_SYSCALL
	static inline long __hexagon_syscall(long num, long a1, long a2, long a3, long a4, long a5, long a6) {
		long result;
		register long r6 __asm__("r6") = num;
		register long r0 __asm__("r0") = a1;
		register long r1 __asm__("r1") = a2;
		register long r2 __asm__("r2") = a3;
		register long r3 __asm__("r3") = a4;
		register long r4 __asm__("r4") = a5;
		register long r5 __asm__("r5") = a6;

		__asm__ volatile ("trap0(#0)"
			: "=r"(r0)
			: "r"(r6), "r"(r0), "r"(r1), "r"(r2), "r"(r3), "r"(r4), "r"(r5)
			: "memory");

		result = r0;

		return result;
	}
#undef __ARCH_SYSCALL
#endif

#ifdef __ARCH_START
	__asm__(
		".global _start\n"
		"_start:\n"
		"r29 = #0\n"
		"r0 = r29\n"
		"r29 = and(r29, #-16)\n"
		"call _start_main\n"
	);

	#if JACL_OS_LINUX
		__asm__(
			".text\n"
			".globl __restore_rt\n"
			".type __restore_rt,@function\n"
			"__restore_rt:\n"
			"r6 = #139\n"
			"trap0(#1)\n"
		);
	#endif
#undef __ARCH_START
#endif

#ifdef __ARCH_TLS
	static inline void __hexagon_set_tp_register(void* addr) {
		__asm__ volatile("ugp = %0" : : "r"(addr) : "memory");
	}

	static inline void* __hexagon_get_tp_register(void) {
		void* result;

		__asm__("%0 = ugp" : "=r"(result));

		return result;
	}
#undef __ARCH_TLS
#endif

#ifdef __cplusplus
	}
#endif
