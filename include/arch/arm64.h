/* (c) 2025 FRINKnet & Friends – MIT licence */

#ifdef __cplusplus
	extern "C" {
#endif

#ifdef __ARCH_CONFIG
	#undef arm64
	#define JACL_ARCH arm64
	#define JACL_ARCH_ARM64 1
	#define __jacl_arch_syscall __arm64_syscall
	#define __jacl_arch_tls_set __arm64_set_tp_register
	#define __jacl_arch_tls_get __arm64_get_tp_register
	#define __jacl_arch_clone_thread __arm64_clone_thread
	#define JACL_BITS 64
#undef __ARCH_CONFIG
#endif

#ifdef __ARCH_SYSCALL
	static inline long __arm64_syscall(long num, long a1, long a2, long a3, long a4, long a5, long a6) {
		long result;
		register long x8 __asm__("x8") = num;
		register long x0 __asm__("x0") = a1;
		register long x1 __asm__("x1") = a2;
		register long x2 __asm__("x2") = a3;
		register long x3 __asm__("x3") = a4;
		register long x4 __asm__("x4") = a5;
		register long x5 __asm__("x5") = a6;

		__asm__ volatile ("svc #0"
			: "=r"(x0)
			: "r"(x8), "r"(x0), "r"(x1), "r"(x2), "r"(x3), "r"(x4), "r"(x5)
			: "memory");

		result = x0;

		return result;
	}
#undef __ARCH_SYSCALL
#endif

#ifdef __ARCH_START
	__asm__(
		".global _start\n"
		"_start:\n"
		"mov x29, #0\n"
		"mov x30, #0\n"
		"mov x0, sp\n"
		"and sp, x0, #-16\n"
		"bl _start_main\n"
	);

	#if JACL_OS_LINUX
		__asm__(
			".text\n"
			".globl __restore_rt\n"
			".type __restore_rt,@function\n"
			"__restore_rt:\n"
			"mov x8, #139\n"
			"svc #0\n"
		);
	#endif
#undef __ARCH_START
#endif

#ifdef __ARCH_TLS
	static inline void __arm64_set_tp_register(void* addr) {
		__asm__ volatile("msr tpidr_el0, %0" : : "r"(addr) : "memory");
	}

	static inline void* __arm64_get_tp_register(void) {
		void* result;
		__asm__("mrs %0, tpidr_el0" : "=r"(result));
		return result;
	}
#undef __ARCH_TLS
#endif

#ifdef __ARCH_CLONE && JACL_OS_LINUX
	static inline pid_t __arm64_clone_thread(void *stack, size_t stack_size, int (*fn)(void *), void *arg) {
		char *stack_top = (char *)stack + stack_size;
		stack_top = (char *)((uintptr_t)stack_top & ~15UL) - 16;

		int flags = CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_THREAD;
		long ret;

		__asm__ volatile(
			"mov x0, %2\n\t"
			"mov x1, %3\n\t"
			"mov x2, xzr\n\t"
			"mov x3, xzr\n\t"
			"mov x4, xzr\n\t"
			"mov x8, #220\n\t"
			"svc #0\n\t"
			"cbnz x0, 1f\n\t"

			"mov x29, xzr\n\t"
			"mov x0, %5\n\t"
			"blr %4\n\t"
			"mov x8, #93\n\t"
			"svc #0\n\t"

			"1:\n\t"
			: "=r"(ret)
			: "r"((long)220), "r"((long)flags), "r"(stack_top), "r"(fn), "r"(arg)
			: "x0", "x1", "x2", "x3", "x4", "x8", "memory"
		);

		return ret;
	}
#undef __ARCH_CLONE
#endif

#ifdef __cplusplus
	}
#endif
