/* (c) 2025 FRINKnet & Friends – MIT licence */

#ifdef __cplusplus
	extern "C" {
#endif

#ifdef __ARCH_CONFIG
	#undef m68k
	#define JACL_ARCH m68k
	#define JACL_ARCH_M68K 1
	#define __jacl_arch_syscall __m68k_syscall
	#define __jacl_arch_tls_set __m68k_set_tp_register
	#define __jacl_arch_tls_get __m68k_get_tp_register
	#define JACL_BITS 32
#undef __ARCH_CONFIG
#endif

#ifdef __ARCH_SYSCALL
	static inline long __m68k_syscall(long num, long a1, long a2, long a3, long a4, long a5, long a6) {
		long result;
		register long d0 __asm__("d0") = num;
		register long d1 __asm__("d1") = a1;
		register long d2 __asm__("d2") = a2;
		register long d3 __asm__("d3") = a3;
		register long d4 __asm__("d4") = a4;
		register long d5 __asm__("d5") = a5;
		register long a0 __asm__("a0") = a6;

		__asm__ volatile ("trap #0"
			: "=r"(d0)
			: "r"(d0), "r"(d1), "r"(d2), "r"(d3), "r"(d4), "r"(d5), "r"(a0)
			: "memory");

		result = d0;

		return result;
	}
#undef __ARCH_SYSCALL
#endif

#ifdef __ARCH_START
	__asm__(
		".global _start\n"
		"_start:\n"
		"move.l #0, %fp\n"
		"move.l %sp, %d0\n"
		"and.l #-16, %sp\n"
		"move.l %d0, -(%sp)\n"
		"jsr _start_main\n"
	);

	#if JACL_OS_LINUX
		__asm__(
			".text\n"
			".globl __restore_rt\n"
			".type __restore_rt,@function\n"
			"__restore_rt:\n"
			"move.l #173, %d0\n"
			"trap #0\n"
		);
	#endif
#undef __ARCH_START
#endif

#ifdef __ARCH_TLS
	static inline void __m68k_set_tp_register(void* addr) {
		__asm__ volatile("move.l %0, %%a6" : : "r"(addr) : "memory");
	}

	static inline void* __m68k_get_tp_register(void) {
		void* result;

		__asm__("move.l %%a6, %0" : "=r"(result));

		return result;
	}
#undef __ARCH_TLS
#endif

#ifdef __cplusplus
	}
#endif

