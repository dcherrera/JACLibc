/* (c) 2025 FRINKnet & Friends – MIT licence */

#ifdef __cplusplus
	extern "C" {
#endif

#ifdef __ARCH_CONFIG
	#undef powerpc32
	#define JACL_ARCH powerpc32
	#define JACL_ARCH_POWERPC32 1
	#define __jacl_arch_syscall __powerpc32_syscall
	#define __jacl_arch_tls_set __powerpc32_set_tp_register
	#define __jacl_arch_tls_get __powerpc32_get_tp_register
	#define __jacl_arch_clone_thread __powerpc32_clone_thread
	#define JACL_BITS 32
#undef __ARCH_CONFIG
#endif

#ifdef __ARCH_SYSCALL
	static inline long __powerpc32_syscall(long num, long a1, long a2, long a3, long a4, long a5, long a6) {
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
		"clrrwi 1, 1, 4\n"
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
	static inline void __powerpc32_set_tp_register(void* addr) {
		__asm__ volatile("mr 2, %0" : : "r"(addr) : "memory");
	}

	static inline void* __powerpc32_get_tp_register(void) {
		void* result;

		__asm__("mr %0, 2" : "=r"(result));

		return result;
	}
#undef __ARCH_TLS
#endif

#ifdef __ARCH_CLONE && JACL_OS_LINUX
	static inline pid_t __powerpc64_clone_thread(void *stack, size_t stack_size, int (*fn)(void *), void *arg) {
		char *stack_top = (char *)stack + stack_size;
		stack_top = (char *)((uintptr_t)stack_top & ~15UL) - 112;  // ABI red zone

		int flags = CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_THREAD;
		long ret;

		__asm__ volatile(
			"mr 3, %2\n\t"
			"mr 4, %3\n\t"
			"li 5, 0\n\t"
			"li 6, 0\n\t"
			"li 7, 0\n\t"
			"li 0, 120\n\t"
			"sc\n\t"
			"cmpwi 3, 0\n\t"
			"bne 1f\n\t"

			"li 31, 0\n\t"
			"mr 3, %5\n\t"
			"mtctr %4\n\t"
			"bctrl\n\t"
			"li 0, 1\n\t"
			"sc\n\t"

			"1:\n\t"
			: "=r"(ret)
			: "r"((long)120), "r"((long)flags), "r"(stack_top), "r"(fn), "r"(arg)
			: "r0", "r3", "r4", "r5", "r6", "r7", "ctr", "memory"
		);

		return ret;
	}
#undef __ARCH_CLONE
#endif

#ifdef __cplusplus
	}
#endif
