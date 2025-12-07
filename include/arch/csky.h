/* (c) 2025 FRINKnet & Friends – MIT licence */

#ifdef __cplusplus
	extern "C" {
#endif

#ifdef __ARCH_CONFIG
	#undef csky
	#define JACL_ARCH csky
	#define JACL_ARCH_CSKY 1
	#define __jacl_arch_syscall __csky_syscall
	#define __jacl_arch_tls_set __csky_set_tp_register
	#define __jacl_arch_tls_get __csky_get_tp_register
	#define JACL_BITS 32
#undef __ARCH_CONFIG
#endif

#ifdef __ARCH_SYSCALL
	static inline long __csky_syscall(long num, long a1, long a2, long a3, long a4, long a5, long a6) {
		long result;
		register long r7 __asm__("r7") = num;
		register long r0 __asm__("r0") = a1;
		register long r1 __asm__("r1") = a2;
		register long r2 __asm__("r2") = a3;
		register long r3 __asm__("r3") = a4;
		register long r4 __asm__("r4") = a5;
		register long r5 __asm__("r5") = a6;

		__asm__ volatile ("trap 0"
			: "=r"(r0)
			: "r"(r7), "r"(r0), "r"(r1), "r"(r2), "r"(r3), "r"(r4), "r"(r5)
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
		"movi r8, 0\n"
		"mov r0, sp\n"
		"andi sp, sp, -16\n"
		"jsr _start_main\n"
	);

	#if JACL_OS_LINUX
		__asm__(
			".text\n"
			".globl __restore_rt\n"
			".type __restore_rt,@function\n"
			"__restore_rt:\n"
			"movi r7, 139\n"
			"trap 0\n"
		);
	#endif
#undef __ARCH_START
#endif

#ifdef __ARCH_TLS
	static inline void __csky_set_tp_register(void* addr) {
		__asm__ volatile("mov r31, %0" : : "r"(addr) : "memory");
	}

	static inline void* __csky_get_tp_register(void) {
		void* result;

		__asm__("mov %0, r31" : "=r"(result));

		return result;
	}
#undef __ARCH_TLS
#endif

#ifdef __cplusplus
	}
#endif
