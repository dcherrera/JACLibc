/* (c) 2025 FRINKnet & Friends – MIT licence */

#ifdef __cplusplus
	extern "C" {
#endif

#ifdef __ARCH_CONFIG
	#undef nios2
	#define JACL_ARCH nios2
	#define JACL_ARCH_NIOS2 1
	#define __jacl_arch_syscall __nios2_syscall
	#define __jacl_arch_tls_set __nios2_set_tp_register
	#define __jacl_arch_tls_get __nios2_get_tp_register
	#define JACL_BITS 32
#undef __ARCH_CONFIG
#endif

#ifdef __ARCH_SYSCALL
	static inline long __nios2_syscall(long num, long a1, long a2, long a3, long a4, long a5, long a6) {
		long result;
		register long r2 __asm__("r2") = num;
		register long r4 __asm__("r4") = a1;
		register long r5 __asm__("r5") = a2;
		register long r6 __asm__("r6") = a3;
		register long r7 __asm__("r7") = a4;
		register long r8 __asm__("r8") = a5;
		register long r9 __asm__("r9") = a6;

		__asm__ volatile ("trap"
			: "=r"(r2)
			: "r"(r2), "r"(r4), "r"(r5), "r"(r6), "r"(r7), "r"(r8), "r"(r9)
			: "memory");

		result = r2;

		return result;
	}
#undef __ARCH_SYSCALL
#endif

#ifdef __ARCH_START
	__asm__(
		".global _start\n"
		"_start:\n"
		"mov fp, zero\n"
		"mov r4, sp\n"
		"andi sp, sp, -16\n"
		"call _start_main\n"
	);

	#if JACL_OS_LINUX
		__asm__(
			".text\n"
			".globl __restore_rt\n"
			".type __restore_rt,@function\n"
			"__restore_rt:\n"
			"movi r2, 173\n"
			"trap\n"
		);
	#endif
#undef __ARCH_START
#endif

#ifdef __ARCH_TLS
	static inline void __nios2_set_tp_register(void* addr) {
		__asm__ volatile("mov r23, %0" : : "r"(addr) : "memory");
	}

	static inline void* __nios2_get_tp_register(void) {
		void* result;

		__asm__("mov %0, r23" : "=r"(result));

		return result;
	}
#undef __ARCH_TLS
#endif

#ifdef __cplusplus
	}
#endif
