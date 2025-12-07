/* (c) 2025 FRINKnet & Friends – MIT licence */

#ifdef __cplusplus
	extern "C" {
#endif

#ifdef __ARCH_CONFIG
	#undef s390x
	#define JACL_ARCH s390x
	#define JACL_ARCH_S390X 1
	#define __jacl_arch_syscall __s390x_syscall
	#define __jacl_arch_tls_set __s390x_set_tp_register
	#define __jacl_arch_tls_get __s390x_get_tp_register
	#define JACL_BITS 64
#undef __ARCH_CONFIG
#endif

#ifdef __ARCH_SYSCALL
	static inline long __s390x_syscall(long num, long a1, long a2, long a3, long a4, long a5, long a6) {
		long result;
		register long r1 __asm__("1") = num;
		register long r2 __asm__("2") = a1;
		register long r3 __asm__("3") = a2;
		register long r4 __asm__("4") = a3;
		register long r5 __asm__("5") = a4;
		register long r6 __asm__("6") = a5;
		register long r7 __asm__("7") = a6;

		__asm__ volatile ("svc 0"
			: "=r"(r2)
			: "r"(r1), "r"(r2), "r"(r3), "r"(r4), "r"(r5), "r"(r6), "r"(r7)
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
		"lghi %r11, 0\n"
		"lgr %r2, %r15\n"
		"aghi %r15, -160\n"
		"nill %r15, 0xfff0\n"
		"brasl %r14, _start_main\n"
	);

	#if JACL_OS_LINUX
		__asm__(
			".text\n"
			".globl __restore_rt\n"
			".type __restore_rt,@function\n"
			"__restore_rt:\n"
			"lghi %r1, 173\n"
			"svc 0\n"
		);
	#endif
#undef __ARCH_START
#endif

#ifdef __ARCH_TLS
	static inline void __s390x_set_tp_register(void* addr) {
		__asm__ volatile("lgr %%r0, %0; ear %%r0, %%a0; sar %%a0, %%r0" : : "r"(addr) : "memory", "r0");
	}

	static inline void* __s390x_get_tp_register(void) {
		void* result;

		__asm__("ear %0, %%a0" : "=r"(result));

		return result;
	}
#undef __ARCH_TLS
#endif

#ifdef __cplusplus
	}
#endif
